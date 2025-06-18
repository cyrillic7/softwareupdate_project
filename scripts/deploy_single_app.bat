@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

REM 使用说明: deploy_single_app.bat <exe文件名>
REM 例如: deploy_single_app.bat QtProject.exe

if "%1"=="" (
    echo 使用方法: %0 ^<exe文件名^>
    echo 例如: %0 QtProject.exe
    echo.
    echo 可用的exe文件:
    dir bin\*.exe /b
    pause
    exit /b 1
)

set APP_EXE=%1
set APP_NAME=%~n1

echo 正在打包应用程序: %APP_EXE%
echo.

REM 设置Qt安装路径
set QT_DIR=G:\qt5.12.12\5.12.12\mingw73_64
set QT_BIN_DIR=%QT_DIR%\bin
set MINGW_DIR=G:\qt5.12.12\Tools\mingw730_64\bin

REM 添加Qt和MinGW到PATH
set PATH=%QT_BIN_DIR%;%MINGW_DIR%;%PATH%

REM 检查windeployqt是否存在
if not exist "%QT_BIN_DIR%\windeployqt.exe" (
    echo 错误: 找不到windeployqt.exe
    echo 请检查Qt安装路径: %QT_BIN_DIR%
    pause
    exit /b 1
)

REM 检查exe文件是否存在
if not exist "bin\%APP_EXE%" (
    echo 错误: 找不到 bin\%APP_EXE%
    echo.
    echo 可用的exe文件:
    dir bin\*.exe /b
    pause
    exit /b 1
)

REM 创建打包目录
set PACKAGE_DIR=packaged_%APP_NAME%
if exist "%PACKAGE_DIR%\" (
    echo 清理旧的打包目录...
    rmdir /s /q "%PACKAGE_DIR%\"
)
mkdir "%PACKAGE_DIR%"

echo 复制应用程序文件...
copy "bin\%APP_EXE%" "%PACKAGE_DIR%\" >nul

echo 使用windeployqt部署Qt依赖...
"%QT_BIN_DIR%\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "%PACKAGE_DIR%\%APP_EXE%"

if %errorlevel% neq 0 (
    echo 打包失败！
    pause
    exit /b 1
)

REM 复制相关配置文件
if "%APP_NAME%"=="QtProject" (
    if exist "bin\upload_settings.json" copy "bin\upload_settings.json" "%PACKAGE_DIR%\" >nul
    if exist "main.qss" copy "main.qss" "%PACKAGE_DIR%\" >nul
    echo 已复制配置文件
)

if exist "bin\machine_code_*.txt" copy "bin\machine_code_*.txt" "%PACKAGE_DIR%\" >nul

echo.
echo ================================
echo 打包成功！
echo.
echo 应用程序: %APP_EXE%
echo 打包目录: %PACKAGE_DIR%\
echo.
echo 您可以将 %PACKAGE_DIR%\ 目录复制到其他计算机上运行
echo ================================
echo.
pause 