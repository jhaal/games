@echo off
REM Pac-Man Game - Runtime Verification Script

echo.
echo ========================================
echo Pac-Man Game - Verification Report
echo ========================================
echo.

REM Check executable
if exist "x64\Debug\pacman.exe" (
    echo [OK] Executable found: pacman.exe
    for /F "tokens=*" %%A in ('wmic datafile where name="C:\\Users\\alok jha\\source\\repos\\games\\pacman\\x64\\Debug\\pacman.exe" get FileSize ^| findstr [0-9]') do (
        echo      Size: %%A bytes
    )
) else (
    echo [ERROR] Executable not found!
    goto end
)

echo.
echo Checking SFML Runtime Dependencies:
echo.

REM Check DLLs
setlocal enabledelayedexpansion
set "dlls=sfml-graphics-d-2.dll sfml-window-d-2.dll sfml-system-d-2.dll sfml-audio-d-2.dll openal32.dll"
set "missing=0"

for %%D in (!dlls!) do (
    if exist "x64\Debug\%%D" (
        echo [OK] %%D
    ) else (
        echo [MISSING] %%D
        set /a missing=!missing!+1
    )
)

echo.
if !missing! equ 0 (
    echo [SUCCESS] All runtime dependencies are present!
    echo.
    echo You can now run the game:
    echo   - From Visual Studio: Press F5
    echo   - From Command Line: x64\Debug\pacman.exe
) else (
    echo [WARNING] !missing! DLL^(s^) missing - game may not run
)

endlocal

echo.
echo Checking SFML Library Files:
echo.

if exist "SFML\include\SFML\Graphics.hpp" (
    echo [OK] SFML headers present
) else (
    echo [ERROR] SFML headers not found
)

if exist "SFML\lib\sfml-graphics-d.lib" (
    echo [OK] SFML debug libraries present
) else (
    echo [ERROR] SFML debug libraries not found
)

:end
echo.
echo ========================================
echo Verification Complete
echo ========================================
echo.
pause
