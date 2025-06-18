@echo off
REM 设置代码页为简体中文GBK编码
chcp 936 >nul 2>&1

REM 设置控制台属性
mode con: cols=80 lines=30

REM /**
REM  * @File Name: run_tools_fix.bat
REM  * @brief  680图像机软件工具套件启动脚本 (修复中文乱码版本)
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.1
REM  * @Creat Date : 2025
REM  * @Update : 修复中文乱码问题
REM  */

title 680图像机软件工具套件 - 工具启动器
color 0B

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

REM 检查Qt环境
if not exist "%QTDIR%\bin\Qt5Core.dll" (
    echo [警告] 未找到Qt运行库，程序可能无法正常启动
    echo Qt路径: %QTDIR%
    echo 请确认Qt安装路径是否正确
    echo.
)

REM 切换到项目根目录
cd /d "%~dp0.."

:MENU
cls
echo ================================================================
echo             680图像机软件工具套件 - 工具启动器
echo ================================================================
echo 当前目录: %CD%
echo Qt路径: %QTDIR%
echo.
echo 请选择要运行的工具：
echo.
echo [1] 680图像机软件升级工具 (主程序)
echo [2] 机器码查看工具
echo [3] 授权文件生成工具
echo [4] 综合授权工具
echo [5] 测试工具
echo.
echo [8] 编译所有工具
echo [9] 检查所有工具状态
echo [0] 退出
echo.
echo ================================================================

set /p choice="请输入选项 (0-9): "

if "%choice%"=="1" goto RUN_MAIN
if "%choice%"=="2" goto RUN_MACHINE_CODE
if "%choice%"=="3" goto RUN_GENERATE_AUTH
if "%choice%"=="4" goto RUN_AUTH_TOOL
if "%choice%"=="5" goto RUN_TEST_TOOL
if "%choice%"=="8" goto BUILD_ALL
if "%choice%"=="9" goto CHECK_STATUS
if "%choice%"=="0" goto EXIT
echo [错误] 无效选项，请重新选择...
timeout /t 2 >nul
goto MENU

:RUN_MAIN
echo.
echo [信息] 启动680图像机软件升级工具...
if exist "bin\680SoftwareUpdate.exe" (
    start "" "bin\680SoftwareUpdate.exe"
    echo [成功] 程序已启动
) else (
    echo [错误] 未找到 bin\680SoftwareUpdate.exe
    echo [提示] 请先运行编译脚本构建项目
)
echo.
pause
goto MENU

:RUN_MACHINE_CODE
echo.
echo [信息] 启动机器码查看工具...
if exist "bin\GetMachineCode.exe" (
    start "" "bin\GetMachineCode.exe"
    echo [成功] 程序已启动
) else (
    echo [错误] 未找到 bin\GetMachineCode.exe
    echo [提示] 请先运行编译脚本构建项目
)
echo.
pause
goto MENU

:RUN_GENERATE_AUTH
echo.
echo [信息] 启动授权文件生成工具...
if exist "bin\GenerateAuth.exe" (
    start "" "bin\GenerateAuth.exe"
    echo [成功] 程序已启动
) else (
    echo [错误] 未找到 bin\GenerateAuth.exe
    echo [提示] 请先运行编译脚本构建项目
)
echo.
pause
goto MENU

:RUN_AUTH_TOOL
echo.
echo [信息] 启动综合授权工具...
if exist "bin\680_AuthTool.exe" (
    start "" "bin\680_AuthTool.exe"
    echo [成功] 程序已启动
) else (
    echo [错误] 未找到 bin\680_AuthTool.exe
    echo [提示] 请先运行编译脚本构建项目
)
echo.
pause
goto MENU

:RUN_TEST_TOOL
echo.
echo [信息] 启动测试工具...
if exist "bin\test_double_compass.exe" (
    start "" "bin\test_double_compass.exe"
    echo [成功] 程序已启动
) else (
    echo [错误] 未找到 bin\test_double_compass.exe
    echo [提示] 请先运行编译脚本构建项目
)
echo.
pause
goto MENU

:BUILD_ALL
echo.
echo [信息] 开始编译所有工具...
if exist "scripts\build_all_tools.bat" (
    call "scripts\build_all_tools.bat"
) else (
    echo [错误] 未找到编译脚本: scripts\build_all_tools.bat
)
echo.
pause
goto MENU

:CHECK_STATUS
cls
echo ================================================================
echo                     工具状态检查
echo ================================================================
echo.
echo 检查可执行文件状态：
echo.

if exist "bin\680SoftwareUpdate.exe" (
    echo [√] 680图像机软件升级工具: bin\680SoftwareUpdate.exe
) else (
    echo [×] 680图像机软件升级工具: 未编译
)

if exist "bin\GetMachineCode.exe" (
    echo [√] 机器码查看工具: bin\GetMachineCode.exe
) else (
    echo [×] 机器码查看工具: 未编译
)

if exist "bin\GenerateAuth.exe" (
    echo [√] 授权文件生成工具: bin\GenerateAuth.exe
) else (
    echo [×] 授权文件生成工具: 未编译
)

if exist "bin\680_AuthTool.exe" (
    echo [√] 综合授权工具: bin\680_AuthTool.exe
) else (
    echo [×] 综合授权工具: 未编译
)

if exist "bin\test_double_compass.exe" (
    echo [√] 测试工具: bin\test_double_compass.exe
) else (
    echo [×] 测试工具: 未编译
)

echo.
echo ================================================================
echo 如需编译所有工具，请运行: scripts\build_all_tools.bat
echo 或者在主菜单选择选项 [8]
echo ================================================================
echo.
pause
goto MENU

:EXIT
echo.
echo [信息] 感谢使用680图像机软件工具套件！
echo [信息] 程序即将退出...
timeout /t 2 >nul
exit /b 0 