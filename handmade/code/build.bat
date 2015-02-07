@echo off

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
cl -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -FC -Zi ..\handmade\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
