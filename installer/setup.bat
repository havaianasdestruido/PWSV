@echo off
setlocal enabledelayedexpansion
title Pato's WebSocket VST Installer
color 0B

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Requesting administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:MENU
cls
echo.
echo  ============================================
echo   Pato's WebSocket VST Installer v1.0
echo  ============================================
echo.
echo  Select components to install:
echo.
echo    [1] Both Effect and Generator
echo    [2] Effect Only
echo    [3] Generator Only
echo    [4] Uninstall All
echo    [5] Exit
echo.
set /p choice="  Enter choice [1-5]: "

if "%choice%"=="1" goto INSTALL_BOTH
if "%choice%"=="2" goto INSTALL_EFFECT
if "%choice%"=="3" goto INSTALL_GENERATOR
if "%choice%"=="4" goto UNINSTALL
if "%choice%"=="5" goto EXIT
echo  Invalid choice. Press any key to try again...
pause >nul
goto MENU

:INSTALL_BOTH
set INSTALL_EFFECT=1
set INSTALL_GENERATOR=1
goto INSTALL

:INSTALL_EFFECT
set INSTALL_EFFECT=1
set INSTALL_GENERATOR=0
goto INSTALL

:INSTALL_GENERATOR
set INSTALL_EFFECT=0
set INSTALL_GENERATOR=1
goto INSTALL

:INSTALL
cls
echo.
echo  Installing Pato's WebSocket VST...
echo.

set VST3_DIR=C:\Program Files\Common Files\VST3
set CLAP_DIR=C:\Program Files\Common Files\CLAP

if not exist "%VST3_DIR%" mkdir "%VST3_DIR%"
if not exist "%CLAP_DIR%" mkdir "%CLAP_DIR%"

set "SRC=%~dp0"

if "%INSTALL_EFFECT%"=="1" (
    echo  [1/4] Installing Effect VST3...
    if exist "%SRC%VST3\Patos WebSocket VST Effect.vst3" (
        xcopy /E /I /Y "%SRC%VST3\Patos WebSocket VST Effect.vst3" "%VST3_DIR%\Patos WebSocket VST Effect.vst3" >nul
        echo         Done.
    ) else (
        echo         Source not found, skipping.
    )

    echo  [2/4] Installing Effect CLAP...
    if exist "%SRC%CLAP\Patos WebSocket VST Effect.clap" (
        copy /Y "%SRC%CLAP\Patos WebSocket VST Effect.clap" "%CLAP_DIR%\" >nul
        echo         Done.
    ) else (
        echo         Source not found, skipping.
    )
) else (
    echo  [1/4] Effect: skipped.
    echo  [2/4] Effect: skipped.
)

if "%INSTALL_GENERATOR%"=="1" (
    echo  [3/4] Installing Generator VST3...
    if exist "%SRC%VST3\Patos WebSocket VST Generator.vst3" (
        xcopy /E /I /Y "%SRC%VST3\Patos WebSocket VST Generator.vst3" "%VST3_DIR%\Patos WebSocket VST Generator.vst3" >nul
        echo         Done.
    ) else (
        echo         Source not found, skipping.
    )

    echo  [4/4] Installing Generator CLAP...
    if exist "%SRC%CLAP\Patos WebSocket VST Generator.clap" (
        copy /Y "%SRC%CLAP\Patos WebSocket VST Generator.clap" "%CLAP_DIR%\" >nul
        echo         Done.
    ) else (
        echo         Source not found, skipping.
    )
) else (
    echo  [3/4] Generator: skipped.
    echo  [4/4] Generator: skipped.
)

echo.
echo  ============================================
echo   Installation complete!
echo  ============================================
echo.
echo  Installed to:
if "%INSTALL_EFFECT%"=="1" (
    echo    - Effect  -^> %VST3_DIR%
    echo    - Effect  -^> %CLAP_DIR%
)
if "%INSTALL_GENERATOR%"=="1" (
    echo    - Generator -^> %VST3_DIR%
    echo    - Generator -^> %CLAP_DIR%
)
echo.
echo  Please restart your DAW to detect the new plugins.
echo.
pause
goto EXIT

:UNINSTALL
cls
echo.
echo  Uninstalling Pato's WebSocket VST...
echo.

set VST3_DIR=C:\Program Files\Common Files\VST3
set CLAP_DIR=C:\Program Files\Common Files\CLAP

echo  Removing Effect VST3...
if exist "%VST3_DIR%\Patos WebSocket VST Effect.vst3" (
    rmdir /S /Q "%VST3_DIR%\Patos WebSocket VST Effect.vst3"
    echo     Done.
) else echo     Not found.

echo  Removing Effect CLAP...
if exist "%CLAP_DIR%\Patos WebSocket VST Effect.clap" (
    del /Q "%CLAP_DIR%\Patos WebSocket VST Effect.clap"
    echo     Done.
) else echo     Not found.

echo  Removing Generator VST3...
if exist "%VST3_DIR%\Patos WebSocket VST Generator.vst3" (
    rmdir /S /Q "%VST3_DIR%\Patos WebSocket VST Generator.vst3"
    echo     Done.
) else echo     Not found.

echo  Removing Generator CLAP...
if exist "%CLAP_DIR%\Patos WebSocket VST Generator.clap" (
    del /Q "%CLAP_DIR%\Patos WebSocket VST Generator.clap"
    echo     Done.
) else echo     Not found.

echo.
echo  ============================================
echo   Uninstall complete!
echo  ============================================
echo.
pause
goto EXIT

:EXIT
endlocal
exit /b 0
