@echo off
chcp 65001 >nul
title 680 Image Machine Software - Auth File Generator

echo ================================================================
echo           680 Image Machine Software - Auth File Generator
echo ================================================================
echo.

if not exist "bin\GenerateAuth.exe" (
    echo ERROR: Auth File Generator not found!
    echo Please run build_generate_auth.bat to compile the program first
    echo.
    pause
    exit /b 1
)

echo WARNING: This tool is for software vendors only!
echo.
echo Starting Auth File Generator...
echo.
echo Features:
echo - Input or load customer machine code from file
echo - Generate corresponding auth file (machine_auth.key)
echo - Validate auth file format
echo.
echo Usage Flow:
echo 1. Receive machine code from customer
echo 2. Input machine code in the tool
echo 3. Generate auth file
echo 4. Send auth file to customer
echo.

:: Set Qt library path
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo Setting up Qt environment...
start "" "bin\GenerateAuth.exe"

echo Auth File Generator has been started
echo Please operate in the newly opened window
echo.
pause 