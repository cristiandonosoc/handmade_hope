@echo off

REM TODO - can we just build both with one compiler version?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
SET warnings=-WX -W4 -wd4100 -wd4201 -wd4244
SET buildconf=-DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1
SET dependencies=user32.lib gdi32.lib winmm.lib
SET envconf=-GR- -EHa- -Oi -MT -Gm- -Od
SET linkerconf=-opt:ref
SET mapconf=-Fmwin32_handmade.map

REM 32-Bit build
REM cl %envconf% %warnings% %buildconf% -FC -Z7 %mapconf% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %linkerconf% %dependencies%

REM 64-Bit build
cl %envconf% %warnings% %buildconf% -FC -Z7 %mapconf% ..\handmade\code\win32_handmade.cpp /link %linkerconf% %dependencies%
popd
