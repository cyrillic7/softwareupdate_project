@echo off
REM /**
REM  * @File Name: run.bat
REM  * @brief  680图像机软件快速启动脚本
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 2.0
REM  * @Creat Date : 2025
REM  * @Update: 适应新的目录结构，支持多工具启动
REM  */

title 680图像机软件 - 快速启动
color 0E

REM 设置Qt环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set MINGW_PATH=G:\qt5.12.12\Tools\mingw730_64\bin
set PATH=%QTDIR%\bin;%MINGW_PATH%;%PATH%

REM 切换到项目根目录
cd /d "%~dp0.."

echo ================================================================
echo              680图像机软件 - 快速启动
echo ================================================================
echo 当前目录: %CD%
echo.

REM 检查主程序是否存在
if exist "bin\680SoftwareUpdate.exe" (
    echo 正在启动680图像机软件升级工具（主程序）...
    echo.
    echo 软件功能特性：
    echo - SSH文件传输与校验
    echo - 远程命令执行终端  
    echo - 完整的设置管理系统
    echo - 自动保存和智能路径管理
    echo - 日志清理和存储管理
    echo - 现代化用户界面
    echo.
    
    REM 启动主程序
    start "680图像机软件升级工具" "bin\680SoftwareUpdate.exe"
    
    echo ✓ 主程序已启动成功！
    echo.
    echo 其他可用工具：
    
    REM 检查其他工具
    if exist "bin\GetMachineCode.exe" (
        echo ✓ 机器码查看工具: bin\GetMachineCode.exe
    ) else (
        echo ✗ 机器码查看工具: 未编译
    )
    
    if exist "bin\GenerateAuth.exe" (
        echo ✓ 授权文件生成工具: bin\GenerateAuth.exe
    ) else (
        echo ✗ 授权文件生成工具: 未编译
    )
    
    if exist "bin\680_AuthTool.exe" (
        echo ✓ 综合授权工具: bin\680_AuthTool.exe
    ) else (
        echo ✗ 综合授权工具: 未编译
    )
    
    echo.
    echo 提示：
    echo - 使用 scripts\run_tools.bat 可以启动工具选择器
    echo - 使用 scripts\build_all_tools.bat 可以编译所有工具
    echo.
    
    goto :success
) else (
    echo ✗ 错误：未找到主程序 680SoftwareUpdate.exe
    echo.
    echo 请先编译项目：
    echo   方式1: .\build.bat                      （编译主程序）
    echo   方式2: .\scripts\build_all_tools.bat    （编译所有工具）
    echo   方式3: .\scripts\build_complete_settings.bat （编译完整设置版本）
    echo.
    echo 预期文件位置: bin\680SoftwareUpdate.exe
    echo.
    
    set /p compile="是否现在编译主程序？(Y/N): "
    if /i "%compile%"=="Y" (
        echo.
        echo 正在启动编译...
        call build.bat
        if exist "bin\680SoftwareUpdate.exe" (
            echo.
            echo 编译成功！正在启动程序...
            start "680图像机软件升级工具" "bin\680SoftwareUpdate.exe"
            goto :success
        ) else (
            echo.
            echo 编译失败，请检查错误信息
            goto :error
        )
    ) else (
        goto :error
    )
)

:success
echo ================================================================
echo 启动完成！
echo.
echo 如需技术支持，请联系：
echo 作者: chency
echo 邮箱: 121888719@qq.com
echo ================================================================
timeout /t 3 >nul
exit /b 0

:error
echo ================================================================
echo 启动失败！
echo.
echo 故障排除建议：
echo 1. 检查Qt环境是否正确安装
echo 2. 运行构建脚本重新编译
echo 3. 查看构建日志中的错误信息
echo 4. 确认所有源文件都在src目录中
echo ================================================================
pause
exit /b 1 