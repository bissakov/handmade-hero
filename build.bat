@echo off

call vcvarsall.bat x64 > nul 2>&1
pushd build
cl -D DEV=1 -D DEBUG=1 -nologo -Oi -GR- -EHa- -MT -Gm- -Od -W4 -WX -wd4201 -wd4127 -wd4100 -FC -Z7 -Fmwin32_handmade_hero.map ../src/win32/win32-handmade-hero.cpp ../src/win32/win32-input.cpp ../src/win32/win32-file-io.cpp ../src/win32/win32-sound.cpp ../src/win32/win32-clock.cpp ../src/win32/win32-display.cpp ../src/handmade-hero/handmade-hero.cpp user32.lib gdi32.lib xinput.lib winmm.lib /link -opt:ref
popd
pause
