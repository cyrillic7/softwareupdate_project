@echo off
setlocal
chcp 65001 >nul

echo 正在使用CMake编译QtProject (无控制台版本)...
echo.

REM 设置Qt环境
set QT_DIR=G:\qt5.12.12\5.12.12\mingw73_64
set QT_BIN_DIR=%QT_DIR%\bin
set MINGW_DIR=G:\qt5.12.12\Tools\mingw730_64\bin
set CMAKE_DIR=G:\qt5.12.12\Tools\CMake_64\bin

REM 添加到PATH
set PATH=%QT_BIN_DIR%;%MINGW_DIR%;%CMAKE_DIR%;%PATH%

REM 检查必要工具
if not exist "%CMAKE_DIR%\cmake.exe" (
    echo 错误: 找不到cmake.exe
    echo 请确保CMake已安装在: %CMAKE_DIR%
    pause
    exit /b 1
)

if not exist "%MINGW_DIR%\mingw32-make.exe" (
    echo 错误: 找不到mingw32-make.exe
    echo 路径: %MINGW_DIR%
    pause
    exit /b 1
)

REM 清理并创建构建目录
echo 清理构建目录...
if exist "build_cmake\" rmdir /s /q "build_cmake\"
mkdir "build_cmake"

echo CMake工具路径: %CMAKE_DIR%
echo Qt工具路径: %QT_BIN_DIR%
echo MinGW工具路径: %MINGW_DIR%
echo.

REM 进入构建目录
cd "build_cmake"

REM 配置项目
echo 正在配置CMake项目...
"%CMAKE_DIR%\cmake.exe" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%" ..
if %errorlevel% neq 0 (
    echo CMake配置失败！
    cd ..
    pause
    exit /b 1
)

REM 编译项目
echo 正在编译项目...
"%CMAKE_DIR%\cmake.exe" --build . --config Release -- -j4
if %errorlevel% neq 0 (
    echo 编译失败！
    cd ..
    pause
    exit /b 1
)

REM 返回根目录
cd ..

REM 检查输出文件
if exist "bin\QtProject.exe" (
    echo.
    echo ================================
    echo CMake编译成功！
    echo.
    echo 输出文件: bin\QtProject.exe
    echo 版本: 无控制台窗口版本 (CMake构建)
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