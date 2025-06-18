@echo off
echo Building Machine Code Viewer...

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
if exist "GetMachineCode.exe" del "GetMachineCode.exe"
if exist "get_machine_code.exe" del "get_machine_code.exe"

:: Generate and compile directly
echo Generating Makefile...
qmake get_machine_code.pro

echo Compiling...
mingw32-make

:: Check for output files and copy to bin
echo.
echo Checking for compiled executable...

if exist "GetMachineCode.exe" (
    copy "GetMachineCode.exe" "bin\GetMachineCode.exe"
    echo SUCCESS: GetMachineCode.exe copied to bin directory
) else if exist "get_machine_code.exe" (
    copy "get_machine_code.exe" "bin\GetMachineCode.exe"
    echo SUCCESS: get_machine_code.exe copied to bin directory as GetMachineCode.exe
) else if exist "release\GetMachineCode.exe" (
    copy "release\GetMachineCode.exe" "bin\GetMachineCode.exe"
    echo SUCCESS: Release version copied to bin directory
) else if exist "debug\GetMachineCode.exe" (
    copy "debug\GetMachineCode.exe" "bin\GetMachineCode.exe"
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
if exist "bin\GetMachineCode.exe" (
    echo.
    echo ================================================
    echo BUILD SUCCESSFUL!
    echo Machine Code Viewer created: bin\GetMachineCode.exe
    echo ================================================
    echo.
) else (
    echo.
    echo ERROR: Final executable not found in bin directory
    echo.
)

pause 