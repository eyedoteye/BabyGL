@echo off

del *.swp > NUL 2>NUL

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set IncludesDirectory=..\Includes
set GLFWLibDirectory=..\Libs\GLFW
set GLEWLibDirectory=..\Libs\GL

set PreprocessorFlags=-DPAR_SHAPES_IMPLEMENTATION
set CompilerFlags=-MTd -nologo -GR- -Od -W4 -Z7 -FC -wd4201 -EHa -I %IncludesDirectory%

set LinkerFlags=-INCREMENTAL:NO -opt:ref user32.lib gdi32.lib shell32.lib opengl32.lib -LIBPATH:%GLFWLibDirectory% glfw3.lib -LIBPATH:%GLEWLibDirectory% libglew32d.lib

cl %PreprocessorFlags% %CompilerFlags% ..\main.cpp /link %LinkerFlags%
popd
