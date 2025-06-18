@echo off
REM /**
REM  * @File Name: deploy_qt_apps.bat
REM  * @brief  680图像机软件工具套件Qt部署脚本
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 2.0
REM  * @Creat Date : 2025
REM  * @Update: 适应新的目录结构，增强部署功能
REM  */

setlocal enabledelayedexpansion
chcp 65001 >nul

title 680图像机软件工具套件 - Qt部署工具
color 0D

REM 切换到项目根目录
cd /d "%~dp0.."

echo ================================================================
echo            680图像机软件工具套件 - Qt部署工具
echo ================================================================
echo 当前目录: %CD%
echo.

REM 设置Qt和MinGW环境
set QT_DIR=G:\qt5.12.12\5.12.12\mingw73_64
set QT_BIN_DIR=%QT_DIR%\bin
set MINGW_DIR=G:\qt5.12.12\Tools\mingw730_64\bin

REM 添加Qt和MinGW到PATH
set PATH=%QT_BIN_DIR%;%MINGW_DIR%;%PATH%

echo 环境配置：
echo Qt路径: %QT_DIR%
echo Qt工具路径: %QT_BIN_DIR%
echo MinGW路径: %MINGW_DIR%
echo.

REM 环境检查
echo 正在检查部署环境...
if not exist "%QT_BIN_DIR%\windeployqt.exe" (
    echo ✗ 错误: 找不到windeployqt.exe
    echo   当前查找路径: %QT_BIN_DIR%\windeployqt.exe
    echo   请检查Qt安装路径是否正确
    pause
    exit /b 1
)

if not exist "%MINGW_DIR%\libgcc_s_seh-1.dll" (
    echo ✗ 警告: MinGW运行库可能不完整
    echo   路径: %MINGW_DIR%
)

if not exist "bin\" (
    echo ✗ 错误: 找不到bin目录
    echo   请先编译项目生成可执行文件
    echo   运行: .\scripts\build_all_tools.bat
    pause
    exit /b 1
)

echo ✓ 环境检查完成
echo.

REM 检查可用的应用程序
echo 扫描可用的应用程序...
set APP_COUNT=0
for %%f in (bin\*.exe) do (
    echo   发现: %%~nxf
    set /a APP_COUNT+=1
)

if %APP_COUNT% equ 0 (
    echo ✗ 错误: bin目录中没有找到任何exe文件
    echo   请先编译项目
    pause
    exit /b 1
)

echo ✓ 找到 %APP_COUNT% 个应用程序
echo.

REM 询问部署选项
echo 部署选项：
echo [1] 快速部署（默认选项）
echo [2] 完整部署（包含调试信息）
echo [3] 最小部署（仅核心依赖）
echo [4] 自定义部署
echo.
set /p DEPLOY_OPTION="请选择部署方式 (1-4, 默认1): "
if "%DEPLOY_OPTION%"=="" set DEPLOY_OPTION=1

REM 设置部署参数
if "%DEPLOY_OPTION%"=="1" (
    set DEPLOY_ARGS=--release --no-translations --no-system-d3d-compiler --no-opengl-sw
    set DEPLOY_NAME=快速部署
) else if "%DEPLOY_OPTION%"=="2" (
    set DEPLOY_ARGS=--debug-and-release --force --verbose 2
    set DEPLOY_NAME=完整部署
) else if "%DEPLOY_OPTION%"=="3" (
    set DEPLOY_ARGS=--release --no-translations --no-system-d3d-compiler --no-opengl-sw --no-quick-import --no-webkit2
    set DEPLOY_NAME=最小部署
) else if "%DEPLOY_OPTION%"=="4" (
    echo.
    echo 自定义部署参数（高级用户）:
    set /p DEPLOY_ARGS="请输入windeployqt参数: "
    set DEPLOY_NAME=自定义部署
) else (
    echo 无效选项，使用默认设置
    set DEPLOY_ARGS=--release --no-translations --no-system-d3d-compiler --no-opengl-sw
    set DEPLOY_NAME=快速部署
)

echo.
echo ================================================================
echo 开始执行%DEPLOY_NAME%...
echo 部署参数: %DEPLOY_ARGS%
echo ================================================================
echo.

REM 创建打包目录
if exist "packaged\" (
    echo 清理旧的打包目录...
    rmdir /s /q "packaged\" 2>nul
)
mkdir "packaged" 2>nul

