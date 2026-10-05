from contextlib import redirect_stderr, redirect_stdout
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("run_validation", Path(__file__).resolve().parents[1] / "run_validation.py")
validation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validation)


class ValidationWrapperTests(unittest.TestCase):
    def setUp(self):
        temp_root = Path(__file__).resolve().parents[2] / "Saved/Automation/PythonUnitTests"
        temp_root.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix="validation tests ", dir=temp_root)
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.ue = self.directory / "UE Cmd.exe"
        self.project = self.directory / "Test Project.uproject"
        self.ue.touch()
        self.project.touch()
        self.report = self.directory / "Saved/ValidationReports/ValidationReport.json"
        self.argv = ["--ue-cmd", str(self.ue), "--project", str(self.project)]

    def report_data(self, failed=0):
        return {"schema_version": 1, "tool": "ProductionPipelineToolkit", "scope": "/Game/Test",
                "asset_count": 1, "passed": 1-failed, "failed": failed, "skipped": 0, "results": [{}]}

    def write_report(self, report=None):
        self.report.parent.mkdir(parents=True, exist_ok=True)
        self.report.write_text(json.dumps(report if report is not None else self.report_data()), encoding="utf-8")

    def run_main(self, argv=None):
        with redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()) as err:
            code = validation.main(self.argv if argv is None else argv)
        return code, out.getvalue(), err.getvalue()

    def test_command_defaults_and_spaces(self):
        command, cwd, report = validation.build_command(validation.parse_args(self.argv))
        self.assertEqual(command[:2], [str(self.ue), str(self.project)])
        self.assertEqual(cwd, self.directory)
        self.assertEqual(report, self.report)
        self.assertIn("-Scope=/Game/ValidationTest", command)
        self.assertIn("-BlueprintPrefix=BP_", command)
        self.assertIn(f"-ReportPath={self.report}", command)

    def test_custom_arguments_and_absolute_report(self):
        target = self.directory / "Absolute report.json"
        args = validation.parse_args(self.argv + ["--scope=/Game/Characters", "--blueprint-prefix=Test_", "--report-path", str(target)])
        command, _, report = validation.build_command(args)
        self.assertEqual(report, target)
        self.assertIn("-Scope=/Game/Characters", command)
        self.assertIn("-BlueprintPrefix=Test_", command)
        args.report_path = "Saved/Relative report.json"
        self.assertEqual(validation.build_command(args)[2], self.directory / args.report_path)

    def test_pass_and_rule_failure(self):
        for exit_code in (0, 1):
            with self.subTest(exit_code=exit_code):
                self.report.unlink(missing_ok=True)
                def execute(*args, **kwargs):
                    self.assertFalse(kwargs["shell"])
                    self.write_report(self.report_data(exit_code))
                    return subprocess.CompletedProcess(args[0], exit_code)
                with patch.object(validation.subprocess, "run", side_effect=execute):
                    code, out, err = self.run_main()
                self.assertEqual(code, exit_code)
                self.assertIn(f"FAIL={exit_code}", out)
                self.assertEqual(err, "")

    def test_execution_error_and_crash_do_not_read_old_report(self):
        self.write_report()
        original = self.report.read_bytes()
        for exit_code in (2, -1073741571):
            with self.subTest(exit_code=exit_code), patch.object(validation.subprocess, "run", return_value=subprocess.CompletedProcess([], exit_code)), patch.object(validation, "read_summary") as reader:
                code, out, err = self.run_main()
                self.assertEqual(code, 2)
                reader.assert_not_called()
                self.assertNotIn("PASS=", out)
                self.assertTrue(err)
        self.assertEqual(self.report.read_bytes(), original)

    def test_missing_or_unchanged_report(self):
        for existing in (False, True):
            with self.subTest(existing=existing):
                if existing:
                    self.write_report()
                with patch.object(validation.subprocess, "run", return_value=subprocess.CompletedProcess([], 0)):
                    code, _, err = self.run_main()
                self.assertEqual(code, 2)
                self.assertIn("fresh report", err)

    def test_launch_failure(self):
        with patch.object(validation.subprocess, "run", side_effect=OSError("cannot launch")):
            code, _, err = self.run_main()
        self.assertEqual(code, 2)
        self.assertIn("cannot launch", err)

    def test_missing_required_arguments(self):
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
            validation.main([])
        self.assertEqual(error.exception.code, 2)

    def test_bad_paths_and_blank_report_do_not_launch(self):
        cases = [self.argv + ["--ue-cmd", str(self.directory / "missing.exe")],
                 self.argv + ["--project", str(self.directory / "missing.uproject")],
                 self.argv + ["--report-path=   "]]
        for argv in cases:
            with self.subTest(argv=argv), patch.object(validation.subprocess, "run") as process:
                self.assertEqual(self.run_main(argv)[0], 2)
                process.assert_not_called()

    def test_invalid_reports(self):
        reports = [[], self.report_data() | {"schema_version": 2},
                   self.report_data() | {"schema_version": True},
                   self.report_data() | {"tool": "Other"},
                   self.report_data() | {"passed": -1},
                   self.report_data() | {"passed": True},
                   self.report_data() | {"passed": 0},
                   self.report_data() | {"results": []},
                   self.report_data() | {"scope": None}, self.report_data(1)]
        for report in reports:
            with self.subTest(report=report):
                self.write_report(report)
                with self.assertRaises(ValueError):
                    validation.read_summary(self.report, 0)
        self.report.write_text("not JSON", encoding="utf-8")
        with self.assertRaises(ValueError):
            validation.read_summary(self.report, 0)

    def test_malformed_current_report_returns_two(self):
        def execute(*args, **kwargs):
            self.write_report()
            self.report.write_text("broken JSON", encoding="utf-8")
            return subprocess.CompletedProcess([], 0)
        with patch.object(validation.subprocess, "run", side_effect=execute):
            code, out, err = self.run_main()
        self.assertEqual(code, 2)
        self.assertNotIn("PASS=", out)
        self.assertTrue(err)

    def test_bom_encodings_and_empty_run(self):
        report = self.report_data() | {"scope": "/Game/测试", "asset_count": 0, "passed": 0, "results": []}
        for encoding in ("utf-8-sig", "utf-16"):
            with self.subTest(encoding=encoding):
                self.report.parent.mkdir(parents=True, exist_ok=True)
                self.report.write_text(json.dumps(report, ensure_ascii=False), encoding=encoding)
                self.assertEqual(validation.read_summary(self.report, 0)[0], "/Game/测试")


if __name__ == "__main__":
    unittest.main()
