@echo off
echo Building Auth File Generator...

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
if exist "GenerateAuth.exe" del "GenerateAuth.exe"
if exist "generate_auth.exe" del "generate_auth.exe"

:: Generate and compile directly
echo Generating Makefile...
qmake generate_auth.pro

echo Compiling...
mingw32-make

:: Check for output files and copy to bin
echo.
echo Checking for compiled executable...

if exist "GenerateAuth.exe" (
    copy "GenerateAuth.exe" "bin\GenerateAuth.exe"
    echo SUCCESS: GenerateAuth.exe copied to bin directory
) else if exist "generate_auth.exe" (
    copy "generate_auth.exe" "bin\GenerateAuth.exe"
    echo SUCCESS: generate_auth.exe copied to bin directory as GenerateAuth.exe
) else if exist "release\GenerateAuth.exe" (
    copy "release\GenerateAuth.exe" "bin\GenerateAuth.exe"
    echo SUCCESS: Release version copied to bin directory
) else if exist "debug\GenerateAuth.exe" (
    copy "debug\GenerateAuth.exe" "bin\GenerateAuth.exe"
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
if exist "bin\GenerateAuth.exe" (
    echo.
    echo ================================================
    echo BUILD SUCCESSFUL!
    echo Auth File Generator created: bin\GenerateAuth.exe
    echo ================================================
    echo.
) else (
    echo.
    echo ERROR: Final executable not found in bin directory
    echo.
)

pause 