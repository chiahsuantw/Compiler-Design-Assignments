#!/usr/bin/python3

import subprocess
import sys
from argparse import ArgumentParser
from collections import abc
from enum import Enum, auto
from pathlib import Path
from typing import Dict, List

import colorama

DIR = Path(__file__).resolve().parent


class TestStatus(Enum):
    PASS = auto()
    FAIL = auto()
    SKIP = auto()


class Grader:
    class CaseType(Enum):
        BASIC = auto()
        ADVANCE = auto()
        HIDDEN = auto()

    CASE_DIR: Dict[CaseType, Path] = {
        CaseType.BASIC: DIR / "basic_cases",
        CaseType.ADVANCE: DIR / "advance_cases",
        CaseType.HIDDEN: DIR / "hidden_cases",
    }

    CASES: Dict[CaseType, Dict[int, str]] = {
        CaseType.BASIC: {
            1: "1_keywords1",
            2: "2_keywords2",
            3: "3_delimiters",
            4: "4_arithmetic",
            5: "5_logical",
            6: "6_relational",
            7: "7_identifier",
            8: "8_constant",
            9: "9_scientific_string",
            10: "10_comment",
            11: "11_pseudocomment",
        },
        CaseType.ADVANCE: {
            1: "1_pseudo_optS_1",
            2: "2_pseudo_optS_2",
            3: "3_pseudo_optT_1",
            4: "4_pseudo_optT_2",
            5: "5_comment_1",
            6: "6_comment_2",
            7: "7_operator",
            8: "8_zero",
            9: "9_float",
            10: "10_scientific",
            11: "11_string_1",
            12: "12_string_2",
            13: "13_string_3",
            14: "14_error_1",
            15: "15_error_2",
            16: "16_error_3",
            17: "17_error_4",
            18: "18_error_5",
        },
        CaseType.HIDDEN: {
            1: "1_keywords1",
            2: "2_keywords2",
            3: "3_delimiters",
            4: "4_arithmetic",
            5: "5_logical",
            6: "6_relational",
            7: "7_identifier",
            8: "8_constant",
            9: "9_scientific_string",
            10: "10_comment",
            11: "11_pseudocomment",
            12: "101_pseudo_optS_1",
            13: "102_pseudo_optS_2",
            14: "103_pseudo_optT_1",
            15: "104_pseudo_optT_2",
            16: "105_comment_1",
            17: "106_comment_2",
            18: "107_operator",
            19: "108_zero",
            20: "109_float",
            21: "110_scientific",
            22: "111_string_1",
            23: "112_string_2",
            24: "113_string_3",
            25: "114_error_1",
            26: "115_error_2",
            27: "116_error_3",
            28: "117_error_4",
            29: "118_error_5",
        },
    }

    UNUSED: float = 0.0
    CASE_SCORES: Dict[CaseType, List[float]] = {
        CaseType.BASIC: [UNUSED, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5],
        CaseType.ADVANCE: [UNUSED, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0, 1.0],
        CaseType.HIDDEN: [UNUSED, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0, 1.0],
    }

    RUN_ALL_CASES: int = 0

    def __init__(self, scanner: Path) -> None:
        self.scanner: Path = scanner
        self.case_ids_to_run: Dict[self.CaseType, abc.Set[int]] = {
            case_type: set() for case_type in self.CaseType
        }
        self.diff_result: str = ""
        self.output_dir: Path = DIR / "result"
        if not self.output_dir.exists():
            self.output_dir.mkdir()

    def set_case_ids_to_run_of(self, case_type: CaseType, case_id: int) -> None:
        """Sets the case IDs of the case type to run. If the ID is 0, all cases will be run."""
        if case_id == self.RUN_ALL_CASES:
            self.case_ids_to_run[case_type] = self.CASES[case_type].keys()
        else:
            if case_id not in self.CASES[case_type]:
                print(f"ERROR: Invalid {case_type.name.lower()} case ID {case_id}")
                exit(1)
            self.case_ids_to_run[case_type] = {case_id}

    def set_case_ids_to_run(self, basic_id: int, advance_id: int, hidden_id: int) -> None:
        """Sets the case IDs to run. If the ID is 0, all cases will be run."""
        self.set_case_ids_to_run_of(self.CaseType.BASIC, basic_id)
        self.set_case_ids_to_run_of(self.CaseType.ADVANCE, advance_id)
        self.set_case_ids_to_run_of(self.CaseType.HIDDEN, hidden_id)

    def should_be_skipped(self, case_type: CaseType, case_id: int) -> bool:
        """A test case should be skipped if it doesn't exist. This is to support hidden cases, which the students don't have."""
        test_case: Path = self.CASE_DIR[case_type] / "test_cases" / f"{self.CASES[case_type][case_id]}.p"
        return not test_case.exists()

    def gen_output(self, case_type: CaseType, case_id: int) -> None:
        test_case: Path = self.CASE_DIR[case_type] / "test_cases" / f"{self.CASES[case_type][case_id]}.p"
        output_file: Path = self.output_dir / f"{self.CASES[case_type][case_id]}"

        commands: List[str] = [str(self.scanner), str(test_case)]
        try:
            proc = subprocess.Popen(commands, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except Exception as e:
            print(f"Call of '{' '.join(commands)}' failed: {e}")
            return
        assert proc.stdout is not None and proc.stderr is not None
        stdout: bytes = proc.stdout.read()
        stderr: bytes = proc.stderr.read()
        _ = proc.wait()
        with output_file.open("wb") as out:
            out.write(stdout)
            out.write(stderr)

    def test_sample_case(self, case_type: CaseType, case_id: int) -> bool:
        self.gen_output(case_type, case_id)
        output_file: Path = self.output_dir / f"{self.CASES[case_type][case_id]}"
        solution: Path = self.CASE_DIR[case_type] / "sample_solutions" / f"{self.CASES[case_type][case_id]}"

        commands: List[str] = ["diff", "-u", str(output_file), str(solution), f"--label=your output:({output_file})", f"--label=answer:({solution})"]
        try:
            proc = subprocess.Popen(commands, stdout=subprocess.PIPE)
        except Exception as e:
            print(f"Call of '{' '.join(commands)}' failed: {e}")
            return False
        assert proc.stdout is not None
        output: str = proc.stdout.read().decode()
        retcode: int = proc.wait()
        if retcode != 0:
            # The header part.
            self.diff_result += f"{self.CASES[case_type][case_id]}\n"
            # The diff part.
            if case_type == self.CaseType.HIDDEN:
                self.diff_result += "// Diff of hidden cases is not shown.\n"
            else:
                self.diff_result += f"{output}\n"
        return retcode == 0

    def run(self) -> int:
        print("---\tCase\t\tPoints")

        # The score student gets.
        total_score: float = 0
        # The score student doesn't get to see.
        hidden_score: float = 0
        # The maximum score student can get, including hidden cases.
        max_score: float = 0

        for case_type, case_ids in self.case_ids_to_run.items():
            for case_id in case_ids:
                case_name: str = self.CASES[case_type][case_id]
                print(f"+++ TESTING {case_type.name.lower()} case {case_name}:")
                max_val: float = self.CASE_SCORES[case_type][case_id]
                get_val: float
                if self.should_be_skipped(case_type, case_id):
                    get_val = 0
                    hidden_score += self.CASE_SCORES[case_type][case_id]
                    self.set_text_color(TestStatus.SKIP)
                    modified_score = self.CASE_SCORES[case_type][case_id]
                    print(f"---\t{case_name}\tSKIPPED\t0/{modified_score}")
                else:
                    ok: bool = self.test_sample_case(case_type, case_id)
                    get_val = max_val if ok else 0
                    self.set_text_color(TestStatus.PASS if ok else TestStatus.FAIL)
                    modified_get_val = get_val
                    modified_max_val = max_val
                    print(f"---\t{case_name}\t{modified_get_val}/{modified_max_val}")
                self.reset_text_color()
                total_score += get_val
                max_score += max_val

        get_all_can_see: bool = total_score == max_score - hidden_score
        if get_all_can_see:
            self.set_text_color(TestStatus.PASS)
        else:
            self.set_text_color(TestStatus.FAIL)
        print(f"---\tTOTAL\t\t{total_score}/{max_score}")
        self.reset_text_color()

        with (self.output_dir / "score.txt").open("w") as result:
            result.write(f"---\tTOTAL\t\t{total_score}/{max_score}")
        (self.output_dir / "diff.txt").write_text(self.diff_result)

        # NOTE: Return 1 on test failure to support GitHub CI; otherwise, such CI never fails.
        if not get_all_can_see:
            return 1
        return 0

    @staticmethod
    def set_text_color(test_status: TestStatus) -> None:
        """Sets the color based on whether the test has passed or not."""
        if test_status is TestStatus.PASS:
            color = colorama.Fore.GREEN
        elif test_status is TestStatus.FAIL:
            color = colorama.Fore.RED
        else:
            color = colorama.Fore.YELLOW
        print(color, end='')

    @staticmethod
    def reset_text_color() -> None:
        print(colorama.Style.RESET_ALL, end='')


def main() -> int:
    parser = ArgumentParser()
    parser.add_argument("--scanner", help="scanner to grade", type=Path, default=DIR.parent / "src" / "scanner")
    parser.add_argument("--basic_case_id", help="basic case's ID", type=int, default=Grader.RUN_ALL_CASES)
    parser.add_argument("--advance_case_id", help="advance case's ID", type=int, default=Grader.RUN_ALL_CASES)
    parser.add_argument("--hidden_case_id", help="hidden case's ID", type=int, default=Grader.RUN_ALL_CASES)
    args = parser.parse_args()

    g = Grader(scanner=args.scanner)
    g.set_case_ids_to_run(args.basic_case_id, args.advance_case_id, args.hidden_case_id)
    return g.run()


if __name__ == "__main__":
    sys.exit(main())
