@echo off
REM /**
REM  * @File Name: create_auth.bat
REM  * @brief  680图像机软件测试授权文件创建脚本
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.0
REM  * @Creat Date : 2025
REM  *
REM  */

echo Creating test authorization file...

:: 手动创建一个测试机器码（用于测试）
set TEST_MACHINE_CODE=A1B2C3D4E5F67890ABCDEF1234567890

echo Test Machine Code: %TEST_MACHINE_CODE%

:: 创建临时测试文件
echo %TEST_MACHINE_CODE% > machine_auth.key

echo Authorization file created: machine_auth.key
echo.
echo Content:
type machine_auth.key

pause 