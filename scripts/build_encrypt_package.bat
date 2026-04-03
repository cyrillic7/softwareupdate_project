@echo off
chcp 65001 >nul
REM /**
REM  * @File Name: build_encrypt_package.bat
REM  * @brief  680图像机软件升级包加密工具编译脚本
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.0
REM  * @Creat Date : 2025
REM  * @Update: 编译升级包加密工具
REM  */

title 680图像机软件 - 编译升级包加密工具
color 0A

REM 切换到项目根目录
cd /d "%~dp0.."

echo ================================================================
echo        680图像机软件 - 升级包加密工具编译脚本
echo ================================================================
echo 当前目录: %CD%
echo.

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

echo 设置编译环境...
echo QTDIR: %QTDIR%
echo MINGW_PATH: %MINGW_PATH%
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

REM 创建必要目录
if not exist build mkdir build
if not exist bin mkdir bin

echo ================================================================
echo 开始编译升级包加密工具...
echo ================================================================
echo.

REM 清理之前的构建文件
if exist "build\encrypt_package" rmdir /s /q "build\encrypt_package"
mkdir "build\encrypt_package"

REM 生成Makefile
echo 执行 qmake...
%QTDIR%\bin\qmake.exe "src\encrypt_package.pro" -o "build\encrypt_package\Makefile"
if errorlevel 1 (
    echo ✗ qmake 失败
    pause
    exit /b 1
)
echo ✓ qmake 成功

REM 编译
echo.
echo 执行编译...
cd "build\encrypt_package"
mingw32-make.exe
set BUILD_RESULT=%ERRORLEVEL%
cd ..\..

if %BUILD_RESULT% neq 0 (
    echo.
    echo ✗ 编译失败
    pause
    exit /b 1
)

echo.
echo ================================================================
echo 编译完成！
echo ================================================================

REM 检查编译结果
if exist "bin\EncryptPackage.exe" (
    echo ✓ 升级包加密工具编译成功: bin\EncryptPackage.exe
    echo.
    echo 文件信息:
    dir "bin\EncryptPackage.exe"
) else (
    echo ✗ 编译失败，未找到可执行文件
    pause
    exit /b 1
)

echo.
echo ================================================================
echo 使用方法:
echo   直接运行: .\bin\EncryptPackage.exe
echo   或双击运行可执行文件
echo.
echo 功能说明:
echo   - 选择 tar.gz 压缩包进行加密
echo   - 生成 .enc 加密文件
echo   - 支持解密还原原始文件
echo ================================================================

pause
exit /b 0

