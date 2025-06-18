@echo off
chcp 65001 >nul
echo ========================================
echo    680图像机软件构建脚本 (远程命令执行版)
echo ========================================
echo.

REM 设置Qt环境
set QT_DIR=G:\qt5.12.12\5.12.12\mingw73_64
set QT_BIN=%QT_DIR%\bin
set MINGW_BIN=G:\qt5.12.12\Tools\mingw730_64\bin

REM 设置临时环境变量
set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%

echo [信息] Qt路径: %QT_DIR%
echo [信息] 编译器: MinGW 7.3.0 64-bit
echo.

REM 检查Qt安装
if not exist "%QT_BIN%\qmake.exe" (
    echo [错误] 找不到qmake.exe，请检查Qt安装路径
    pause
    exit /b 1
)

if not exist "%MINGW_BIN%\g++.exe" (
    echo [错误] 找不到g++.exe，请检查MinGW安装
    pause
    exit /b 1
)

REM 检查源文件
if not exist "QtProject.pro" (
    echo [错误] 找不到QtProject.pro文件
    pause
    exit /b 1
)

echo [信息] 开始清理旧的构建文件...
if exist "Makefile" del /f /q "Makefile" >nul 2>&1
if exist "Makefile.Debug" del /f /q "Makefile.Debug" >nul 2>&1
if exist "Makefile.Release" del /f /q "Makefile.Release" >nul 2>&1
if exist ".qmake.stash" del /f /q ".qmake.stash" >nul 2>&1

REM 清理调试和发布目录
if exist "debug" (
    echo [信息] 清理debug目录...
    rmdir /s /q "debug" >nul 2>&1
)

if exist "release" (
    echo [信息] 清理release目录...
    rmdir /s /q "release" >nul 2>&1
)

echo [信息] 清理完成
echo.

REM 运行qmake生成Makefile
echo [信息] 正在生成Makefile...
qmake QtProject.pro -spec win32-g++ "CONFIG+=release" "CONFIG+=qtquickcompiler" "CONFIG-=console" "CONFIG+=windows"

if errorlevel 1 (
    echo [错误] qmake执行失败
    pause
    exit /b 1
)

echo [信息] Makefile生成成功
echo.

REM 编译项目
echo [信息] 开始编译项目（远程命令执行版）...
echo [信息] 新功能: 自定义SSH命令执行，类似终端界面
echo [信息] 特性: 实时输出显示、彩色终端、命令历史
echo.

mingw32-make clean >nul 2>&1
mingw32-make -j4

if errorlevel 1 (
    echo [错误] 编译失败，请检查源代码
    pause
    exit /b 1
)

echo.
echo [成功] 编译完成！
echo.

REM 检查生成的可执行文件
if exist "release\QtProject.exe" (
    echo [信息] 生成的可执行文件:
    echo         release\QtProject.exe
    
    REM 显示文件信息
    for %%f in (release\QtProject.exe) do (
        echo [信息] 文件大小: %%~zf 字节
        echo [信息] 修改时间: %%~tf
    )
    echo.
    echo [功能] 新增远程命令执行功能:
    echo         • SSH终端式界面
    echo         • 实时命令输出显示
    echo         • 支持彩色终端显示
    echo         • 自动滚动和历史记录
    echo         • 回车快捷执行
    echo         • 一键清空输出
    echo.
) else (
    echo [错误] 没有找到生成的可执行文件
    exit /b 1
)

REM 提示部署
echo [提示] 如需部署到其他电脑，请运行:
echo         deploy_qt_apps.bat
echo.
echo [提示] 功能说明文档:
echo         远程命令执行功能说明.md
echo.

echo 构建完成！按任意键退出...
pause >nul 