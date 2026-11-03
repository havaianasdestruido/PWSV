#!/usr/bin/env python3
"""Collect PWSV build artefacts into a ready-to-install release package.

The CMake build tree is the single source of truth: every artefact is looked up
through the ``<target>_artefacts/<config>/<FORMAT>/`` directories that JUCE and
clap-juce-extensions generate for us. Nothing is hard-coded to a compiler or an
IDE, so the same script works for Ninja, Makefiles, Xcode and MSVC builds.

Usage
-----
    python scripts/package.py --platform linux --config Release \
        --version 1.1.0 --commit abc1234

Result::

    dist/
      PWSV-1.1.0-linux-x86_64/          <- staging directory
        README.txt
        VST3/Patos WebSocket VST Effect.vst3
        CLAP/Patos WebSocket VST Effect.clap
        Standalone/Patos WebSocket VST Effect
        ...
      PWSV-1.1.0-linux-x86_64.tar.gz
      PWSV-1.1.0-linux-x86_64.tar.gz.sha256
"""

from __future__ import annotations

import argparse
import datetime as _datetime
import hashlib
import os
import platform as _platform
import shutil
import sys
import tarfile
import zipfile
from pathlib import Path

# (short name, CMake target, PRODUCT_NAME used by juce_add_plugin)
PLUGINS = (
    ("Effect", "PatoWebSocketEffect", "Patos WebSocket VST Effect"),
    ("Generator", "PatoWebSocketGenerator", "Patos WebSocket VST Generator"),
)

FORMATS = ("VST3", "CLAP", "Standalone")

# Artefacts produced by the "Standalone" format are executables, not bundles.
STANDALONE_SUFFIX = {
    "windows": ".exe",
    "macos": ".app",
    "linux": "",
}


def log(message: str) -> None:
    print(f"[package] {message}", flush=True)


def artefact_dir(build_dir: Path, target: str, config: str) -> Path:
    return build_dir / f"{target}_artefacts" / config


def find_artefact(root: Path, fmt: str, product_name: str, platform_name: str) -> Path | None:
    """Return the artefact produced for ``fmt`` or ``None`` when it was not built."""
    folder = root / fmt

    if fmt == "Standalone":
        candidate = folder / f"{product_name}{STANDALONE_SUFFIX[platform_name]}"
        # A Linux standalone binary has no extension; be forgiving about it.
        if not candidate.exists() and platform_name == "linux":
            candidate = folder / product_name
        return candidate if candidate.exists() else None

    candidate = folder / f"{product_name}.{fmt.lower()}"
    return candidate if candidate.exists() else None


