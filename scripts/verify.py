#!/usr/bin/env python3
"""Sanity-check a packaged PWSV release before it is uploaded.

CI runs this after ``scripts/build.py``. It checks that every plugin format the
CMake build claims to produce really made it into the archive, that bundles have
their expected internal structure, and that the native binaries have the right
machine type (catches e.g. a macOS build that silently lost its arm64 slice, or
a Linux build that accidentally linked the wrong architecture).

Exit code 0 = everything looks good, 1 = a check failed.
"""

from __future__ import annotations

import argparse
import platform
import shutil
import struct
import sys
import tarfile
import zipfile
from pathlib import Path

PRODUCTS = (
    "Patos WebSocket VST Effect",
    "Patos WebSocket VST Generator",
)

ELF_MACHINES = {0x3E: "x86_64", 0xB7: "aarch64"}
PE_MACHINES = {0x8664: "x86_64", 0xAA64: "arm64"}
MACHO_CPUS = {0x01000007: "x86_64", 0x0100000C: "arm64", 0x00000007: "i386", 0x0000000C: "arm"}

FAILURES: list[str] = []


def ok(message: str) -> None:
    print(f"  ok    {message}")


def fail(message: str) -> None:
    print(f"  FAIL  {message}")
    FAILURES.append(message)


def warn(message: str) -> None:
    print(f"  warn  {message}")


# ── minimal binary inspection (no external tools needed) ────────────────────

def elf_machine(path: Path) -> int | None:
    try:
        with path.open("rb") as handle:
            header = handle.read(20)
    except OSError:
        return None
    if header[:4] != b"\x7fELF":
        return None
    endian = "<" if header[5] == 1 else ">"
    return struct.unpack_from(endian + "H", header, 18)[0]


def pe_machine(path: Path) -> int | None:
    try:
        with path.open("rb") as handle:
            dos = handle.read(64)
            if dos[:2] != b"MZ":
                return None
            offset = struct.unpack_from("<I", dos, 60)[0]
            handle.seek(offset)
            coff = handle.read(6)
    except (OSError, struct.error):
        return None
    if coff[:4] != b"PE\0\0":
        return None
    return struct.unpack_from("<H", coff, 4)[0]


def macho_arches(path: Path) -> list[str] | None:
    """Return the architectures in a Mach-O binary, fat or thin."""
    try:
        with path.open("rb") as handle:
            header = handle.read(8)
            if len(header) < 8:
                return None
            magic = struct.unpack(">I", header[:4])[0]

            if magic in (0xCAFEBABE, 0xCAFEBABF):
                is64 = magic == 0xCAFEBABF
                count = struct.unpack(">I", header[4:8])[0]
                arches = []
                for _ in range(count):
                    entry = handle.read(32 if is64 else 20)
                    cpu = struct.unpack(">I", entry[:4])[0]
                    arches.append(MACHO_CPUS.get(cpu, hex(cpu)))
                return arches
    except (OSError, struct.error):
        return None

    if magic in (0xFEEDFACE, 0xFEEDFACF):
        return [MACHO_CPUS.get(struct.unpack(">I", header[4:8])[0], "unknown")]
    if magic in (0xCEFAEDFE, 0xCFFAEDFE):
        return [MACHO_CPUS.get(struct.unpack("<I", header[4:8])[0], "unknown")]
    return None


def check_native(path: Path, platform_name: str, label: str) -> None:
    """Assert that ``path`` is a native binary of this platform and report its arch."""
    if platform_name == "linux":
        machine = elf_machine(path)
        if machine is None:
            fail(f"{label}: not an ELF binary")
        elif machine in ELF_MACHINES:
            ok(f"{label} (ELF {ELF_MACHINES[machine]})")
        else:
            fail(f"{label}: unexpected ELF machine {machine:#x}")
    elif platform_name == "windows":
        machine = pe_machine(path)
        if machine is None:
            fail(f"{label}: not a PE/COFF binary")
        elif machine in PE_MACHINES:
            ok(f"{label} (PE {PE_MACHINES[machine]})")
        else:
            fail(f"{label}: unexpected PE machine {machine:#x}")
    else:
        arches = macho_arches(path)
        if arches is None:
            fail(f"{label}: not a Mach-O binary")
        else:
            ok(f"{label} (Mach-O {', '.join(arches)})")


# ── layout helpers ──────────────────────────────────────────────────────────

def vst3_binary(bundle: Path, platform_name: str, product: str) -> Path | None:
    """Locate the loadable binary inside a VST3 bundle (folder on every OS)."""
    contents = bundle / "Contents"
    if platform_name == "linux":
        matches = sorted(contents.glob("*-linux/*.so"))
    elif platform_name == "windows":
        matches = sorted(contents.glob("*-win/*.vst3"))
    else:
        binary = contents / "MacOS" / product
        return binary if binary.is_file() else None
    return matches[0] if matches else None


