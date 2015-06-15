@echo off

REM TODO - can we just build both with one compiler version?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
REM TODO(Cristián): Remove this warnings ignore on final build
SET warnings=-WX -W4 -wd4100 -wd4201 -wd4244 -wd4189
SET buildconf=-DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1
SET dependencies=user32.lib gdi32.lib winmm.lib
SET envconf=-GR- -EHa- -Oi -MT -Gm- -Od -nologo
SET linkerconf=-opt:ref -incremental:no
SET mapconf=-Fmwin32_handmade.map

REM 32-Bit build
REM cl %envconf% %warnings% %buildconf% -FC -Z7 %mapconf% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %linkerconf% %dependencies%

SET exports=-EXPORT:GameGetSound -EXPORT:GameUpdateAndRender
REM SET pdb_path=-PDB:handmade_%time::=,%.pdb
SET pdb_path=-PDB:"handmade%time::=,%.pdb"

REM 64-Bit build
del *.pdb > NUL 2> NUL
cl %envconf% %warnings% %buildconf% -FC -Z7 -Fmhandmade.map ..\handmade\code\handmade.cpp /LD /link %linkerconf% %exports% %pdb_path%
cl %envconf% %warnings% %buildconf% -FC -Z7 -Fmwin32_handmade.map ..\handmade\code\win32_handmade.cpp /link %linkerconf% %dependencies%
popd
