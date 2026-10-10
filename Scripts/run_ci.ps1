param(
    [string]$UERoot = $env:UE_ROOT,
    [string]$Python = $env:PYTHON_COMMAND
)

$ErrorActionPreference = 'Stop'
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$project = Join-Path $projectRoot 'PipelineToolkit.uproject'
$runId = [guid]::NewGuid().ToString('N')
$relativeArtifacts = "Saved/Automation/CI/$runId"
$artifacts = Join-Path $projectRoot $relativeArtifacts
New-Item -ItemType Directory -Path $artifacts -Force | Out-Null
if ($env:GITHUB_OUTPUT) {
    Add-Content -LiteralPath $env:GITHUB_OUTPUT -Value "artifact_dir=$relativeArtifacts" -Encoding UTF8
}
$script:stages = @()
$pipelineExit = 1
$beforeAssets = $null

function Invoke-CIProcess {
    param([string]$Name, [string]$Program, [string[]]$Arguments, [int]$ExpectedExit = 0)
    Write-Host "Starting $Name (expected exit $ExpectedExit)"
    $log = Join-Path $artifacts "$Name.log"
    $started = [DateTime]::UtcNow.ToString('o')
    $previousPreference = $ErrorActionPreference
    try {
        # Windows PowerShell treats native stderr as ErrorRecords; retain diagnostics
        # and decide success from the native exit code instead.
        $ErrorActionPreference = 'Continue'
        $global:LASTEXITCODE = $null
        & $Program @Arguments 2>&1 | ForEach-Object { $_.ToString() } |
            Tee-Object -FilePath $log | Out-Host
        $actualExit = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousPreference
    }
    $script:stages += [pscustomobject]@{
        Name = $Name; Program = $Program; Arguments = $Arguments
        StartedUtc = $started; Exit = $actualExit; ExpectedExit = $ExpectedExit
        Passed = ($null -ne $actualExit -and $actualExit -eq $ExpectedExit)
    }
    if ($null -eq $actualExit -or $actualExit -ne $ExpectedExit) {
        throw "$Name exited '$actualExit'; expected $ExpectedExit. See $log"
    }
}

function Get-AssetSnapshot {
    @(Get-ChildItem -LiteralPath (Join-Path $projectRoot 'Content') -Recurse -File -Filter '*.uasset' |
        Sort-Object FullName | ForEach-Object {
            [pscustomobject]@{
                Path = $_.FullName.Substring($projectRoot.Length + 1)
                Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
            }
        })
}

function Confirm-FixtureReport {
    param([string]$File, [int[]]$Counts, [string]$Scope)
    if (-not (Test-Path -LiteralPath $File -PathType Leaf)) { throw "Missing fixture report: $File" }
    $report = Get-Content -Raw -LiteralPath $File -Encoding UTF8 | ConvertFrom-Json
    $actual = @($report.asset_count, $report.passed, $report.failed, $report.skipped)
    if ($report.schema_version -ne 1 -or $report.scope -ne $Scope -or
        ($actual -join ',') -ne ($Counts -join ',')) {
        throw "Unexpected fixture report: $File"
    }
}

