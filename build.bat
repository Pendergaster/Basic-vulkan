@echo off

SETLOCAL
cls
REM -lUser32 -lGdi32  -lmsvcmrt -lShell32 
REM gdi32.lib opengl32.lib kernel32.lib user32.lib shell32.lib   -luser32 -lgdi32 -lshell32 -lkernel32.lib -lopengl32 
SET C_VERSION= -std=c99
SET WARNING_IGNORES= -Wno-unused-function  -Wno-missing-braces
SET INCLUDE_FOLDERS= -I "external/include" -I "C:/VulkanSDK/1.1.114.0/Include"
SET SOURCE_FILES= src/main.c 
SET DEBUG_DEFINITIONS= -DBUILD_DEBUG -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX
SET RELEASE_DEFINITIONS= -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX
SET LIBS= -lmsvcrt -lUser32 -lGdi32 -lShell32 -lKernel32.lib -L"external/debuglibs" -lglfw3 -L C:\VulkanSDK\1.1.114.0\Lib -lvulkan-1
SET DEBUG_PATH= ./build/debug/


clang %C_VERSION% %SOURCE_FILES% %INCLUDE_FOLDERS% %DEBUG_DEFINITIONS% -gcodeview -O0 -fstrict-aliasing -fexceptions -g -Wall -Wextra -Wstrict-aliasing  %WARNING_IGNORES% %LIBS% -o %DEBUG_PATH%motor.exe

IF /I "%1"=="build_release" (
		cd ./ReleaseBin
		clang %C_VERSION% %SOURCE_FILES% %INCLUDE_FOLDERS% %RELEASE_DEFINITIONS% -gcodeview -O3  -fstrict-aliasing -g  -Wstrict-aliasing=2 -Wall -Wextra  %WARNING_IGNORES%  %LIBS%  -o motor.exe
		cd ../
		)



IF /I "%1"=="run" (
		build\debug\motor.exe
		)
IF /I "%1"=="run_release" (
		ReleaseBin\motor.exe
		)


IF /I "%1"=="build_run" (
		cd ./DebugBin
		clang %C_VERSION%  %SOURCE_FILES% %INCLUDE_FOLDERS%  %DEBUG_DEFINITIONS% -g -Wall  -Wstrict-aliasing=2 -fstrict-aliasing  -fexceptions %WARNING_IGNORES% -o %LIBS% motor.exe
		cd ../

		DebugBin\motor.exe
		)


IF /I "%1"=="build_shaders" (
		C:/VulkanSDK/1.1.114.0/Bin32/glslc.exe shaders/basic_shader.vert -o shaders/basic_shader_vert.spv
		C:/VulkanSDK/1.1.114.0/Bin32/glslc.exe shaders/basic_shader.frag -o shaders/basic_shader_frag.spv
		)

ENDLOCAL
