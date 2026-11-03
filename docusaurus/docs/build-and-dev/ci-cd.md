---
sidebar_position: 3
title: "CI/CD: Build Workflow & Automated Labeling"
description: "How the GitHub Actions build pipeline produces Windows, macOS and Linux plugin packages, and how PRs are labeled."
---

# CI/CD: Build Workflow & Automated Labeling

The repository ships two GitHub Actions workflows:

| Workflow | File | Purpose |
|---|---|---|
| **Build** | `.github/workflows/build.yml` | Compiles both plugins for Windows, macOS and Linux, packages them, and (on tags) drafts a release. |
| **Build and Deploy Documentation** | `.github/workflows/deploy-docs.yml` | Builds Jekyll + Docusaurus and publishes GitHub Pages. |
| **Labeler** | `.github/workflows/label.yml` | Categorizes pull requests by changed files. |

---

## Build Workflow (`.github/workflows/build.yml`)

### Build matrix

| Job | Runner | Generator | Output architecture |
|---|---|---|---|
| `windows` | `windows-2022` | Visual Studio 17 2022 (`-A x64`) | x64 |
| `macos` | `macos-15` (Apple Silicon) | Unix Makefiles | **universal** (`x86_64;arm64`) |
| `linux` | `ubuntu-22.04` | Ninja | x86_64 (glibc 2.35 baseline) |

Every job builds **both** targets, `PatoWebSocketEffect` and `PatoWebSocketGenerator`, in
**VST3**, **CLAP** and **Standalone** formats — exactly what `juce_add_plugin` +
`clap_juce_extensions_plugin` declare in `CMakeLists.txt`.

:::note macOS universal binaries
The macOS job runs on an arm64 runner but configures
`-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"`, so every `.vst3`/`.clap`/`.app` binary is a
single universal Mach-O that loads natively on Intel and Apple Silicon. The build log
prints `lipo -archs` for each produced binary so the universal slice set is verifiable
from CI.
:::

### Triggers

| Event | Behavior |
|---|---|
| `push` to `main` or `arena/**` | Build + upload artifacts |
| `pull_request` | Build + upload artifacts (in-progress runs are cancelled on new pushes) |
| `push` of tag `v*` | Build + **draft GitHub Release** with all packages attached |
| `workflow_dispatch` | Manual build; set the `version` input to also draft a release |

### Version resolution

The `meta` job derives the package version, in order of precedence:

1. `workflow_dispatch` input `version`
2. Git tag: `v1.2.0` → `1.2.0`
3. `project(... VERSION x.y.z)` in `CMakeLists.txt`
4. Fallback `0.0.0`

### Package layout

```text
PWSV-<version>-windows-x64.zip
PWSV-<version>-macos-universal.zip
PWSV-<version>-linux-x86_64.tar.gz

└── PWSV-<version>-<platform>-<arch>/
    ├── VST3/          "Patos WebSocket VST Effect.vst3", "… Generator.vst3"
    ├── CLAP/          "Patos WebSocket VST Effect.clap", "… Generator.clap"
    ├── Standalone/    standalone executables / .app bundles (macOS, Linux, Windows)
    └── install.sh     Linux/macOS per-user installer (Windows zip ships setup.bat)
```

Install scripts:

- **Windows** — the existing `installer/setup.bat` (elevates via UAC, copies to
  `%PROGRAMFILES%\Common Files\VST3` and `…\CLAP`).
- **macOS / Linux** — `packaging/install.sh`:
  ```bash
  ./install.sh            # ~/Library/Audio/Plug-Ins/{VST3,CLAP}  or  ~/.vst3 + ~/.clap
  ./install.sh --system   # /Library/Audio/Plug-Ins/...           or  /usr/local/lib/...
  ./install.sh --dry-run  # print the copy operations only
  ```

### Fetching build artifacts

Artifacts are kept for 14 days:

```bash
# via the GitHub CLI, for the latest run on the current branch
gh run download --name PWSV-linux-x86_64
gh run download --name PWSV-macos-universal
gh run download --name PWSV-windows-x64
```

### Cutting a release

```bash
git tag v1.1.0
git push origin v1.1.0
```

The three build jobs run, then the `release` job downloads the packages, collects the
`.zip`/`.tar.gz` files and creates a **draft** release (with generated notes) tagged
`v1.1.0`. Review it in the Releases tab and publish when ready.

:::tip Pinned actions
Every `uses:` reference is pinned to a full commit SHA with the release tag in a comment,
matching the convention already used in `deploy-docs.yml`.
:::

---

## Local equivalents

The workflow steps are plain CMake invocations, so the same build can be reproduced
locally:

```bash
# Windows (Developer PowerShell)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel

# macOS — universal
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release --parallel

# Linux
sudo apt-get install -y libasound2-dev libjack-jackd2-dev libfreetype-dev \
  libfontconfig1-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
  libxinerama-dev libxrandr-dev libxrender-dev libglu1-mesa-dev mesa-common-dev ninja-build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Binaries land in `build/<Target>_artefacts/Release/<VST3|CLAP|Standalone>/`, which is the
layout the workflow stages from.

---

## Platform notes

- **Linux CI images ship no audio/X11 dev packages** — the `linux` job installs JUCE's
  [Linux dependencies](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md)
  before configuring. `pkg-config` (or `pkgconf`) must be present or JUCE's CMake
  configuration fails with `Could NOT find PkgConfig`.
- **Ubuntu 22.04 instead of `ubuntu-latest`** deliberately targets an older glibc so the
  produced `.vst3`/`.clap` binaries load on more distributions.
- **Windows** needs the sockets headers for the embedded WebSocket server; `ws2_32` is
  linked in `CMakeLists.txt` and the socket types (`fd_set`) are forward-declared in
  `Source/WebSocketServer.h` so MSVC can parse the class declarations without a prior
  `<winsock2.h>` include.

---

## Labeler Workflow (`.github/workflows/label.yml`)

The labeler triggers on `pull_request_target`:

```yaml
name: Labeler
on: [pull_request_target]

jobs:
  label:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      pull-requests: write
    steps:
    - uses: actions/labeler@8558fd74291d67161a8a78ce36a881fa63b766a9 # v5.0.0
      with:
        repo-token: "${{ secrets.GITHUB_TOKEN }}"
```

:::security Pull Request Target Safety
Because `pull_request_target` workflows run with write permissions in the context of the base repository, this workflow only inspects PR file paths for labeling and never checks out or executes untrusted code from pull request heads.
:::

---

## Category Mapping (`.github/labeler.yml`)

File changes are automatically labeled across multiple domains:
- **`build`**: `CMakeLists.txt`, `cmake/**`, `*.bat`
- **`ci`**: `.github/workflows/**`, `.gitignore`
- **`documentation`**: `*.md`, `docs/**`, `LICENSE`
- **`vst-plugin`**: `src/**`, `Source/**`
- **`websocket`**: `Source/WebSocket*`
- **`ui`**: `Source/*Editor*`
- **`audio`**: `Source/*Processor*`
- **`windows` / `macos` / `linux`**: Platform-specific changes
