@echo off
echo Starting 680 Auth Tool...

:: Set Qt environment variables
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

:: Check if auth tool exists
if exist "..\bin\680_AuthTool.exe" (
    echo Running 680 Auth Tool...
    echo.
    echo Note: 680 Image Machine Software Authorization Tool
    echo Features include:
    echo - View current machine code
    echo - Copy machine code to clipboard
    echo - Generate authorization files
    echo - Verify authorization files
    echo - Authorization management
    echo.
    start "680 Auth Tool" "..\bin\680_AuthTool.exe"
    echo Auth Tool started successfully!
) else (
    echo.
    echo ERROR: 680_AuthTool.exe not found in bin directory!
    echo Please build the auth tool first using: .\build_auth.bat
    echo.
    echo Expected location: bin\680_AuthTool.exe
    echo.
    echo If you just built it, please check for compilation errors.
    echo.
    pause
) 