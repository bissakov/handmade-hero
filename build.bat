@echo off

mkdir build > nul 2>&1
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
pushd build
cl -Zi ../handmade-hero.cpp user32.lib gdi32.lib
popd
