@echo off

pushd %CD%
IF NOT EXIST build mkdir build
pushd build

set CommonCompilerFlags= -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 /I..\sdl-vc\include /I..\glad
set CommonLinkerFlags= -incremental:no -opt:ref shell32.lib opengl32.lib ../sdl-vc/lib/x64/SDL2main.lib ../sdl-vc/lib/x64/SDL2.lib /subsystem:console

cl %CommonCompilerFlags% ..\main.cpp /link %CommonLinkerFlags% /out:Boat.exe

popd