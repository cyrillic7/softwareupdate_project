@echo off
echo Building Auth Tool...

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

:: Create build directory for auth tool
if not exist build_auth mkdir build_auth
if not exist bin mkdir bin

:: Clean previous build files
echo Cleaning previous build files...
if exist "Makefile.Debug" del "Makefile.Debug"
if exist "Makefile.Release" del "Makefile.Release"
if exist "Makefile" del "Makefile"
if exist "auth_tool.exe" del "auth_tool.exe"
if exist "680_AuthTool.exe" del "680_AuthTool.exe"

:: Generate Makefile for auth tool
echo Generating Makefile for Auth Tool...
%QTDIR%\bin\qmake.exe auth_tool.pro CONFIG+=release

:: Build the auth tool
echo Compiling Auth Tool...
mingw32-make.exe release

:: Check if build was successful and copy to bin
if exist "release\680_AuthTool.exe" (
    echo.
    echo Copying Auth Tool to bin directory...
    copy "release\680_AuthTool.exe" "bin\680_AuthTool.exe"
    
    if exist "bin\680_AuthTool.exe" (
        echo.
        echo Auth Tool build successful! 
        echo Executable location: bin\680_AuthTool.exe
        echo.
        echo You can run the auth tool with: .\bin\680_AuthTool.exe
        echo.
    ) else (
        echo ERROR: Failed to copy Auth Tool to bin directory
        pause
        exit /b 1
    )
) else (
    echo.
    echo Auth Tool build failed! Please check the error messages above.
    echo Checking for debug build...
    
    if exist "debug\680_AuthTool.exe" (
        echo Debug build found, copying to bin...
        copy "debug\680_AuthTool.exe" "bin\680_AuthTool.exe"
        echo Debug version copied to bin\680_AuthTool.exe
    ) else (
        echo No executable found in release or debug directories.
        echo Please check the compilation errors.
        pause
        exit /b 1
    )
)

:: Clean build files (optional)
echo.
echo Cleaning temporary build files...
if exist "Makefile.Debug" del "Makefile.Debug"
if exist "Makefile.Release" del "Makefile.Release"
if exist "Makefile" del "Makefile"
if exist ".qmake.stash" del ".qmake.stash"

echo.
echo Build process completed!
pause 