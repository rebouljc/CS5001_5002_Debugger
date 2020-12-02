@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
mkdir Debug
set IMGUI_DIR=imgui
cl /nologo /MD /Zi /I %IMGUI_DIR%\backends /I %IMGUI_DIR% /I "%DXSDK_DIR%/Include" /D WIN32_LEAN_AND_MEAN /D WINVER=0x601 /D _WIN32_WINNT=0x601 /D UNICODE /D _UNICODE windows_platform_debugger.cpp dx9_win32_platform_main.cpp main_ui.cpp %IMGUI_DIR%\backends\imgui_impl_dx9.cpp %IMGUI_DIR%\backends\imgui_impl_win32.cpp %IMGUI_DIR%\imgui*.cpp /FeDebug/win_test_imgui.exe /FoDebug/ /link /LIBPATH:"%DXSDK_DIR%/Lib/x86" d3d9.lib
