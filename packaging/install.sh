#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# PWSV — install.sh
#
# Installs the VST3 / CLAP / Standalone builds shipped in the PWSV release
# package into the standard per-user plugin folders.
#
#   packaging/install.sh            install for the current user (no sudo)
#   packaging/install.sh --system   install system-wide (uses sudo when needed)
#   packaging/install.sh --dry-run  show what would be copied
#
# Mirrors installer/setup.bat on Windows.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

SCOPE="user"
DRY_RUN=0

usage() {
    sed -n '2,12p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    exit "${1:-0}"
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --system)  SCOPE="system" ;;
        --user)    SCOPE="user" ;;
        --dry-run) DRY_RUN=1 ;;
        -h|--help) usage 0 ;;
        *) echo "Unknown option: $1" >&2; usage 1 ;;
    esac
    shift
done

if [ "$(uname -s)" = "Darwin" ]; then
    if [ "$SCOPE" = "system" ]; then
        VST3_DIR="/Library/Audio/Plug-Ins/VST3"
        CLAP_DIR="/Library/Audio/Plug-Ins/CLAP"
    else
        VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
        CLAP_DIR="$HOME/Library/Audio/Plug-Ins/CLAP"
    fi
else
    if [ "$SCOPE" = "system" ]; then
        VST3_DIR="/usr/local/lib/vst3"
        CLAP_DIR="/usr/local/lib/clap"
    else
        VST3_DIR="$HOME/.vst3"
        CLAP_DIR="$HOME/.clap"
    fi
fi

run() {
    if [ "$DRY_RUN" -eq 1 ]; then
        echo "  [dry-run] $*"
    else
        "$@"
    fi
}

need_sudo() {
    [ "$SCOPE" = "system" ] && [ "$(id -u)" -ne 0 ]
}

echo "PWSV installer"
echo "  source : $SCRIPT_DIR"
echo "  scope  : $SCOPE"
echo "  VST3   : $VST3_DIR"
echo "  CLAP   : $CLAP_DIR"
echo

if [ -d "$SCRIPT_DIR/VST3" ]; then
    if need_sudo; then SUDO="sudo"; else SUDO=""; fi
    run $SUDO mkdir -p "$VST3_DIR"
    for bundle in "$SCRIPT_DIR/VST3"/*.vst3; do
        [ -e "$bundle" ] || continue
        echo "Installing $(basename "$bundle")"
        run $SUDO rm -rf "$VST3_DIR/$(basename "$bundle")"
        run $SUDO cp -R "$bundle" "$VST3_DIR/"
    done
fi

if [ -d "$SCRIPT_DIR/CLAP" ]; then
    if need_sudo; then SUDO="sudo"; else SUDO=""; fi
    run $SUDO mkdir -p "$CLAP_DIR"
    for plugin in "$SCRIPT_DIR/CLAP"/*.clap; do
        [ -e "$plugin" ] || continue
        echo "Installing $(basename "$plugin")"
        run $SUDO cp -R "$plugin" "$CLAP_DIR/"
    done
fi

echo
echo "Done. Restart your DAW so it rescans the plugin folders."
echo "The Standalone builds (if included) can be run directly from Standalone/."
