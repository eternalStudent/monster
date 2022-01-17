@echo off

set CommonCompilerFlags=-MT -nologo -GR- -Od -Oi -WX -W4 -wd4201 -wd4100 -Femonster.exe -Z7
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib opengl32.lib Comdlg32.lib

IF NOT EXIST build mkdir build
pushd build
cl %CommonCompilerFlags% ..\main.cpp /link %CommonLinkerFlags%
popd