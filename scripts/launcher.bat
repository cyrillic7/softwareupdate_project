@echo off
chcp 936 >nul 2>&1

title Choose Tool Launcher
color 0C

echo ================================================================
echo                Tool Launcher Selector
echo ================================================================
echo.
echo Choose which version to run:
echo.
echo [1] English Version (run_tools_ansi.bat)
echo     - Uses English interface to avoid encoding issues
echo     - Most compatible with different Windows versions
echo.
echo [2] Chinese Version Fixed (run_tools_fix.bat)
echo     - Chinese interface with encoding fixes
echo     - May still have issues on some systems
echo.
echo [3] PowerShell Version (run_tools.ps1)
echo     - Modern PowerShell interface
echo     - Best Chinese support
echo     - Requires PowerShell
echo.
echo [4] Encoding Diagnosis (encoding_diagnosis.bat)
echo     - Test which encoding works on your system
echo     - Diagnostic tool
echo.
echo [0] Exit
echo.
echo ================================================================

set /p choice="Please enter option (0-4): "

if "%choice%"=="1" goto ENGLISH
if "%choice%"=="2" goto CHINESE
if "%choice%"=="3" goto POWERSHELL
if "%choice%"=="4" goto DIAGNOSIS
if "%choice%"=="0" goto EXIT

echo Invalid choice!
timeout /t 2 >nul
goto :eof

:ENGLISH
echo.
echo Starting English version...
call run_tools_ansi.bat
goto :eof

:CHINESE
echo.
echo Starting Chinese version...
call run_tools_fix.bat
goto :eof

:POWERSHELL
echo.
echo Starting PowerShell version...
powershell -ExecutionPolicy Bypass -File run_tools.ps1
goto :eof

:DIAGNOSIS
echo.
echo Starting encoding diagnosis...
call encoding_diagnosis.bat
goto :eof

:EXIT
echo.
echo Goodbye!
exit /b 0 