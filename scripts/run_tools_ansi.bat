@echo off
chcp 936 >nul 2>&1

REM /**
REM  * @File Name: run_tools_ansi.bat
REM  * @brief  680图像机软件工具套件启动脚本 (ANSI编码版本)
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.2
REM  * @Creat Date : 2025
REM  * @Note : 此版本专门解决中文乱码问题
REM  */

title 680图像机软件工具套件
color 0B

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

REM 检查Qt环境
if not exist "%QTDIR%\bin\Qt5Core.dll" (
    echo [Warning] Qt runtime not found, program may not start properly
    echo Qt Path: %QTDIR%
    echo Please check Qt installation path
    echo.
)

REM 切换到项目根目录
cd /d "%~dp0.."

:MENU
cls
echo ================================================================
echo          680 Image Machine Software Tool Suite
echo ================================================================
echo Current Directory: %CD%
echo Qt Path: %QTDIR%
echo.
echo Please select the tool to run:
echo.
echo [1] 680 Image Machine Update Tool (Main Program)
echo [2] Machine Code Viewer
echo [3] Authorization File Generator
echo [4] Comprehensive Authorization Tool
echo [5] Test Tool
echo.
echo [8] Build All Tools
echo [9] Check All Tools Status
echo [0] Exit
echo.
echo ================================================================

set /p choice="Please enter option (0-9): "

if "%choice%"=="1" goto RUN_MAIN
if "%choice%"=="2" goto RUN_MACHINE_CODE
if "%choice%"=="3" goto RUN_GENERATE_AUTH
if "%choice%"=="4" goto RUN_AUTH_TOOL
if "%choice%"=="5" goto RUN_TEST_TOOL
if "%choice%"=="8" goto BUILD_ALL
if "%choice%"=="9" goto CHECK_STATUS
if "%choice%"=="0" goto EXIT
echo [Error] Invalid option, please select again...
timeout /t 2 >nul
goto MENU

:RUN_MAIN
echo.
echo [Info] Starting 680 Image Machine Update Tool...
if exist "bin\680SoftwareUpdate.exe" (
    start "" "bin\680SoftwareUpdate.exe"
    echo [Success] Program started
) else (
    echo [Error] File not found: bin\680SoftwareUpdate.exe
    echo [Tip] Please run build script first
)
echo.
pause
goto MENU

:RUN_MACHINE_CODE
echo.
echo [Info] Starting Machine Code Viewer...
if exist "bin\GetMachineCode.exe" (
    start "" "bin\GetMachineCode.exe"
    echo [Success] Program started
) else (
    echo [Error] File not found: bin\GetMachineCode.exe
    echo [Tip] Please run build script first
)
echo.
pause
goto MENU

:RUN_GENERATE_AUTH
echo.
echo [Info] Starting Authorization File Generator...
if exist "bin\GenerateAuth.exe" (
    start "" "bin\GenerateAuth.exe"
    echo [Success] Program started
) else (
    echo [Error] File not found: bin\GenerateAuth.exe
    echo [Tip] Please run build script first
)
echo.
pause
goto MENU

:RUN_AUTH_TOOL
echo.
echo [Info] Starting Comprehensive Authorization Tool...
if exist "bin\680_AuthTool.exe" (
    start "" "bin\680_AuthTool.exe"
    echo [Success] Program started
) else (
    echo [Error] File not found: bin\680_AuthTool.exe
    echo [Tip] Please run build script first
)
echo.
pause
goto MENU

:RUN_TEST_TOOL
echo.
echo [Info] Starting Test Tool...
if exist "bin\test_double_compass.exe" (
    start "" "bin\test_double_compass.exe"
    echo [Success] Program started
) else (
    echo [Error] File not found: bin\test_double_compass.exe
    echo [Tip] Please run build script first
)
echo.
pause
goto MENU

:BUILD_ALL
echo.
echo [Info] Starting to build all tools...
if exist "scripts\build_all_tools.bat" (
    call "scripts\build_all_tools.bat"
) else (
    echo [Error] Build script not found: scripts\build_all_tools.bat
)
echo.
pause
goto MENU

:CHECK_STATUS
cls
echo ================================================================
echo                    Tools Status Check
echo ================================================================
echo.
echo Checking executable files status:
echo.

if exist "bin\680SoftwareUpdate.exe" (
    echo [OK] 680 Image Machine Update Tool: bin\680SoftwareUpdate.exe
) else (
    echo [NO] 680 Image Machine Update Tool: Not built
)

if exist "bin\GetMachineCode.exe" (
    echo [OK] Machine Code Viewer: bin\GetMachineCode.exe
) else (
    echo [NO] Machine Code Viewer: Not built
)

if exist "bin\GenerateAuth.exe" (
    echo [OK] Authorization File Generator: bin\GenerateAuth.exe
) else (
    echo [NO] Authorization File Generator: Not built
)

if exist "bin\680_AuthTool.exe" (
    echo [OK] Comprehensive Authorization Tool: bin\680_AuthTool.exe
) else (
    echo [NO] Comprehensive Authorization Tool: Not built
)

if exist "bin\test_double_compass.exe" (
    echo [OK] Test Tool: bin\test_double_compass.exe
) else (
    echo [NO] Test Tool: Not built
)

echo.
echo ================================================================
echo To build all tools, run: scripts\build_all_tools.bat
echo Or select option [8] in main menu
echo ================================================================
echo.
pause
goto MENU

:EXIT
echo.
echo [Info] Thank you for using 680 Image Machine Software Tool Suite!
echo [Info] Program will exit now...
timeout /t 2 >nul
exit /b 0 