@echo off

mkdir build > nul 2>&1

set ARCH=64

if "%1"=="32" (
    set ARCH=32
)

if "%ARCH%"=="32" (
    call vcvars32.bat > nul 2>&1
) else (
    call vcvars64.bat > nul 2>&1
)

pushd build
cl -nologo -Oi -GR- -EHa- -W4 -FC -Z7 -wd4201 -wd4127 ../src/win32/win32-handmade-hero.cpp user32.lib gdi32.lib xinput.lib ../src/handmade-hero/handmade-hero.cpp
popd
