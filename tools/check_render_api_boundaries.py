#!/usr/bin/env python3
"""Validate render API usage boundaries during RenderBackend migration.

Usage examples:
  tools/check_render_api_boundaries.py --phase A
  tools/check_render_api_boundaries.py --phase all
  tools/check_render_api_boundaries.py --final
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = REPO_ROOT / "handheld" / "src"

CODE_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh", ".inl", ".mm"}

API_PATTERNS = [
    re.compile(r"\bgl[A-Za-z0-9_]*\s*\("),
    re.compile(r"\begl[A-Za-z0-9_]*\s*\("),
    re.compile(r"\bGX_[A-Z0-9_]+\b"),
    re.compile(r"\bGX[A-Za-z0-9_]*\s*\("),
    re.compile(r"\bGL_[A-Z0-9_]+\b"),
    re.compile(r"\bEGL_[A-Z0-9_]+\b"),
]

@dataclass(frozen=True)
class Phase:
    name: str
    description: str
    roots: tuple[str, ...]


PHASES = {
    "A": Phase(
        name="A",
        description="GUI / HUD / screens",
        roots=("handheld/src/client/gui",),
    ),
    "B": Phase(
        name="B",
        description="Entity / item renderers",
        roots=(
            "handheld/src/client/renderer/entity",
            "handheld/src/client/renderer/tileentity",
            "handheld/src/client/renderer/ItemInHandRenderer.cpp",
            "handheld/src/client/model",
        ),
    ),
    "C": Phase(
        name="C",
        description="Level / world rendering",
        roots=(
            "handheld/src/client/renderer/LevelRenderer.cpp",
            "handheld/src/client/renderer/Chunk.cpp",
            "handheld/src/client/renderer/RenderChunk.cpp",
            "handheld/src/client/renderer/TileRenderer.cpp",
            "handheld/src/client/renderer/GameRenderer.cpp",
            "handheld/src/client/renderer/Tesselator.cpp",
            "handheld/src/client/renderer/culling",
        ),
    ),
    "D": Phase(
        name="D",
        description="Particles / effects / debug overlays",
        roots=(
            "handheld/src/client/particle",
            "handheld/src/util/PerfRenderer.cpp",
            "handheld/src/util/PerfRenderer.h",
        ),
    ),
}

# These are the only files expected to contain platform graphics API calls after migration is complete.
FINAL_ALLOWED_API_PATHS = {
    "handheld/src/client/renderer/RenderBackend.cpp",
    "handheld/src/client/renderer/renderer_gx.cpp",
    "handheld/src/client/renderer/render_compat.h",
    "handheld/src/client/renderer/render_compat.cpp",
    "handheld/src/main_wii.cpp",
    "handheld/src/main_rpi.h",
    "handheld/src/App.h",
    "handheld/src/AppPlatform_android.h",
    "handheld/src/EglConfigPrinter.h",
}


def iter_source_files(root: Path):
    if root.is_file():
        if root.suffix in CODE_EXTENSIONS:
            yield root
        return

    for path in root.rglob("*"):
        if path.suffix in CODE_EXTENSIONS and path.is_file():
            yield path


def find_api_hits(paths: list[Path]):
    failures: list[tuple[str, int, str]] = []
    for path in sorted(paths):
        rel_path = path.relative_to(REPO_ROOT).as_posix()
        with path.open("r", encoding="utf-8", errors="ignore") as handle:
            for lineno, line in enumerate(handle, start=1):
                for pattern in API_PATTERNS:
                    if pattern.search(line):
                        failures.append((rel_path, lineno, line.rstrip()))
                        break
    return failures


def collect_phase_files(phase: Phase) -> list[Path]:
    files: list[Path] = []
    for rel_root in phase.roots:
        root = REPO_ROOT / rel_root
        files.extend(iter_source_files(root))
    return files


def run_phase(phase: Phase) -> int:
    hits = find_api_hits(collect_phase_files(phase))
    if hits:
        print(f"Phase {phase.name} FAILED ({phase.description}): found direct graphics API tokens")
        for rel_path, lineno, line in hits:
            print(f"  {rel_path}:{lineno}: {line}")
        return 1

    print(f"Phase {phase.name} OK ({phase.description}): zero direct graphics API usage")
    return 0


def run_final_scan() -> int:
    hits = find_api_hits(list(iter_source_files(SOURCE_ROOT)))
    unexpected = [hit for hit in hits if hit[0] not in FINAL_ALLOWED_API_PATHS]

    if unexpected:
        print("Final scan FAILED: found API-specific graphics calls outside backend implementation files")
        for rel_path, lineno, line in unexpected:
            print(f"  {rel_path}:{lineno}: {line}")
        return 1

    print("Final scan OK: API-specific graphics calls are restricted to backend implementation files")
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check RenderBackend boundary migration constraints")
    parser.add_argument(
        "--phase",
        choices=["A", "B", "C", "D", "all"],
        help="Run one migration phase check (or all phases)",
    )
    parser.add_argument(
        "--final",
        action="store_true",
        help="Run the final tree-wide backend-only API usage scan",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if not args.phase and not args.final:
        print("Nothing to do. Choose --phase <A|B|C|D|all> and/or --final.")
        return 2

    rc = 0
    if args.phase:
        phases = PHASES.values() if args.phase == "all" else (PHASES[args.phase],)
        for phase in phases:
            rc = max(rc, run_phase(phase))

    if args.final:
        rc = max(rc, run_final_scan())

    return rc


if __name__ == "__main__":
    sys.exit(main())
