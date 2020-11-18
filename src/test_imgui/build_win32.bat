@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
set OUT_DIR=Debug
set OUT_EXE=win_test_imgui
set IMGUI_DIR=imgui
set SDL2_DIR=D:\Programs\SDL2-2.0.12
set INCLUDES=/I%IMGUI_DIR%\backends /I%IMGUI_DIR% /I%SDL2_DIR%\include /I%IMGUI_DIR%\examples\libs\gl3w
set SOURCES=main_ui.cpp platform_main.cpp %IMGUI_DIR%\backends\imgui_impl_sdl.cpp %IMGUI_DIR%\backends\imgui_impl_opengl3.cpp %IMGUI_DIR%\imgui*.cpp %IMGUI_DIR%\examples\libs\gl3w\GL\gl3w.c
set LIBS=/libpath:%SDL2_DIR%\lib\x86 SDL2.lib SDL2main.lib opengl32.lib
mkdir %OUT_DIR%
cl /nologo /Zi /MD %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:console
