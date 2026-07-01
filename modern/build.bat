@echo off
REM Baut Spaceflight.exe mit MinGW-w64 (gcc muss im PATH sein).
REM WinLibs/MinGW liefert die noetigen d3d11/dxgi/d3dcompiler-Header und -Libs.

where gcc >nul 2>nul
if errorlevel 1 (
  echo FEHLER: gcc nicht im PATH gefunden.
  echo Installiere MinGW-w64, z.B.:  winget install BrechtSanders.WinLibs.POSIX.UCRT
  echo und oeffne danach eine neue Eingabeaufforderung.
  exit /b 1
)

REM sprites.h bei Bedarf neu aus den Original-.PAS-Dateien erzeugen:
REM   py tools\extract_sprites.py ..\SPACDATA.PAS sprites.h

gcc -std=c11 -O2 -mwindows ^
    game.c gfx.c platform_win.c ^
    -o Spaceflight.exe ^
    -ld3d11 -ldxgi -ld3dcompiler -ldxguid -lgdi32 -luser32 -lwinmm

if errorlevel 1 (
  echo BUILD FEHLGESCHLAGEN.
  exit /b 1
)
echo BUILD OK  ->  Spaceflight.exe
