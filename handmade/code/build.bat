@echo off

REM TODO - can we just build both with one compiler version?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
SET warnings=-WX -W4 -wd4100 -wd4201
SET buildconf=-DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1
SET dependencies=user32.lib gdi32.lib
SET envconf=-GR- -EHa- -Oi -MT -Gm- -Od
SET linkerconf=/link -subsystem:windows,5.1 -opt:ref
SET mapconf=-Fmwin32_handmade.map
cl %envconf% %warnings% %buildconf% -FC -Z7 %mapconf% ..\handmade\code\win32_handmade.cpp %dependencies%
popd