Push-Location $projectRoot
try {
    if ([string]::IsNullOrWhiteSpace($UERoot)) { throw 'Set UE_ROOT or pass -UERoot with the installed UE 5.8.2 directory.' }
    if ([string]::IsNullOrWhiteSpace($Python)) { $Python = 'python' }
    $UERoot = [System.IO.Path]::GetFullPath($UERoot)
    $build = Join-Path $UERoot 'Engine/Build/BatchFiles/Build.bat'
    $editor = Join-Path $UERoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
    $versionPath = Join-Path $UERoot 'Engine/Build/Build.version'
    foreach ($required in @($build, $editor, $versionPath, $project)) {
        if (-not (Test-Path -LiteralPath $required -PathType Leaf)) { throw "Required file missing: $required" }
    }
    $version = Get-Content -Raw -LiteralPath $versionPath | ConvertFrom-Json
    if ($version.MajorVersion -ne 5 -or $version.MinorVersion -ne 8 -or $version.PatchVersion -ne 2) {
        throw 'This pipeline requires UE 5.8.2.'
    }
    $pythonCommand = @(Get-Command $Python -CommandType Application -ErrorAction Stop)[0]
    $Python = $pythonCommand.Source
    Invoke-CIProcess 'PythonVersion' $Python @('-B', '-c', 'import sys; print(sys.version); sys.exit(0 if sys.version_info >= (3, 9) else 1)')
    $beforeAssets = ConvertTo-Json -InputObject @(Get-AssetSnapshot) -Depth 4
    Set-Content -LiteralPath (Join-Path $artifacts 'AssetsBefore.json') -Value $beforeAssets -Encoding UTF8
    Invoke-CIProcess 'PythonUnitTests' $Python @('-B', '-m', 'unittest', 'discover', '-s', 'Scripts/tests', '-v')
    Invoke-CIProcess 'Build' $build @('PipelineToolkitEditor', 'Win64', 'Development', "-Project=$project", '-WaitMutex', '-NoHotReloadFromIDE')

    $automationDirectory = Join-Path $artifacts 'Automation'
    Invoke-CIProcess 'Automation' $editor @($project, '-unattended', '-NullRHI', '-nosplash',
        '-stdout', '-FullStdOutLogOutput', '-ExecCmds=Automation RunTests PipelineToolkit.Validation',
        '-TestExit=Automation Test Queue Empty', "-ReportExportPath=$automationDirectory",
        "-abslog=$(Join-Path $artifacts 'AutomationEngine.log')")
    $indexPath = Join-Path $automationDirectory 'index.json'
    if (-not (Test-Path -LiteralPath $indexPath -PathType Leaf)) { throw 'Automation exited without a report.' }
    $automation = Get-Content -Raw -LiteralPath $indexPath -Encoding UTF8 | ConvertFrom-Json
    $expectedTests = @('PipelineToolkit.Validation.BlueprintNamingRule',
        'PipelineToolkit.Validation.ReportWriter', 'PipelineToolkit.Validation.Runner')
    foreach ($testName in $expectedTests) {
        $test = @($automation.tests | Where-Object { $_.fullTestPath -eq $testName })
        if ($test.Count -ne 1 -or $test[0].state -ne 'Success' -or $test[0].errors -ne 0 -or $test[0].warnings -ne 0) {
            throw "Automation contract did not pass: $testName"
        }
    }
    if ($automation.failed -ne 0 -or $automation.notRun -ne 0 -or $automation.inProcess -ne 0) {
        throw 'Automation contains failed or incomplete tests.'
    }
    $defaultReport = Join-Path $artifacts 'Default.json'
    Invoke-CIProcess 'FixtureSmoke' $Python @('-B', 'Scripts/run_validation.py', '--ue-cmd', $editor,
        '--project', $project, '--report-path', $defaultReport) 1
    Confirm-FixtureReport $defaultReport @(4, 2, 1, 1) '/Game/ValidationTest'
    $passingReport = Join-Path $artifacts 'Characters.json'
    Invoke-CIProcess 'PassingScope' $Python @('-B', 'Scripts/run_validation.py', '--ue-cmd', $editor,
        '--project', $project, '--scope', '/Game/ValidationTest/Characters', '--report-path', $passingReport)
    Confirm-FixtureReport $passingReport @(1, 1, 0, 0) '/Game/ValidationTest/Characters'
    $pipelineExit = 0
}
catch {
    $failure = $_.Exception.Message
    Write-Host "CI failed: $failure"
    Set-Content -LiteralPath (Join-Path $artifacts 'Failure.txt') -Value $failure -Encoding UTF8
}
finally {
    if ($null -ne $beforeAssets) {
        try {
            $afterAssets = ConvertTo-Json -InputObject @(Get-AssetSnapshot) -Depth 4
            Set-Content -LiteralPath (Join-Path $artifacts 'AssetsAfter.json') -Value $afterAssets -Encoding UTF8
            if ($beforeAssets -ne $afterAssets) {
                $pipelineExit = 1
                Add-Content -LiteralPath (Join-Path $artifacts 'Failure.txt') -Value 'Asset hashes changed.' -Encoding UTF8
            }
        }
        catch {
            $pipelineExit = 1
            Add-Content -LiteralPath (Join-Path $artifacts 'Failure.txt') -Value $_.Exception.Message -Encoding UTF8
        }
    }
    [pscustomobject]@{Exit = $pipelineExit; Stages = $script:stages} |
        ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $artifacts 'Results.json') -Encoding UTF8
    Pop-Location
}
Write-Host "CI exit=$pipelineExit | Evidence=$artifacts"
exit $pipelineExit
