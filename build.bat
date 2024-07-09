@echo off

mkdir build > nul 2>&1
call vcvars64.bat > nul 2>&1
pushd build
cl -W4 -FC -Zi -wd4201 -wd4127 ../src/win32/win32-handmade-hero.cpp user32.lib gdi32.lib xinput.lib ../src/handmade-hero/handmade-hero.cpp
popd
