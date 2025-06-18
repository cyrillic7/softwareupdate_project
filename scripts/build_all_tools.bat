@echo off
REM /**
REM  * @File Name: build_all_tools.bat
REM  * @brief  680图像机软件工具套件统一编译脚本，一键编译所有工具
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 2.0
REM  * @Creat Date : 2025
REM  * @Update: 适应新的目录结构
REM  */

title 680图像机软件工具套件 - 编译所有工具
color 0A

REM 切换到项目根目录
cd /d "%~dp0.."

echo ================================================================
echo             680图像机软件工具套件 - 编译所有工具
echo ================================================================
echo 当前目录: %CD%
echo.
echo 即将编译以下工具：
echo [1] 主程序 - 680图像机软件升级工具 (680SoftwareUpdate.exe)
echo [2] 机器码查看工具 - GetMachineCode.exe
echo [3] 授权文件生成工具 - GenerateAuth.exe
echo [4] 综合授权工具 - 680_AuthTool.exe
echo [5] 测试工具 - test_double_compass.exe
echo.
echo ================================================================

set /p choice="是否继续编译所有工具？(Y/N): "
if /i "%choice%" neq "Y" (
    echo 操作已取消
    pause
    exit /b 0
)

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo.
echo ================================================================
echo 设置编译环境...
echo QTDIR: %QTDIR%
echo MINGW_PATH: %MINGW_PATH%
echo ================================================================

REM 检查Qt环境
if not exist "%QTDIR%\bin\qmake.exe" (
    echo 错误：qmake.exe未找到，路径: %QTDIR%\bin\qmake.exe
    echo 请检查Qt安装路径是否正确
    pause
    exit /b 1
)

if not exist "%MINGW_PATH%\g++.exe" (
    echo 错误：g++.exe未找到，路径: %MINGW_PATH%\g++.exe
    echo 请检查MinGW安装路径是否正确
    pause
    exit /b 1
)

REM 创建必要目录
if not exist build mkdir build
if not exist bin mkdir bin

echo.
echo ================================================================
echo 开始编译主程序...
echo ================================================================
call :BuildProject "QtProject.pro" "680SoftwareUpdate.exe" "主程序"

echo.
echo ================================================================
echo 开始编译机器码查看工具...
echo ================================================================
call :BuildProject "get_machine_code.pro" "GetMachineCode.exe" "机器码查看工具"

echo.
echo ================================================================
echo 开始编译授权文件生成工具...
echo ================================================================
call :BuildProject "generate_auth.pro" "GenerateAuth.exe" "授权文件生成工具"

echo.
echo ================================================================
echo 开始编译综合授权工具...
echo ================================================================
call :BuildProject "auth_tool.pro" "680_AuthTool.exe" "综合授权工具"

echo.
echo ================================================================
echo 开始编译测试工具...
echo ================================================================
call :BuildProject "test_double_compass.pro" "test_double_compass.exe" "测试工具"

echo.
echo ================================================================
echo 编译完成！检查结果...
echo ================================================================

echo.
echo 检查编译结果：
call :CheckResult "680SoftwareUpdate.exe" "主程序"
call :CheckResult "GetMachineCode.exe" "机器码查看工具"
call :CheckResult "GenerateAuth.exe" "授权文件生成工具"
call :CheckResult "680_AuthTool.exe" "综合授权工具"
call :CheckResult "test_double_compass.exe" "测试工具"

echo.
echo ================================================================
echo 工具套件编译完成！
echo.
echo 可执行文件位置：
echo - 主程序：bin\680SoftwareUpdate.exe
echo - 机器码查看：bin\GetMachineCode.exe
echo - 授权生成：bin\GenerateAuth.exe
echo - 综合授权工具：bin\680_AuthTool.exe
echo - 测试工具：bin\test_double_compass.exe
echo.
echo 使用方法：
echo - 直接运行：.\bin\[工具名].exe
echo - 或使用scripts目录下的运行脚本
echo ================================================================

pause
exit /b 0

REM ================================================================
REM 函数：编译单个项目
REM 参数：%1=项目文件名 %2=可执行文件名 %3=工具名称
REM ================================================================
:BuildProject
set PROJECT_FILE=%~1
set EXE_FILE=%~2
set TOOL_NAME=%~3

echo 正在编译 %TOOL_NAME%...
echo 项目文件: src\%PROJECT_FILE%

REM 清理之前的构建文件
if exist "build\%~n1" rmdir /s /q "build\%~n1"
mkdir "build\%~n1"

REM 生成Makefile
%QTDIR%\bin\qmake.exe "src\%PROJECT_FILE%" -o "build\%~n1\Makefile"
if errorlevel 1 (
    echo ✗ %TOOL_NAME% qmake失败
    goto :eof
)

REM 编译
cd "build\%~n1"
mingw32-make.exe
set BUILD_RESULT=%ERRORLEVEL%
cd ..\..

if %BUILD_RESULT% neq 0 (
    echo ✗ %TOOL_NAME% 编译失败
) else (
    echo ✓ %TOOL_NAME% 编译成功
)

goto :eof

REM ================================================================
REM 函数：检查编译结果
REM 参数：%1=可执行文件名 %2=工具名称
REM ================================================================
:CheckResult
if exist "bin\%~1" (
    echo ✓ %~2编译成功: bin\%~1
) else (
    echo ✗ %~2编译失败
)
goto :eof 