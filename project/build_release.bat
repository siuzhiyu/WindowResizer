@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   WindowResizer-ImGui Build Script
echo ========================================
echo.

REM Find MSBuild
set "MSBUILD="
for /f "delims=" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2^>nul') do (
    set "MSBUILD=%%i"
    goto found
)

:found
if not defined MSBUILD (
    echo [ERROR] MSBuild not found
    echo Please install Visual Studio 2022 with C++ workload
    pause
    exit /b 1
)

echo [INFO] Using: %MSBUILD%
echo.

REM Clean previous build
echo [STEP 1/2] Cleaning previous build...
"%MSBUILD%" ..\WindowResizer-imgui.sln /p:Configuration=Release /p:Platform=x64 /t:Clean /v:minimal >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [WARNING] Clean completed with warnings
)
echo [OK] Clean done
echo.

REM Build
echo [STEP 2/2] Building Release x64...
echo.
"%MSBUILD%" ..\WindowResizer-imgui.sln /p:Configuration=Release /p:Platform=x64 /t:Build /v:minimal
set BUILD_RESULT=%ERRORLEVEL%

echo.
if %BUILD_RESULT% EQU 0 (
    echo ========================================
    echo   BUILD SUCCESS
    echo ========================================
    echo Output: ..\project\Release\WindowResizer-imgui.exe
) else (
    echo ========================================
    echo   BUILD FAILED (Error: %BUILD_RESULT%)
    echo ========================================
    echo.
    echo Common solutions:
    echo   1. Check if all source files exist in src\
    echo   2. Ensure imgui folder is present
    echo   3. Verify project configuration
)

echo.
pause
