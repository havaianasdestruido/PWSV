---
sidebar_position: 2
title: "Installer Architecture & 7z SFX"
description: "Deep dive into the Windows 7-Zip SFX installer and setup batch automation script."
---

# Installer Architecture & 7z SFX

PWSV provides a self-contained Windows installation system located in the `installer/` directory.

---

## Installer Files

```text
installer/
├── config.txt      - 7-Zip Self-Extracting Archive (SFX) configuration header
└── setup.bat       - Interactive administrator batch script
```

---

## `installer/config.txt`

The 7-Zip SFX configuration specifies the prompt and auto-executed script upon archive extraction:

```ini
;!@Install@!UTF-8!
Title="Pato's WebSocket VST Installer"
BeginPrompt="This will install Pato's WebSocket VST plugins.\n\nClick OK to continue, or Cancel to exit."
RunProgram="setup.bat"
;!@InstallEnd@!
```

---

## `installer/setup.bat` Automation

The batch installer performs the following operations:
1. **Admin Elevation Check**: Runs `net session` to detect elevated privileges. If standard user, triggers UAC prompt via PowerShell:
   ```cmd
   powershell -Command "Start-Process '%~f0' -Verb RunAs"
   ```
2. **Directory Verification**: Ensures `C:\Program Files\Common Files\VST3` and `C:\Program Files\Common Files\CLAP` exist.
3. **Interactive Menu**: Allows installing both Effect & Generator, Effect only, Generator only, or running full uninstallation.
4. **File Copying**: Uses `xcopy /E /I /Y` for VST3 folder bundles and `copy /Y` for CLAP single-file binaries.
5. **Safe Uninstaller**: Removes files and cleans up directories when requested.
