---
sidebar_position: 2
title: "Installation Guide"
description: "Detailed installation instructions for Windows, macOS, and Linux."
---

# Installation Guide

PWSV can be installed using the automated Windows batch installer or by manually placing the built plugin binaries into your operating system's standard audio plugin directories.

---

## Windows Automated Installer

PWSV includes an interactive installer in the `installer/` directory:

1. Navigate to the `installer/` directory.
2. Right-click `setup.bat` and select **Run as Administrator** (or double-click the self-extracting archive if using a release package).
3. The installer requests UAC privileges and displays an interactive menu:

```text
 ============================================
  Pato's WebSocket VST Installer v1.0
 ============================================

 Select components to install:

   [1] Both Effect and Generator
   [2] Effect Only
   [3] Generator Only
   [4] Uninstall All
   [5] Exit

 Enter choice [1-5]:
```

4. Choose `1` to install both plugins. The installer will copy the VST3 bundle and CLAP binaries into:
   - **VST3 Directory**: `C:\Program Files\Common Files\VST3\`
   - **CLAP Directory**: `C:\Program Files\Common Files\CLAP\`

---

## Manual Installation Paths

If you build from source or prefer manual installation, place the compiled files in the appropriate folders:

### Windows

| Format | File Name | Destination Directory |
|---|---|---|
| **VST3** | `Patos WebSocket VST Effect.vst3` | `C:\Program Files\Common Files\VST3\` |
| **VST3** | `Patos WebSocket VST Generator.vst3` | `C:\Program Files\Common Files\VST3\` |
| **CLAP** | `Patos WebSocket VST Effect.clap` | `C:\Program Files\Common Files\CLAP\` |
| **CLAP** | `Patos WebSocket VST Generator.clap` | `C:\Program Files\Common Files\CLAP\` |

### macOS

| Format | Destination Directory (System-wide) | Destination Directory (User) |
|---|---|---|
| **VST3** | `/Library/Audio/Plug-Ins/VST3/` | `~/Library/Audio/Plug-Ins/VST3/` |
| **CLAP** | `/Library/Audio/Plug-Ins/CLAP/` | `~/Library/Audio/Plug-Ins/CLAP/` |

### Linux

| Format | Destination Directory (System-wide) | Destination Directory (User) |
|---|---|---|
| **VST3** | `/usr/lib/vst3/` or `/usr/local/lib/vst3/` | `~/.vst3/` |
| **CLAP** | `/usr/lib/clap/` or `/usr/local/lib/clap/` | `~/.clap/` |

---

## Uninstalling

### Windows Automated Uninstall
Run `installer/setup.bat` as Administrator and select option `[4] Uninstall All`.

### Manual Uninstall
Delete the `.vst3` folders and `.clap` binary files from the respective system directories listed above, then restart your DAW.
