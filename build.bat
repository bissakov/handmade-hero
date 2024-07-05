@echo off

mkdir build > nul 2>&1
call vcvars64.bat > nul 2>&1
pushd build
cl -FC -Zi ../win32-handmade-hero.cpp user32.lib gdi32.lib xinput.lib
popd
