---
sidebar_position: 1
title: "Building from Source (CMake)"
description: "Prerequisites, CMake configuration, FetchContent dependencies, and compilation instructions for all platforms."
---

# Building from Source (CMake)

PWSV uses **CMake 3.22+** with modern `FetchContent` to download and configure **JUCE 8.0.6** and **clap-juce-extensions** automatically during the build.

---

## Prerequisites

- **CMake**: Version 3.22 or higher.
- **C++ Compiler**: Supporting **C++17**:
  - **Windows**: Visual Studio 2022 (MSVC) or Clang.
  - **macOS**: Xcode 14+ / Apple Clang.
  - **Linux**: GCC 10+ or Clang 12+ along with ALSA/X11 dev packages:
    ```bash
    sudo apt-get install libasound2-dev libjack-jackd2-dev libx11-dev \
      libxinerama-dev libxext-dev libfreetype6-dev libgl1-mesa-dev
    ```
- **Git**: Required for CMake `FetchContent` to clone dependencies.

---

## Build Steps

The quickest path is the cross-platform helper, which configures, builds and
packages the plugins for the current OS:

```bash
git clone https://github.com/havaianasdestruido/PWSV.git
cd PWSV

python3 scripts/build.py            # Windows: py -3 scripts\build.py
```

Artefacts end up in `build/<target>_artefacts/Release/` and the release
archives in `dist/`.

Or drive CMake yourself:

```bash
# 1. Configure build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compile targets
cmake --build build --config Release -j

# 3. Package the artefacts (optional)
python3 scripts/package.py --platform linux --build-dir build --config Release
```

### Per-target builds

```bash
# CMake targets created by juce_add_plugin
cmake --build build --target PatoWebSocketEffect_VST3    --config Release
cmake --build build --target PatoWebSocketEffect_CLAP    --config Release
cmake --build build --target PatoWebSocketGenerator_Standalone --config Release
```

### Output layout

| Path | Contents |
|---|---|
| `build/<target>_artefacts/Release/VST3/` | `<Product>.vst3` (bundle/folder) |
| `build/<target>_artefacts/Release/CLAP/` | `<Product>.clap` (bundle on macOS) |
| `build/<target>_artefacts/Release/Standalone/` | `<Product>`, `<Product>.exe` or `<Product>.app` |
| `dist/PWSV-<version>-<platform>-<arch>/` | staged package with `README.txt` |
| `dist/PWSV-<version>-<platform>-<arch>.(zip\|tar.gz)` | release archive + `.sha256` |

:::tip Universal macOS builds
Pass `-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"` (or set
`PWSV_ARCHITECTURES` when using `scripts/build.py`) to ship a single binary for
Intel and Apple Silicon.
:::

---

## Build Helper Options

`scripts/build.py` wraps the commands above:

| Flag | Effect |
|---|---|
| `--config Debug\|Release\|RelWithDebInfo` | Build configuration (default `Release`) |
| `--clean` | Delete `build/` before configuring |
| `--target NAME` | Build a single CMake target (repeatable) |
| `--configure-only` | Configure without compiling |
| `--no-pack` | Skip the packaging step |
| `--pack` | Only package artifacts that are already built |
| `--arch TAG` | Architecture tag used in the archive name |
| `PWSV_GENERATOR` | Force a CMake generator (e.g. `Ninja`) |
| `PWSV_ARCHITECTURES` | macOS universal build, e.g. `"x86_64;arm64"` |

## CMake Configuration Breakdown

### Dependencies (`FetchContent`)
```cmake
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG main # Pin to a specific release tag or commit hash for production stability
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(clap-juce-extensions)
```

### Compiler Definitions
To minimize binary size and exclude unnecessary browser/curl runtimes:
```cmake
target_compile_definitions(PatoWebSocketEffect PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
```

### Windows Socket Linking
On Windows builds, `ws2_32` is linked for TCP networking:
```cmake
if(WIN32)
    target_link_libraries(PatoWebSocketEffect PRIVATE ws2_32)
    target_link_libraries(PatoWebSocketGenerator PRIVATE ws2_32)
endif()
```
