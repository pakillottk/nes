@echo off

pushd ".\build"
    cl /Zi ..\src\win32_nes.cpp user32.lib gdi32.lib opengl32.lib glew32.lib
popd