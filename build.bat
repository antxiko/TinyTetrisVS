:: ____________________________________________________________________________
:: TiniTetris 4P — Build script
:: Copies src/* to MSXgl tree, builds, copies versioned ROM into builds/
:: ____________________________________________________________________________
@echo off
setlocal enabledelayedexpansion
cls

set MSXGL=C:\Users\Antxiko\Documents\MSXgl-1.4.1
set PROJ=%MSXGL%\projects\tinitetris4p
set ROOT=%~dp0
set SRC=%ROOT%src
set BUILDS=%ROOT%builds

:: Read version number
set /p VER=<"%ROOT%version.txt"

:: Pad version to 2 digits
if !VER! LSS 10 (set VSTR=0!VER!) else (set VSTR=!VER!)

:: Copy source files to MSXgl project folder
echo [v!VSTR!] Syncing sources to MSXgl tree...
copy /Y "%SRC%\*.c"   "%PROJ%\" >nul
copy /Y "%SRC%\*.h"   "%PROJ%\" >nul
copy /Y "%SRC%\*.asm" "%PROJ%\" >nul 2>nul
copy /Y "%SRC%\project_config.js" "%PROJ%\" >nul

:: Build from MSXgl tree
pushd "%PROJ%"
"%MSXGL%\tools\build\Node\node.exe" "%MSXGL%\engine\script\js\build.js" target=%1
set BUILD_ERR=!ERRORLEVEL!
popd

:: If build succeeded, copy ROM with version and increment
if !BUILD_ERR!==0 (
    if exist "%PROJ%\out\tinitetris4p.rom" (
        if not exist "%BUILDS%" mkdir "%BUILDS%"
        copy /Y "%PROJ%\out\tinitetris4p.rom" "%BUILDS%\tinitetris4p.rom" >nul
        copy /Y "%PROJ%\out\tinitetris4p.rom" "%BUILDS%\tinitetris!VSTR!.rom" >nul
        echo ROM v!VSTR! saved as builds\tinitetris!VSTR!.rom

        :: Increment version for next build
        set /a NEWVER=!VER!+1
        echo !NEWVER!> "%ROOT%version.txt"

        :: Prune: keep only the 5 most recent versioned ROMs.
        :: Skip tinitetris4p.rom — it's the tracked "latest" pointer and would
        :: also match tinitetris??.rom (the "4p" passes the "??" mask).
        set COUNT=0
        for /f "delims=" %%f in ('dir /b /o-n "%BUILDS%\tinitetris??.rom" 2^>nul') do (
            if /I not "%%f"=="tinitetris4p.rom" (
                set /a COUNT+=1
                if !COUNT! GTR 5 del "%BUILDS%\%%f" >nul 2>&1
            )
        )
    )
)

endlocal
