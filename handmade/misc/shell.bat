@echo off
if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
  call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
) else (
  call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
)

set path=w:\handmade\misc\;%path%

REM We start the devenv
pushd ..\..\build
devenv win32_handmade.exe
popd
cd w:\handmade\code
