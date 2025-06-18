@echo off
chcp 936 >nul 2>&1

REM /**
REM  * @File Name: test_chinese_display.bat
REM  * @brief  测试中文字符显示是否正常
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.0
REM  * @Creat Date : 2025
REM  */

title 中文显示测试
color 0A

cls
echo ================================================================
echo                    中文显示测试
echo ================================================================
echo.
echo 正在测试中文字符显示...
echo.
echo 如果您能看到以下中文内容且显示正常，说明编码设置成功：
echo.
echo ★ 测试内容 ★
echo   • 680图像机软件升级工具
echo   • 服务器连接设置
echo   • 文件上传进度显示
echo   • 远程命令执行
echo   • 机器码查看工具
echo   • 授权文件生成工具
echo.
echo ※ 特殊字符测试
echo   √ 成功标记
echo   × 失败标记
echo   ⚠ 警告标记
echo   ℹ 信息标记
echo.

REM 测试用户输入
set /p test_input="请输入任意中文内容进行测试: "

echo.
echo ================================================================
echo                    测试结果
echo ================================================================
echo.
echo 您输入的内容是: %test_input%
echo.

if "%test_input%"=="" (
    echo [提示] 您没有输入任何内容
) else (
    echo [成功] 输入内容显示正常
)

echo.
echo 如果以上中文内容显示正常，说明编码设置成功！
echo 如果显示为乱码或问号，请检查：
echo   1. 控制台字体设置
echo   2. 系统区域设置
echo   3. 批处理文件编码格式
echo.
echo ================================================================

pause 