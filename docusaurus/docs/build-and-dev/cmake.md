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

```bash
# 1. Clone the repository
git clone https://github.com/havaianasdestruido/PWSV.git
cd PWSV

# 2. Configure build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Compile targets
cmake --build build --config Release -j
```

---

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