def clap_binary(bundle: Path, platform_name: str, product: str) -> Path | None:
    """CLAP is a single file on Windows/Linux and a bundle on macOS."""
    if platform_name == "macos":
        binary = bundle / "Contents" / "MacOS" / product
        return binary if binary.is_file() else None
    return bundle if bundle.is_file() else None


def standalone_binary(path: Path, platform_name: str, product: str) -> Path | None:
    if platform_name == "macos":
        binary = path / "Contents" / "MacOS" / product
        return binary if binary.is_file() else None
    return path if path.is_file() else None


# ── archive handling ────────────────────────────────────────────────────────

def extract(archive: Path, destination: Path) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    if archive.suffix == ".zip":
        with zipfile.ZipFile(archive) as zf:
            zf.extractall(destination)
    else:
        with tarfile.open(archive) as tf:
            # Python 3.12+ wants an explicit filter; older versions do not
            # understand the keyword at all.
            try:
                tf.extractall(destination, filter="data")
            except TypeError:
                tf.extractall(destination)


def host_platform() -> str:
    return {"Windows": "windows", "Darwin": "macos"}.get(platform.system(), "linux")


def verify_stage(stage: Path, platform_name: str) -> None:
    print(f"verifying {stage.name}")
    standalone_suffix = {"windows": ".exe", "macos": ".app", "linux": ""}[platform_name]
    checked = 0

    for product in PRODUCTS:
        for fmt in ("VST3", "CLAP", "Standalone"):
            if fmt == "VST3":
                artefact = stage / "VST3" / f"{product}.vst3"
            elif fmt == "CLAP":
                artefact = stage / "CLAP" / f"{product}.clap"
            else:
                artefact = stage / "Standalone" / f"{product}{standalone_suffix}"

            if not artefact.exists():
                if fmt == "Standalone":
                    warn(f"{product}: no standalone build in this package")
                else:
                    fail(f"{product}: missing {fmt} artefact")
                continue

            if fmt == "VST3":
                binary = vst3_binary(artefact, platform_name, product)
                label = f"{artefact.name}: VST3 bundle"
            elif fmt == "CLAP":
                binary = clap_binary(artefact, platform_name, product)
                label = f"{artefact.name}: CLAP"
            else:
                binary = standalone_binary(artefact, platform_name, product)
                label = f"{artefact.name}: standalone"

            if binary is None:
                fail(f"{label} is incomplete (no loadable binary inside)")
                continue

            checked += 1
            check_native(binary, platform_name, label)

    if checked == 0:
        fail("no artefacts were found in the package")


def verify_universal(stage: Path) -> None:
    """macOS only: confirm the binaries really do contain both slices."""
    binaries = []
    for product in PRODUCTS:
        binaries += [
            stage / "VST3" / f"{product}.vst3" / "Contents" / "MacOS" / product,
            stage / "CLAP" / f"{product}.clap" / "Contents" / "MacOS" / product,
            stage / "Standalone" / f"{product}.app" / "Contents" / "MacOS" / product,
        ]

    for binary in binaries:
        if not binary.is_file():
            continue
        arches = macho_arches(binary) or []
        missing = {"x86_64", "arm64"} - set(arches)
        if missing:
            fail(f"{binary.name}: missing {sorted(missing)} slice (has {arches})")
        else:
            ok(f"{binary.name}: universal ({', '.join(arches)})")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--dist", type=Path, default=Path("dist"))
    parser.add_argument("--platform", default=host_platform(), choices=("windows", "macos", "linux"))
    parser.add_argument("--universal", action="store_true",
                        help="require x86_64 + arm64 slices in macOS binaries")
    parser.add_argument("--keep", action="store_true", help="keep the extraction directory")
    args = parser.parse_args(argv)

    packages = [
        path for path in sorted(args.dist.glob("PWSV-*"))
        if path.name.endswith((".zip", ".tar.gz"))
    ]
    # A dist/ folder may hold several platforms at once; only look at our own.
    archives = [path for path in packages if f"-{args.platform}-" in path.name]
    if not archives:
        print(f"error: no {args.platform} packages found in {args.dist.resolve()}"
              f" (saw: {[path.name for path in packages] or 'nothing'})")
        return 1

    for archive in archives:
        name = archive.name.removesuffix(".tar.gz").removesuffix(".zip")
        work = args.dist / f".verify-{name}"
        if work.exists():
            shutil.rmtree(work)
        extract(archive, work)

        stages = [path for path in work.iterdir() if path.is_dir() and path.name.startswith("PWSV-")]
        if not stages:
            fail(f"{archive.name}: archive does not contain a PWSV-* directory")
        else:
            verify_stage(stages[0], args.platform)
            if args.platform == "macos" and args.universal:
                verify_universal(stages[0])

        (Path(archive).with_name(archive.name + ".sha256")).exists() or warn(
            f"{archive.name}: no .sha256 checksum next to the archive")
        if not args.keep:
            shutil.rmtree(work, ignore_errors=True)

    print()
    if FAILURES:
        print(f"{len(FAILURES)} check(s) failed:")
        for message in FAILURES:
            print(f"  - {message}")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
