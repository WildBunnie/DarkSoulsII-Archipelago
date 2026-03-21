@echo off
setlocal enabledelayedexpansion

if "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    set "ARCH=x64"
    set "DS2_VERSION=sotfs"
) else if "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    set "ARCH=x86"
    set "DS2_VERSION=vanilla"
) else (
    echo MSVC environment not initialized
    exit /b
)

:: create release folder
mkdir "release\%DS2_VERSION%\archipelago\assets" >nul 2>&1

:: build
call build.bat release
copy "build\%ARCH%\archipelago.dll" "release\%DS2_VERSION%\archipelago\" /Y >nul

:: copy only the assets that are tracked by git (ignore project stuff)
git ls-files "assets\%DS2_VERSION%" > filelist.txt
for /f "delims=" %%f in (filelist.txt) do (
    set "file_path=%%f"
    set "file_path=!file_path:/=\!"
    set "relative_path=!file_path:\%DS2_VERSION%\=\\!"
    set "target=release\%DS2_VERSION%\archipelago\!relative_path!"

    for %%T in ("!target!") do set "target_dir=%%~dpT"
    mkdir "!target_dir!" >nul 2>&1
    copy "!file_path!" "!target!" /Y >nul
)

:: use our .ini cause the one in modengine doesnt point to archipelago folder
copy "ds2modengine.ini" "release\%DS2_VERSION%\" /Y >nul

:: modenigne
call git clone https://github.com/WildBunnie/ds2-modengine
cd ds2-modengine
call build.bat
copy "build\%ARCH%\dinput8.dll" "..\release\%DS2_VERSION%\" /Y >nul
cd ..
rmdir /s /q ds2-modengine