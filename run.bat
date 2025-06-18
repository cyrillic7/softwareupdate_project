@echo off
chcp 936 >nul 2>&1
REM 680图像机软件 - 根目录快速启动脚本
REM 调用scripts目录中的详细启动脚本

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo 680图像机软件 - 快速启动
echo ========================

REM 调用scripts目录中的详细启动脚本
call scripts\run.bat

REM 如果scripts版本不存在，提供基本启动功能
if errorlevel 1 (
    echo.
    echo 正在尝试基本启动...
    if exist "bin\680SoftwareUpdate.exe" (
        echo 启动主程序...
        start "" "bin\680SoftwareUpdate.exe"
        echo 程序已启动
    ) else (
        echo 错误：未找到可执行文件 bin\680SoftwareUpdate.exe
        echo 请先运行 build.bat 编译项目
        pause
    )
) 