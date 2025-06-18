@echo off
chcp 65001 >nul
title 680 Image Machine Software - Machine Code Viewer

echo ================================================================
echo            680 Image Machine Software - Machine Code Viewer
echo ================================================================
echo.

if not exist "bin\GetMachineCode.exe" (
    echo ERROR: Machine Code Viewer not found!
    echo Please run build_machine_code.bat to compile the program first
    echo.
    pause
    exit /b 1
)

echo Starting Machine Code Viewer...
echo.
echo Features:
echo - View current machine's unique machine code
echo - Copy machine code to clipboard
echo - Save machine code information to file
echo.
echo Usage Flow:
echo 1. Click "Copy Machine Code" button
echo 2. Send machine code to software vendor
echo 3. Place received auth file in software directory
echo 4. Restart main program
echo.

:: Set Qt library path
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo Setting up Qt environment...
start "" "bin\GetMachineCode.exe"

echo Machine Code Viewer has been started
echo Please operate in the newly opened window
echo.
pause 