def copy_artefact(src: Path, dst: Path) -> None:
    """Copy a file, a bundle or a directory tree (macOS bundles contain symlinks)."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    if src.is_dir():
        shutil.copytree(src, dst, symlinks=True)
    else:
        shutil.copy2(src, dst)


def sha256_of(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def readme_text(platform_name: str, version: str, commit: str, timestamp: str,
                counts: dict[str, int]) -> str:
    lines = [
        "Patos WebSocket VST (PWSV) - " + version,
        f"{platform_name}/{_platform.machine()} build - commit {commit} - {timestamp}",
        "",
        "High-performance WebSocket DAW telemetry & MIDI streaming plugin suite",
        "(VST3 + CLAP + Standalone). All plugins open a WebSocket server on",
        "ws://127.0.0.1:8080 and speak protocol v2.",
        "",
        "WHAT IS IN THIS PACKAGE",
        f"  VST3/        {counts['VST3']} plugin bundle(s)",
        f"  CLAP/        {counts['CLAP']} plugin bundle(s)",
        f"  Standalone/  {counts['Standalone']} standalone app(s) - run without a DAW",
        "",
    ]

    if platform_name == "windows":
        lines += [
            "INSTALL",
            "  Run setup.bat and choose what to install (it will ask for admin rights).",
            "",
            "  Manual install:",
            r"    VST3  ->  C:\Program Files\Common Files\VST3",
            r"    CLAP  ->  C:\Program Files\Common Files\CLAP",
            "    (create the CLAP folder if it does not exist yet)",
            "",
            "  The plugins are unsigned; SmartScreen may warn on first run.",
            "",
        ]
    elif platform_name == "macos":
        lines += [
            "INSTALL",
            "  Copy the bundles to your user plug-in folders:",
            "    mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/CLAP",
            "    cp -R VST3/* ~/Library/Audio/Plug-Ins/VST3/",
            "    cp -R CLAP/* ~/Library/Audio/Plug-Ins/CLAP/",
            "  Use /Library/Audio/Plug-Ins/... instead for an all-users install.",
            "  The standalone apps can be dragged into /Applications.",
            "",
            "  This build is ad-hoc signed but NOT notarised. If macOS refuses to load",
            "  a plug-in, clear the quarantine flag:",
            '    xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"'
            + PLUGINS[0][2] + '.vst3"',
            "",
        ]
    else:
        lines += [
            "INSTALL",
            "  User-wide install (no root required):",
            "    mkdir -p ~/.vst3 ~/.clap",
            "    cp -R VST3/* ~/.vst3/",
            "    cp -R CLAP/* ~/.clap/",
            "  System-wide install: /usr/lib/vst3 and /usr/lib/clap.",
            "",
            "  The standalone binary needs the audio/desktop packages from the docs",
            "  (libasound2, libx11-6, libfreetype6, libxinerama1, libxcursor1,",
            "  libxrandr2, libgl1). Run it with:",
            "    ./Standalone/" + PLUGINS[0][2],
            "",
        ]

    lines += [
        "SOURCES / LICENSE",
        "  https://github.com/havaianasdestruido/PWSV - MIT licensed.",
        "  Bundled dependencies: JUCE 8 (see the JUCE licence) and CLAP.",
        "",
    ]
    return "\n".join(lines)


def stage_package(args, build_dir: Path) -> tuple[Path, dict[str, int]]:
    package_name = f"PWSV-{args.version}-{args.platform}-{args.arch}"
    stage = args.dist / package_name
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    counts = {fmt: 0 for fmt in FORMATS}
    missing: list[str] = []

    for label, target, product_name in PLUGINS:
        root = artefact_dir(build_dir, target, args.config)
        if not root.is_dir():
            missing.append(f"{label}: build directory {root} does not exist")
            continue

        for fmt in FORMATS:
            src = find_artefact(root, fmt, product_name, args.platform)
            if src is None:
                missing.append(f"{label}: no {fmt} artefact in {root / fmt}")
                continue
            copy_artefact(src, stage / fmt / src.name)
            counts[fmt] += 1
            log(f"staged {fmt:<10} {src.name}")

    # Windows ships the interactive helper installer next to the plug-ins; its
    # setup.bat expects VST3\ and CLAP\ to be siblings, which is exactly the
    # layout of the staging directory.
    if args.platform == "windows":
        installer_src = args.repo_root / "installer"
        if installer_src.is_dir():
            installer_dst = stage / "installer"
            installer_dst.mkdir()
            for name in ("setup.bat", "config.txt"):
                source = installer_src / name
                if source.is_file():
                    # Batch files must reach Windows with CRLF line endings.
                    text = source.read_text(encoding="utf-8").replace("\r\n", "\n")
                    (installer_dst / name).write_bytes(
                        text.replace("\n", "\r\n").encode("utf-8"))
            shutil.move(str(installer_dst / "setup.bat"), str(stage / "setup.bat"))
            shutil.move(str(installer_dst / "config.txt"), str(stage / "config.txt"))
            installer_dst.rmdir()
            log("staged     windows installer (setup.bat)")

    # Standalone binaries must stay executable after unpacking a tarball.
    for artefact in (stage / "Standalone").glob("*") if (stage / "Standalone").is_dir() else []:
        if artefact.is_file():
            artefact.chmod(artefact.stat().st_mode | 0o755)

    timestamp = _datetime.datetime.now(_datetime.timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    (stage / "README.txt").write_text(
        readme_text(args.platform, args.version, args.commit, timestamp, counts),
        encoding="utf-8",
    )

    log("summary: " + ", ".join(f"{fmt}={count}" for fmt, count in counts.items()))
    for message in missing:
        log(("WARNING " if args.allow_missing else "error: ") + message)

    if sum(counts.values()) == 0:
        raise SystemExit("error: no artefacts found - did the build run before packaging?")
    if missing and not args.allow_missing:
        raise SystemExit(
            f"error: {len(missing)} artefact(s) missing; pass --allow-missing to "
            "package a partial build anyway")
    if missing:
        (stage / "INCOMPLETE.txt").write_text(
            "This package is missing artefacts:\n" + "\n".join(missing) + "\n",
            encoding="utf-8")

    return stage, counts


def write_zip(stage: Path, archive: Path) -> None:
    timestamp = _datetime.datetime.now().timetuple()[:6]
    with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
        for path in sorted(stage.rglob("*")):
            arcname = Path(stage.name) / path.relative_to(stage)
            if path.is_dir() and not path.is_symlink():
                # VST3 bundles are folder structures, so directory entries have
                # to be written explicitly - including a sane mode, otherwise
                # unzip(1) on other platforms creates them unreadable.
                info = zipfile.ZipInfo(str(arcname) + "/", date_time=timestamp)
                info.external_attr = (0o40755 << 16) | 0x10  # drwxr-xr-x + DOS dir bit
                info.compress_type = zipfile.ZIP_STORED
                zf.writestr(info, b"")
            elif path.is_file():
                zf.write(path, str(arcname))


def write_tar_gz(stage: Path, archive: Path) -> None:
    with tarfile.open(archive, "w:gz") as tf:
        tf.add(stage, arcname=stage.name, recursive=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--platform", required=True, choices=sorted(STANDALONE_SUFFIX))
    parser.add_argument("--arch", default=None,
                        help="architecture tag used in the archive name (default: detected)")
    parser.add_argument("--build-dir", type=Path, default=Path("build"))
    parser.add_argument("--dist", type=Path, default=Path("dist"))
    parser.add_argument("--config", default="Release",
                        help="CMake configuration name (default: Release)")
    parser.add_argument("--version", default="0.0.0-dev",
                        help="version string used in the package name")
    parser.add_argument("--commit", default=os.environ.get("GITHUB_SHA", "unknown")[:7])
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument("--no-archive", action="store_true",
                        help="only stage the directory, do not create the archive")
    parser.add_argument("--allow-missing", action="store_true",
                        help="package what exists even if a format was not built "
                             "(default: any missing artefact is an error)")
    args = parser.parse_args(argv)

    args.arch = args.arch or _platform.machine().lower().replace("amd64", "x86_64")
    build_dir = args.build_dir.resolve()
    args.dist = args.dist.resolve()
    args.dist.mkdir(parents=True, exist_ok=True)

    stage, _counts = stage_package(args, build_dir)

    if not args.no_archive:
        if args.platform == "windows":
            archive = args.dist / f"{stage.name}.zip"
            write_zip(stage, archive)
        else:
            archive = args.dist / f"{stage.name}.tar.gz"
            write_tar_gz(stage, archive)

        digest = sha256_of(archive)
        (archive.with_name(archive.name + ".sha256")).write_text(
            f"{digest}  {archive.name}\n", encoding="utf-8")
        log(f"archive {archive.name} ({archive.stat().st_size / (1024 * 1024):.1f} MiB)")
        log(f"sha256  {digest}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
