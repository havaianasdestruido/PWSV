#!/usr/bin/env python3
"""Configure, build and package PWSV with CMake - on Windows, macOS or Linux.

This is the same entry point CI uses, so a local run reproduces the release
artefacts exactly. Only Python 3.9 and CMake 3.22+ are required; the compiler is
picked per platform (MSVC on Windows, clang on macOS, gcc/clang on Linux).

Examples
--------
    python scripts/build.py                  # Release, everything, packaged
    python scripts/build.py --config Debug   # quick local iteration
    python scripts/build.py --pack           # only stage + archive, no rebuild
    python scripts/build.py --clean          # wipe the build directory first

Artefacts land in ``build/<target>_artefacts/<Config>/`` and the release
archives in ``dist/``.
"""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CMAKE_MINIMUM = (3, 22)

# Packages JUCE's Linux modules need at link time.
LINUX_PACKAGES = (
    "libasound2-dev libjack-jackd2-dev libx11-dev libxext-dev libxinerama-dev "
    "libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev"
)


def log(message: str) -> None:
    print(f"[build] {message}", flush=True)


def run(cmd: list[str], cwd: Path | None = None) -> int:
    log("$ " + " ".join(str(part) for part in cmd))
    return subprocess.call(cmd, cwd=str(cwd or REPO_ROOT))


def check_cmake() -> str:
    cmake = shutil.which("cmake")
    if cmake is None:
        raise SystemExit(
            "error: cmake was not found on PATH (CMake 3.22 or newer is required).\n"
            "       Linux: sudo apt install cmake   macOS: brew install cmake   "
            "Windows: winget install Kitware.CMake"
        )
    output = subprocess.run([cmake, "--version"], capture_output=True, text=True).stdout
    try:
        version = tuple(int(part) for part in output.split()[2].split(".")[:3])
    except (IndexError, ValueError):
        return cmake
    if version[:2] < CMAKE_MINIMUM:
        raise SystemExit(
            f"error: CMake {output.split()[2]} is too old, {CMAKE_MINIMUM[0]}.{CMAKE_MINIMUM[1]}"
            " or newer is required."
        )
    return cmake


def detect_generator(cmake: str) -> tuple[str, list[str]]:
    """Pick a generator that exists on this machine and return (name, extra args)."""
    requested = os.environ.get("PWSV_GENERATOR")
    if requested:
        return requested, []

    if platform.system() == "Windows":
        # The Visual Studio generator is multi-config and is the one the MSVC
        # toolchain is tested against; fall back to NMake/Ninja if it is absent.
        help_text = subprocess.run([cmake, "--help"], capture_output=True, text=True).stdout
        if "Visual Studio 17 2022" in help_text:
            return "Visual Studio 17 2022", ["-A", "x64"]
        if shutil.which("ninja"):
            return "Ninja", []
        raise SystemExit("error: neither Visual Studio 2022 nor Ninja was found.")

    if platform.system() == "Darwin":
        # The Xcode generator ad-hoc signs bundles and produces real .app files;
        # use it when it is available, Ninja/Makefiles otherwise.
        return "Xcode" if shutil.which("xcodebuild") else "Ninja", []

    return "Ninja" if shutil.which("ninja") else "Unix Makefiles", []


def check_linux_dependencies() -> None:
    """Fail early with the exact apt command instead of a wall of compiler errors."""
    marker = Path("/usr/include/alsa/asoundlib.h")
    if marker.exists():
        return
    log("warning: ALSA/X11 development headers were not found.")
    log("         install them with: sudo apt-get install -y " + LINUX_PACKAGES)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--config", default="Release", choices=("Debug", "Release", "RelWithDebInfo"))
    parser.add_argument("--build-dir", type=Path, default=REPO_ROOT / "build")
    parser.add_argument("--dist", type=Path, default=REPO_ROOT / "dist")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    parser.add_argument("--target", action="append", dest="targets",
                        help="build only this CMake target (can be repeated)")
    parser.add_argument("--clean", action="store_true", help="delete the build directory first")
    parser.add_argument("--configure-only", action="store_true")
    parser.add_argument("--no-pack", action="store_true", help="skip packaging into dist/")
    parser.add_argument("--pack", action="store_true",
                        help="only package artefacts that are already built")
    parser.add_argument("--allow-missing", action="store_true",
                        help="package a partial build instead of failing")
    parser.add_argument("--arch", default=None,
                        help="architecture tag for the package name (default: detected)")
    parser.add_argument("--version", default=None,
                        help="version for the build + package name (default: from CMakeLists.txt)")
    args = parser.parse_args(argv)

    system = {"Windows": "windows", "Darwin": "macos"}.get(platform.system(), "linux")
    build_dir: Path = args.build_dir.resolve()

    if args.clean and build_dir.exists():
        log(f"removing {build_dir}")
        shutil.rmtree(build_dir)

    if not args.pack:
        cmake = check_cmake()
        if system == "linux":
            check_linux_dependencies()

        os.environ.setdefault("PWSV_CONFIG", args.config)
        generator, generator_args = detect_generator(cmake)
        log(f"generator: {generator}  config: {args.config}  python: {platform.python_version()}")

        configure = [cmake, "-S", str(REPO_ROOT), "-B", str(build_dir), "-G", generator] + generator_args
        if generator in ("Ninja", "Unix Makefiles"):
            # Single-config generators need the configuration at configure time.
            configure.append(f"-DCMAKE_BUILD_TYPE={args.config}")

        # Universal binaries on macOS: PWSV_ARCHITECTURES="x86_64;arm64"
        architectures = os.environ.get("PWSV_ARCHITECTURES", "").strip()
        if architectures and system == "macos":
            configure.append(f"-DCMAKE_OSX_ARCHITECTURES={architectures}")
            log(f"macOS architectures: {architectures}")

        # Stamp the package version into the bundles' metadata (Info.plist /
        # VST3 version) instead of always shipping the CMakeLists.txt version.
        build_version = args.version or read_version()
        configure.append(f"-DPWSV_VERSION={build_version}")
        log(f"version: {build_version}")
        if run(configure) != 0:
            return 1

        if args.configure_only:
            return 0

        build = [cmake, "--build", str(build_dir), "--config", args.config, "--parallel", str(args.jobs)]
        for target in args.targets or []:
            build += ["--target", target]
        if run(build) != 0:
            return 1

    if args.no_pack:
        log("skipping packaging (--no-pack)")
        return 0

    version = args.version or read_version()
    package = [
        sys.executable, str(REPO_ROOT / "scripts" / "package.py"),
        "--platform", system,
        "--build-dir", str(build_dir),
        "--dist", str(args.dist),
        "--config", args.config,
        "--version", version,
    ]
    if args.arch:
        package += ["--arch", args.arch]
    if args.allow_missing:
        package.append("--allow-missing")
    if run(package) != 0:
        return 1

    log(f"done - artefacts are in {args.dist}")
    return 0


def read_version() -> str:
    """Read the project version out of CMakeLists.txt without needing CMake."""
    import re

    text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(r"project\s*\([^)]*?VERSION\s+([0-9][0-9.]*)", text, re.DOTALL)
    if match:
        return match.group(1).rstrip(".")

    # Never hand a non-numeric version to CMake: `project(... VERSION 0.0.0-dev)`
    # is a hard configure error.
    log("warning: could not read project(... VERSION ...) from CMakeLists.txt; "
        "falling back to 0.0.0")
    return "0.0.0"


if __name__ == "__main__":
    sys.exit(main())
