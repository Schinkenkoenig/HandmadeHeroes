@echo off

if not exist W:\ subst w: D:\RiderProjects\HandmadeHeroes

where cl >nul 2>nul

if %errorlevel% neq 0 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not exist W:\build mkdir W:\build

pushd W:\build

cl -Zi w:\HandmadeHeroes\win32_handmade.cpp user32.lib Gdi32.lib

popd

