#!/usr/bin/env bash
# Baut Spaceflight.exe mit MinGW-w64 (gcc muss im PATH sein).
set -e
cd "$(dirname "$0")"

if ! command -v gcc >/dev/null 2>&1; then
  echo "FEHLER: gcc nicht gefunden. MinGW-w64 installieren, z.B.:"
  echo "  winget install BrechtSanders.WinLibs.POSIX.UCRT"
  exit 1
fi

# sprites.h bei Bedarf neu erzeugen:
#   python tools/extract_sprites.py ../SPACDATA.PAS sprites.h

gcc -std=c11 -O2 -mwindows \
    game.c gfx.c platform_win.c \
    -o Spaceflight.exe \
    -ld3d11 -ldxgi -ld3dcompiler -ldxguid -lgdi32 -luser32 -lwinmm

echo "BUILD OK  ->  Spaceflight.exe"
