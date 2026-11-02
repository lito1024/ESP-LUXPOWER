#!/usr/bin/env python3
"""Build and run the ESP-030 host test for LuxPowerTCP holding block 2
(tools/test_holding_block2.cpp + lib/LuxPowerTCP/src/LuxPowerTCP.cpp, no ARDUINO).

Compiler: g++ / clang++ from PATH, otherwise `python -m ziglang c++` (pip install
ziglang into the ESPHome venv outside Drive). The binary goes to a temp dir,
never into the repository.

Usage:
    python tools/run_holding_block2_tests.py
"""
from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
SOURCES = [TOOLS / "test_holding_block2.cpp", TOOLS.parent / "lib" / "LuxPowerTCP" / "src" / "LuxPowerTCP.cpp"]


def compiler() -> list[str]:
    for name in ("g++", "clang++"):
        path = shutil.which(name)
        if path:
            return [path]
    try:
        import ziglang  # noqa: F401
    except ImportError:
        sys.exit("no C++ compiler: install g++/clang++ or `pip install ziglang`")
    return [sys.executable, "-m", "ziglang", "c++", "-Wno-nullability-completeness"]


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        exe = Path(tmp) / ("test_holding_block2.exe" if sys.platform == "win32" else "test_holding_block2")
        build = compiler() + ["-std=c++17", "-O1", *map(str, SOURCES), "-o", str(exe)]
        if subprocess.run(build).returncode != 0:
            print("build failed")
            return 1
        return subprocess.run([str(exe)]).returncode


if __name__ == "__main__":
    sys.exit(main())
