@echo off
setlocal
chcp 65001 >nul

echo 正在编译包含"升级qt软件"功能的QtProject...
echo.

REM 设置Qt环境
set QT_DIR=G:\qt5.12.12\5.12.12\mingw73_64
set QT_BIN_DIR=%QT_DIR%\bin
set MINGW_DIR=G:\qt5.12.12\Tools\mingw730_64\bin

REM 添加到PATH
set PATH=%QT_BIN_DIR%;%MINGW_DIR%;%PATH%

REM 检查必要工具
if not exist "%QT_BIN_DIR%\qmake.exe" (
    echo 错误: 找不到qmake.exe
    echo 路径: %QT_BIN_DIR%
    pause
    exit /b 1
)

if not exist "%MINGW_DIR%\mingw32-make.exe" (
    echo 错误: 找不到mingw32-make.exe
    echo 路径: %MINGW_DIR%
    pause
    exit /b 1
)

REM 清理旧的构建文件
echo 清理旧的构建文件...
if exist "build\" rmdir /s /q "build\"
if exist "Makefile" del "Makefile"
if exist "Makefile.Debug" del "Makefile.Debug"
if exist "Makefile.Release" del "Makefile.Release"

REM 创建构建目录
mkdir build 2>nul

echo Qt工具路径: %QT_BIN_DIR%
echo MinGW工具路径: %MINGW_DIR%
echo.
echo 新增功能:
echo   - 升级qt软件按钮
echo   - SSH远程命令执行
echo   - 远程解压qt_update.tar.gz功能
echo   - 自动执行sync同步数据到磁盘
echo.

REM 生成Makefile
echo 正在生成Makefile...
"%QT_BIN_DIR%\qmake.exe" QtProject.pro -spec win32-g++ "CONFIG+=release" "CONFIG-=debug"
if %errorlevel% neq 0 (
    echo qmake执行失败！
    pause
    exit /b 1
)

REM 编译项目
echo 正在编译项目...
"%MINGW_DIR%\mingw32-make.exe" -j4
if %errorlevel% neq 0 (
    echo 编译失败！
    pause
    exit /b 1
)

REM 检查输出文件
if exist "bin\QtProject.exe" (
    echo.
    echo ================================
    echo 编译成功！
    echo.
    echo 输出文件: bin\QtProject.exe
    echo 版本: 包含升级qt软件功能
    echo.
    echo 新功能说明:
    echo   1. 升级qt软件按钮 - 橙色按钮
    echo   2. 远程SSH命令执行功能
    echo   3. 在/media/sata/ue_data/目录下执行:
    echo      tar -xzvf qt_update.tar.gz -C /mnt/qtfs ^&^& sync
    echo.
    echo 使用说明:
    echo   1. 配置服务器连接信息
    echo   2. 确保qt_update.tar.gz文件已上传到服务器
    echo   3. 点击"升级qt软件"按钮执行升级
    echo.
    echo 可以运行以下命令测试:
    echo   bin\QtProject.exe
    echo ================================
    echo.
) else (
    echo 编译失败: 未找到输出文件 bin\QtProject.exe
    pause
    exit /b 1
)

pause 