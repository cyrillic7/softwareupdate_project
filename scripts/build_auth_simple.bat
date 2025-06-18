@echo off
echo Building 680 Auth Tool (Simple Version)...

:: Set Qt environment variables
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo Setting up environment...
echo QTDIR: %QTDIR%
echo MINGW_PATH: %MINGW_PATH%
echo.

:: Create bin directory
if not exist bin mkdir bin

:: Clean previous files
echo Cleaning previous build files...
if exist "Makefile" del "Makefile"
if exist "Makefile.Debug" del "Makefile.Debug"
if exist "Makefile.Release" del "Makefile.Release"
if exist ".qmake.stash" del ".qmake.stash"
if exist "680_AuthTool.exe" del "680_AuthTool.exe"
if exist "auth_tool.exe" del "auth_tool.exe"

:: Generate and compile directly
echo Generating Makefile...
qmake auth_tool.pro

echo Compiling...
mingw32-make

:: Check for output files and copy to bin
echo.
echo Checking for compiled executable...

if exist "680_AuthTool.exe" (
    copy "680_AuthTool.exe" "bin\680_AuthTool.exe"
    echo SUCCESS: 680_AuthTool.exe copied to bin directory
) else if exist "auth_tool.exe" (
    copy "auth_tool.exe" "bin\680_AuthTool.exe"
    echo SUCCESS: auth_tool.exe copied to bin directory as 680_AuthTool.exe
) else if exist "release\680_AuthTool.exe" (
    copy "release\680_AuthTool.exe" "bin\680_AuthTool.exe"
    echo SUCCESS: Release version copied to bin directory
) else if exist "debug\680_AuthTool.exe" (
    copy "debug\680_AuthTool.exe" "bin\680_AuthTool.exe"
    echo SUCCESS: Debug version copied to bin directory
) else (
    echo ERROR: No executable file found!
    echo Please check for compilation errors above.
    pause
    exit /b 1
)

:: Clean temporary files
echo.
echo Cleaning temporary files...
if exist "Makefile" del "Makefile"
if exist "Makefile.Debug" del "Makefile.Debug" 
if exist "Makefile.Release" del "Makefile.Release"
if exist ".qmake.stash" del ".qmake.stash"

:: Verify final result
if exist "bin\680_AuthTool.exe" (
    echo.
    echo ================================================
    echo BUILD SUCCESSFUL!
    echo Auth Tool created: bin\680_AuthTool.exe
    echo You can run it with: .\run_auth.bat
    echo Or directly: .\bin\680_AuthTool.exe
    echo ================================================
    echo.
) else (
    echo.
    echo ERROR: Final executable not found in bin directory
    echo.
)

pause 