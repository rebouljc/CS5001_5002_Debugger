@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
mkdir Debug
set IMGUI_DIR=imgui
cl /nologo /Zi /MD /I %IMGUI_DIR%\backends /I %IMGUI_DIR% /I "%DXSDK_DIR%/Include" /D UNICODE /D _UNICODE dx9_win32_platform_main.cpp main_ui.cpp %IMGUI_DIR%\backends\imgui_impl_dx9.cpp %IMGUI_DIR%\backends\imgui_impl_win32.cpp %IMGUI_DIR%\imgui*.cpp /FeDebug/win_test_imgui.exe /FoDebug/ /link /LIBPATH:"%DXSDK_DIR%/Lib/x86" d3d9.lib
