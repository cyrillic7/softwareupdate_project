@echo off
echo Building Qt Application...

:: Set Qt environment variables
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo Setting up environment...
echo QTDIR: %QTDIR%
echo MINGW_PATH: %MINGW_PATH%
echo.

:: Check if paths exist
if not exist "%QTDIR%\bin\qmake.exe" (
    echo ERROR: qmake.exe not found at %QTDIR%\bin\qmake.exe
    echo Please check your Qt installation path.
    pause
    exit /b 1
)

if not exist "%MINGW_PATH%\g++.exe" (
    echo ERROR: g++.exe not found at %MINGW_PATH%\g++.exe
    echo Please check your MinGW installation path.
    pause
    exit /b 1
)

:: Create build directory
if not exist build mkdir build
if not exist bin mkdir bin

:: Generate Makefile
echo Generating Makefile...
%QTDIR%\bin\qmake.exe QtProject.pro -o build/Makefile

:: Change to build directory
cd build

:: Build the project
echo Compiling...
mingw32-make.exe

:: Check if build was successful
if exist "..\bin\QtProject.exe" (
    echo.
    echo Build successful! You can run bin\QtProject.exe
    echo To run the application: .\bin\QtProject.exe
    echo.
    cd ..
    pause
) else (
    echo.
    echo Build failed! Please check the error messages above.
    echo.
    cd ..
    pause
    exit /b 1
) 