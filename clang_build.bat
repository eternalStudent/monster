@echo off

set LinkerFlags=-lopengl32 -luser32 -lgdi32 -lComdlg32

IF NOT EXIST build mkdir build
pushd build
call clang++  ..\main.cpp -o monster.exe %LinkerFlags%
popd