@echo off

::Debug flags
::set compiler_flags=/Zi /D_DEBUG /Od

::Release flags
set compiler_flags=/O2

pushd ".\build"
    cl %compiler_flags% /D_USRDLL /D_WINDLL ..\src\nes.cpp opengl32.lib glew32.lib /link /DLL /OUT:NES.dll 
    cl %compiler_flags% ..\src\win32_nes.cpp user32.lib gdi32.lib comdlg32.lib opengl32.lib glew32.lib
popd