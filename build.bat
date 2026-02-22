@echo off

if "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    set ARCH=x64
    set IMGUI_BACKEND=../../dependencies/imgui/backends/imgui_impl_dx11.cpp
) else if "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    set ARCH=x86
    set IMGUI_BACKEND=../../dependencies/imgui/backends/imgui_impl_dx9.cpp
) else (
    echo MSVC environment not initialized
    exit /b
)

set BUILD=debug
if /i "%1"=="release" (
    set BUILD=release
)

set DEFINES=/DASIO_STANDALONE /D_WEBSOCKETPP_CPP11_INTERNAL_ /DAP_NO_SCHEMA /DNOMINMAX
if "%BUILD%"=="debug" (
    set DEFINES=%DEFINES% /DAPCLIENT_DEBUG /DPRINT_DEBUG_STUFF
)

mkdir build\%ARCH%
pushd build\%ARCH%
    cl ../../src/dllmain.cpp ../../src/overlay.cpp ^
        crypt32.lib advapi32.lib user32.lib ws2_32.lib Shell32.lib d3d9.lib ^
        ../../dependencies/imgui/*.cpp ^
        ../../dependencies/imgui/backends/imgui_impl_win32.cpp ^
        %IMGUI_BACKEND% ^
        ../../dependencies/kiero/kiero.cpp ^
        ../../dependencies/MinHook/libMinHook.%ARCH%.lib ^
        ../../dependencies/openssl/%ARCH%/libcrypto.lib ^
        ../../dependencies/openssl/%ARCH%/libssl.lib ^
        ../../dependencies/zlib/%ARCH%/zlib.lib ^
        /I"../../dependencies/minhook/include" ^
        /I"../../dependencies/apclientpp" ^
        /I"../../dependencies/json/include" ^
        /I"../../dependencies/wswrap/include" ^
        /I"../../dependencies/asio/include" ^
        /I"../../dependencies/websocketpp" ^
        /I"../../dependencies/openssl" ^
        /I"../../dependencies/zlib" ^
        /I"../../dependencies/imgui" /I"../../dependencies/imgui/backends"^
        /I"../../dependencies/kiero"^
        /LD /MT /EHsc /std:c++17 ^
        %DEFINES% ^
        /Fe:archipelago.dll ^
        /link /LTCG
popd