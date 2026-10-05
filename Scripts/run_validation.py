"""Invoke the UE validation Commandlet and summarize its schema 1 report."""

import argparse
import codecs
import json
from pathlib import Path
import subprocess
import sys


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ue-cmd", required=True, help="Path to UnrealEditor-Cmd.exe")
    parser.add_argument("--project", required=True, help="Path to the .uproject file")
    parser.add_argument("--scope", default="/Game/ValidationTest")
    parser.add_argument("--blueprint-prefix", default="BP_")
    parser.add_argument("--report-path", default="Saved/ValidationReports/ValidationReport.json")
    return parser.parse_args(argv)


def build_command(args):
    ue_cmd = Path(args.ue_cmd).resolve()
    project = Path(args.project).resolve()
    if not ue_cmd.is_file():
        raise ValueError(f"UE executable does not exist: {ue_cmd}")
    if not project.is_file() or project.suffix.lower() != ".uproject":
        raise ValueError(f"Project must be an existing .uproject file: {project}")
    report_value = args.report_path.strip()
    if not report_value:
        raise ValueError("Report path is empty.")
    report = Path(report_value)
    if not report.is_absolute():
        report = project.parent / report
    report = report.resolve()
    command = [str(ue_cmd), str(project), "-run=ValidateAssets", "-unattended",
               "-NullRHI", "-nosplash", "-stdout", "-FullStdOutLogOutput",
               f"-Scope={args.scope}", f"-BlueprintPrefix={args.blueprint_prefix}",
               f"-ReportPath={report}"]
    return command, project.parent, report


def report_stamp(path):
    try:
        stat = path.stat()
    except FileNotFoundError:
        return None
    return stat.st_mtime_ns, stat.st_size


def read_summary(path, exit_code):
    data = path.read_bytes()
    # UE FileHelper may use UTF-16 for reports containing non-ASCII text.
    encoding = "utf-16" if data.startswith((codecs.BOM_UTF16_LE, codecs.BOM_UTF16_BE)) else "utf-8-sig"
    report = json.loads(data.decode(encoding))
    if not isinstance(report, dict):
        raise ValueError("Report root must be an object.")
    if type(report.get("schema_version")) is not int or report["schema_version"] != 1:
        raise ValueError("Unsupported report schema; expected schema_version 1.")
    if report.get("tool") != "ProductionPipelineToolkit":
        raise ValueError("Unexpected report tool identity.")
    if not isinstance(report.get("scope"), str) or not isinstance(report.get("results"), list):
        raise ValueError("Report must contain scope and results.")
    counts = {}
    for field in ("asset_count", "passed", "failed", "skipped"):
        value = report.get(field)
        if type(value) is not int or value < 0:
            raise ValueError(f"Report field {field} must be a nonnegative integer.")
        counts[field] = value
    if counts["asset_count"] != sum(counts[key] for key in ("passed", "failed", "skipped")):
        raise ValueError("Report summary counts are inconsistent.")
    if counts["asset_count"] != len(report["results"]):
        raise ValueError("Report result count does not match asset_count.")
    if (counts["failed"] > 0) != (exit_code == 1):
        raise ValueError("Report failure count contradicts the process exit code.")
    return report["scope"], counts


def main(argv=None):
    args = parse_args(argv)
    try:
        command, project_dir, report = build_command(args)
        previous = report_stamp(report)
        print(f"Running validation | Project={command[1]} | Report={report}", flush=True)
        # Inherit stdout/stderr so UE diagnostics remain visible to the caller.
        result = subprocess.run(command, cwd=project_dir, shell=False, check=False)
        if result.returncode == 2:
            print("Validation execution/report failed (UE exit 2); no report consumed.", file=sys.stderr)
            return 2
        if result.returncode not in (0, 1):
            raise ValueError(f"UE terminated with unexpected exit code {result.returncode}; no report consumed.")
        current = report_stamp(report)
        if current is None or current == previous:
            raise ValueError("UE did not produce a fresh report; no stale report consumed.")
        scope, counts = read_summary(report, result.returncode)
        print(f"Scope={scope} | Assets={counts['asset_count']} | PASS={counts['passed']} | "
              f"FAIL={counts['failed']} | SKIP={counts['skipped']} | ExitCode={result.returncode}", flush=True)
        return result.returncode
    except (OSError, ValueError, UnicodeError) as error:
        print(f"Validation error: {error}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("Validation interrupted.", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
