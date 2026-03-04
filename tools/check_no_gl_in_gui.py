#!/usr/bin/env python3
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
GUI_ROOT = REPO_ROOT / "handheld" / "src" / "client" / "gui"

FORBIDDEN_PATTERNS = [
    re.compile(r"\bglEnable\w*\s*\("),
    re.compile(r"\bglDisable\w*\s*\("),
    re.compile(r"\bglMatrixMode\w*\s*\("),
    re.compile(r"\bGL_[A-Z0-9_]+\b"),
]

ALLOWLIST = {
    "handheld/src/client/gui/GuiRenderContext.cpp": [re.compile(r".*")],
    "handheld/src/client/gui/Gui.cpp": [re.compile(r"\bGL_TRIANGLE_FAN\b")],
    "handheld/src/client/gui/screens/TextEditScreen.cpp": [re.compile(r"\bGL_QUADS\b")],
}


def is_allowed(rel_path: str, line: str) -> bool:
    rules = ALLOWLIST.get(rel_path)
    if not rules:
        return False
    return any(rule.search(line) for rule in rules)


def main() -> int:
    failures = []
    for path in sorted(GUI_ROOT.rglob("*")):
        if path.suffix not in {".cpp", ".h"}:
            continue
        rel_path = path.relative_to(REPO_ROOT).as_posix()
        with path.open("r", encoding="utf-8") as f:
            for lineno, line in enumerate(f, start=1):
                for pattern in FORBIDDEN_PATTERNS:
                    if pattern.search(line) and not is_allowed(rel_path, line):
                        failures.append((rel_path, lineno, line.rstrip()))
                        break

    if failures:
        print("Found forbidden GL tokens in GUI sources:")
        for rel_path, lineno, line in failures:
            print(f"  {rel_path}:{lineno}: {line}")
        return 1

    print("OK: no forbidden GL tokens found in handheld/src/client/gui/**")
    return 0


if __name__ == "__main__":
    sys.exit(main())
