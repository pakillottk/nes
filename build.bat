@echo off

pushd ".\build"
    cl /Zi /D_USRDLL /D_WINDLL ..\src\nes.cpp opengl32.lib glew32.lib /link /DLL /OUT:NES.dll
    cl /Zi /DHOT_RELOAD=1 ..\src\win32_nes.cpp user32.lib gdi32.lib opengl32.lib glew32.lib
popd