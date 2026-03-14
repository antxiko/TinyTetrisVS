:: ____________________________________________________________________________
:: TiniTetris 4P — Build script
:: Copies sources to MSXgl tree, builds, copies versioned ROM back
:: ____________________________________________________________________________
@echo off
setlocal enabledelayedexpansion
cls

set MSXGL=C:\Users\Antxiko\Documents\MSXgl-1.2.17
set PROJ=%MSXGL%\projects\tinitetris4p
set SRC=%~dp0

:: Read version number
set /p VER=<"%SRC%version.txt"

:: Pad version to 2 digits
if !VER! LSS 10 (set VSTR=0!VER!) else (set VSTR=!VER!)

:: Copy source files to MSXgl project folder
echo [v!VSTR!] Syncing sources to MSXgl tree...
copy /Y "%SRC%*.c"  "%PROJ%\" >nul
copy /Y "%SRC%*.h"  "%PROJ%\" >nul
copy /Y "%SRC%*.asm" "%PROJ%\" >nul 2>nul
copy /Y "%SRC%project_config.js" "%PROJ%\" >nul

:: Build from MSXgl tree
pushd "%PROJ%"
"%MSXGL%\tools\build\Node\node.exe" "%MSXGL%\engine\script\js\build.js" target=%1
set BUILD_ERR=!ERRORLEVEL!
popd

:: If build succeeded, copy ROM with version and increment
if !BUILD_ERR!==0 (
    if exist "%PROJ%\out\tinitetris4p.rom" (
        copy /Y "%PROJ%\out\tinitetris4p.rom" "%SRC%tinitetris4p.rom" >nul
        copy /Y "%PROJ%\out\tinitetris4p.rom" "%SRC%tinitetris!VSTR!.rom" >nul
        echo ROM v!VSTR! saved as tinitetris!VSTR!.rom

        :: Increment version for next build
        set /a NEWVER=!VER!+1
        echo !NEWVER!> "%SRC%version.txt"
    )
)

endlocal