set SUCCESS_COUNT=0
set FAILED_COUNT=0

REM 打包每个exe文件
for %%f in (bin\*.exe) do (
    echo 正在打包: %%~nxf
    
    REM 创建应用程序目录
    set APP_NAME=%%~nf
    mkdir "packaged\!APP_NAME!" 2>nul
    
    REM 复制exe文件
    copy "%%f" "packaged\!APP_NAME!\" >nul
    if !errorlevel! neq 0 (
        echo   ✗ 复制exe文件失败
        set /a FAILED_COUNT+=1
        goto :next_app
    )
    
    REM 使用windeployqt部署
    "%QT_BIN_DIR%\windeployqt.exe" %DEPLOY_ARGS% "packaged\!APP_NAME!\!APP_NAME!.exe"
    
    if !errorlevel! equ 0 (
        echo   ✓ %%~nf 打包成功
        set /a SUCCESS_COUNT+=1
        
        REM 复制应用程序特定文件
        call :copy_app_files !APP_NAME!
    ) else (
        echo   ✗ %%~nf 打包失败
        set /a FAILED_COUNT+=1
    )
    
    :next_app
    echo.
)

REM 复制公共文件
echo 复制公共文件和文档...
call :copy_common_files

echo.
echo ================================================================
echo 部署完成！
echo ================================================================
echo 统计信息：
echo   成功打包: %SUCCESS_COUNT% 个应用程序
echo   失败: %FAILED_COUNT% 个应用程序
echo   部署方式: %DEPLOY_NAME%
echo.
echo 打包文件位置: packaged\ 目录
echo.

if %SUCCESS_COUNT% gtr 0 (
    echo 应用程序说明:
    if exist "packaged\680SoftwareUpdate\" echo   ✓ 680SoftwareUpdate - 680图像机软件升级工具（主程序）
    if exist "packaged\GetMachineCode\" echo   ✓ GetMachineCode - 机器码查看工具
    if exist "packaged\GenerateAuth\" echo   ✓ GenerateAuth - 授权文件生成工具
    if exist "packaged\680_AuthTool\" echo   ✓ 680_AuthTool - 综合授权工具
    if exist "packaged\test_double_compass\" echo   ✓ test_double_compass - 测试工具
    echo.
    echo 部署的应用程序可以在没有安装Qt开发环境的计算机上运行
    echo 只需将对应的文件夹复制到目标计算机即可
    echo.
    
    set /p OPEN_FOLDER="是否打开packaged目录？(Y/N): "
    if /i "!OPEN_FOLDER!"=="Y" (
        start "" explorer "packaged"
    )
) else (
    echo ✗ 没有应用程序成功打包
    echo   请检查编译是否正确完成
)

echo ================================================================
pause
exit /b 0

REM ================================================================
REM 函数：复制应用程序特定文件
REM 参数：%1=应用程序名称
REM ================================================================
:copy_app_files
set APP=%1

REM 复制样式文件到主程序
if "%APP%"=="680SoftwareUpdate" (
    if exist "src\main.qss" copy "src\main.qss" "packaged\%APP%\" >nul
    if exist "bin\main.qss" copy "bin\main.qss" "packaged\%APP%\" >nul
)

REM 复制设置文件
if exist "bin\upload_settings.json" copy "bin\upload_settings.json" "packaged\%APP%\" >nul

REM 复制机器码文件
if exist "bin\machine_code_*.txt" copy "bin\machine_code_*.txt" "packaged\%APP%\" >nul

goto :eof

REM ================================================================
REM 函数：复制公共文件
REM ================================================================
:copy_common_files

REM 复制文档文件
if exist "docs\*.md" (
    mkdir "packaged\docs" 2>nul
    copy "docs\*.md" "packaged\docs\" >nul
)

REM 复制README
if exist "README.md" copy "README.md" "packaged\" >nul

REM 创建运行脚本
echo 创建启动脚本...
for %%d in (packaged\*) do (
    if exist "%%d\*.exe" (
        set DIR_NAME=%%~nd
        echo @echo off > "%%d\run.bat"
        echo title !DIR_NAME! >> "%%d\run.bat"
        echo start "" "!DIR_NAME!.exe" >> "%%d\run.bat"
    )
)

REM 批量启动脚本已移除，各应用程序独立运行

goto :eof 