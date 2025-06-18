@echo off
echo 正在构建680图像机软件（完整设置版本）...
echo ===============================================

REM 切换到项目根目录
cd /d "%~dp0.."

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo 设置环境变量...
echo QTDIR: %QTDIR%
echo MINGW_PATH: %MINGW_PATH%
echo 当前目录: %CD%
echo.

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

REM 显示Qt版本信息
echo Qt版本信息：
%QTDIR%\bin\qmake.exe --version

REM 检查必要文件
if not exist "src\QtProject.pro" (
    echo 错误：未找到项目文件 src\QtProject.pro
    pause
    exit /b 1
)

if not exist "src\mainwindow.h" (
    echo 错误：未找到主窗口头文件 src\mainwindow.h
    pause
    exit /b 1
)

if not exist "src\mainwindow.cpp" (
    echo 错误：未找到主窗口源文件 src\mainwindow.cpp
    pause
    exit /b 1
)

if not exist "src\settingsdialog.h" (
    echo 错误：未找到设置对话框头文件 src\settingsdialog.h
    pause
    exit /b 1
)

if not exist "src\settingsdialog.cpp" (
    echo 错误：未找到设置对话框源文件 src\settingsdialog.cpp
    pause
    exit /b 1
)

REM 清理旧的构建文件
echo 清理旧的构建文件...
if exist "build" rmdir /s /q build
if exist "Makefile" del Makefile
if exist "*.o" del *.o
if exist "debug" rmdir /s /q debug
if exist "release" rmdir /s /q release

REM 创建构建目录
if not exist build mkdir build
if not exist bin mkdir bin

REM 生成Makefile
echo 生成Makefile...
%QTDIR%\bin\qmake.exe src\QtProject.pro -o build\Makefile
if errorlevel 1 (
    echo 错误：qmake生成Makefile失败
    pause
    exit /b 1
)

REM 切换到构建目录
cd build

REM 编译项目
echo 开始编译项目...
mingw32-make.exe
if errorlevel 1 (
    echo 错误：编译失败
    cd ..
    pause
    exit /b 1
)

cd ..

REM 检查生成的可执行文件
if exist "bin\QtProject.exe" (
    set "EXE_NAME=QtProject.exe"
    set "BUILD_PATH=bin"
    echo.
    echo 编译成功！可执行文件: %BUILD_PATH%\%EXE_NAME%
) else (
    echo.
    echo 编译失败！未找到生成的可执行文件
    echo 检查是否存在错误...
    dir bin /b *.exe 2>nul
    pause
    exit /b 1
)

echo.
echo ===============================================
echo 编译成功！可执行文件位置：%BUILD_PATH%\%EXE_NAME%
echo.
echo 功能特性：
echo - SSH文件传输与校验
echo - 远程命令执行终端
echo - 完整的设置对话框
echo - 应用程序设置管理
echo - 自动保存功能
echo - 启动时显示日志选项
echo - 智能默认文件路径
echo - 日志存储路径设置
echo - 自动日志清理功能
echo - 现代化UI设计
echo ===============================================

REM 复制必要的文件到输出目录
if exist "src\main.qss" copy "src\main.qss" "%BUILD_PATH%\"
if exist "docs\完整设置功能说明.md" copy "docs\完整设置功能说明.md" "%BUILD_PATH%\"

echo.
echo 构建完成！按任意键运行程序...
pause >nul

REM 运行程序
cd "%BUILD_PATH%"
start "" "%EXE_NAME%"
cd ..

echo 程序已启动
pause 