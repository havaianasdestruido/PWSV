---
sidebar_position: 3
title: "CI/CD, Build Matrix & Releases"
description: "How the Build Plugins workflow produces Windows, macOS and Linux archives, and how to publish a release."
---

# CI/CD, Build Matrix & Releases

The repository builds the plugins on **every pull request and every push to
`main`**, and turns a version tag into a downloadable GitHub Release.

| Workflow | Trigger | What it does |
|---|---|---|
| [`build.yml`](https://github.com/havaianasdestruido/PWSV/blob/main/.github/workflows/build.yml) | push to `main`, `v*` tags, PRs, manual | Builds VST3 + CLAP + Standalone on Windows, macOS and Linux, verifies the archives and uploads them |
| [`deploy-docs.yml`](https://github.com/havaianasdestruido/PWSV/blob/main/.github/workflows/deploy-docs.yml) | push to `main` | Builds the Jekyll site + Docusaurus docs and deploys to GitHub Pages |
| [`label.yml`](https://github.com/havaianasdestruido/PWSV/blob/main/.github/workflows/label.yml) | `pull_request_target` | Applies labels from `.github/labeler.yml` |

---

## Build Matrix

| Job | Runner | Output |
|---|---|---|
| **Windows x64** | `windows-2022` (MSVC 2022, x64) | `PWSV-<version>-windows-x86_64.zip` |
| **macOS universal** | `macos-15` (arm64) | `PWSV-<version>-macos-universal.zip` |
| **Linux x64** | `ubuntu-22.04` | `PWSV-<version>-linux-x86_64.tar.gz` |

Notes:

- **Windows** is pinned to `windows-2022` so the toolchain stays Visual Studio
  2022 regardless of what `windows-latest` points at. JUCE generates a
  `<Plugin>.vst3/` folder containing `Contents/x86_64-win/<Plugin>.vst3`.
- **macOS** is built with `CMAKE_OSX_ARCHITECTURES="x86_64;arm64"`, so one
  archive runs natively on both Intel and Apple Silicon. The Xcode generator
  is used when available because it ad-hoc signs the bundles. Packages are
  archived with `ditto` (as `.zip`) so bundle metadata and the ad-hoc
  signature survive the round trip; without `ditto` the script falls back to a
  symlink-preserving `zipfile` archive.
- **Linux** is built on Ubuntu 22.04, the oldest still-supported runner image:
  binaries linked against its glibc keep working on newer distributions
  (22.04, 24.04, Debian 12, Fedora…). Building on `ubuntu-latest` instead
  would produce a plugin that refuses to load on older DAW machines.
- Each archive is accompanied by a `.sha256` checksum file.
- `softprops/action-gh-release` publishes the Release; no personal token is
  needed (the workflow uses the automatic `GITHUB_TOKEN` with `contents: write`).

:::note Pinned actions
Every `uses:` is pinned to a full commit SHA with the human-readable version in
a comment, so a compromised upstream tag cannot inject code into the build.
:::

---

## Artefact Layout

`scripts/package.py` copies the JUCE artefacts out of
`build/<target>_artefacts/Release/` into the same `VST3/`, `CLAP/`,
`Standalone/` layout the Windows installer expects:

```text
PWSV-1.1.0-linux-x86_64/
├── README.txt              install instructions + build metadata
├── VST3/
│   ├── Patos WebSocket VST Effect.vst3/       (Contents/x86_64-linux/*.so)
│   └── Patos WebSocket VST Generator.vst3/
├── CLAP/
│   ├── Patos WebSocket VST Effect.clap
│   └── Patos WebSocket VST Generator.clap
└── Standalone/
    ├── Patos WebSocket VST Effect
    └── Patos WebSocket VST Generator
```

Windows packages additionally contain `setup.bat` and `config.txt`, byte-for-byte
the files from `installer/` re-encoded with CRLF line endings so `cmd.exe` runs
them correctly. Every archive also contains a `README.txt` with install
instructions and build metadata.

:::note Embedded WebSocket server on Windows
`Source/WebSocketServer.h` forward-declares `fd_set` under `_WIN32`: `juce_core`
only pulls in `windows.h` with `WIN32_LEAN_AND_MEAN` (which does not declare
`fd_set`), so without that line MSVC rejects the private
`readFromClients(fd_set&)` member with
`error C2061: syntax error: identifier 'fd_set'`. On POSIX the header includes
`<sys/select.h>` instead.
:::

---

## Smoke Tests

`scripts/verify.py` unpacks every archive and asserts that:

1. both products are present in every format the build claims to produce;
2. the loadable binary exists inside each bundle
   (`Contents/*-linux/*.so`, `Contents/*-win/*.vst3`, `Contents/MacOS/…`);
3. the binaries are native for the platform (ELF / PE / Mach-O) and carry the
   expected machine type (x86_64, aarch64, arm64);
4. on macOS the binaries really contain **both** slices — a silently
   single-arch universal build fails CI instead of shipping.

A missing plugin or a wrong architecture fails the job, so a broken build never
reaches a Release.

---

## Publishing a Release

```bash
# 1. Bump the version in CMakeLists.txt (project(... VERSION x.y.z))
# 2. Commit, then tag and push
git tag v1.1.0
git push origin v1.1.0
```

The tag push runs the matrix, verifies the archives and creates the release
`PWSV 1.1.0` with all three archives plus checksums attached. The version is
derived from the tag (`v1.1.0` → `1.1.0`) and the CMake project version is
used for ordinary branch/PR builds.

A manual **Run workflow** can also publish: tick *upload_release* to attach the
archives to a `nightly` release.

The version is also stamped into the plugin bundles: `scripts/build.py` passes
`-DPWSV_VERSION=<version>`, which feeds `project(... VERSION ...)` in
`CMakeLists.txt` (`PWSV_VERSION` is a cache variable, default `1.1.0`).

---

## Running the Same Build Locally

The workflow calls exactly what you can call from a terminal, so CI failures
are reproducible:

```bash
python3 scripts/build.py                 # Release build + package into dist/
python3 scripts/build.py --config Debug  # faster local iteration
python3 scripts/build.py --target PatoWebSocketEffect_VST3 --no-pack
python3 scripts/build.py --clean         # wipe build/ first
python3 scripts/build.py --pack          # package what is already built

python3 scripts/verify.py --dist dist --platform linux
```

On Windows use `py -3 scripts\build.py` (or `npm run build:plugins`) from a
Developer PowerShell; the script picks the Visual Studio 2022 generator and the
`Release` configuration automatically.

`PWSV_ARCHITECTURES="x86_64;arm64"` produces a universal macOS binary;
`PWSV_GENERATOR="Ninja"` overrides generator auto-detection.

---

## Caching

`FetchContent` clones JUCE 8.0.6 and clap-juce-extensions into `build/_deps`,
which dominates a cold build. The workflow caches that folder keyed on
`hashFiles('CMakeLists.txt')` plus the runner OS and architecture: it is reused
as long as the pinned dependency revisions do not change, and refreshed
automatically when they do.
