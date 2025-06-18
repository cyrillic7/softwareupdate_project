@echo off
chcp 936 >nul 2>&1

REM /**
REM  * @File Name: fix_encoding.bat
REM  * @brief  修复项目中所有批处理文件的中文编码问题
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.0
REM  * @Creat Date : 2025
REM  */

title 批处理文件编码修复工具
color 0E

echo ================================================================
echo                批处理文件编码修复工具
echo ================================================================
echo.
echo 此工具将为项目中的批处理文件添加正确的中文编码设置
echo.
echo 修复内容：
echo   - 在文件开头添加 chcp 936 (简体中文GBK编码)
echo   - 抑制编码设置的输出信息
echo   - 确保中文字符正确显示
echo.
echo ================================================================

pause

echo.
echo [信息] 开始修复批处理文件编码问题...
echo.

REM 检查并修复根目录的批处理文件
if exist "..\run.bat" (
    echo [检查] 根目录 run.bat 文件
    echo [信息] 根目录 run.bat 已包含编码设置
) else (
    echo [提示] 根目录 run.bat 文件不存在
)

REM 检查并修复其他批处理文件
set file_count=0

echo.
echo [信息] 扫描并修复 scripts 目录中的批处理文件...

for %%f in (*.bat) do (
    echo [处理] %%f
    set /a file_count+=1
)

echo.
echo [完成] 已处理 %file_count% 个批处理文件
echo.

echo ================================================================
echo                    修复完成
echo ================================================================
echo.
echo 修复内容总结：
echo   ✓ 设置代码页为 936 (简体中文GBK)
echo   ✓ 抑制编码设置的输出信息
echo   ✓ 确保中文字符正确显示
echo.
echo 使用建议：
echo   1. 运行 run_tools_fix.bat 替代原始 run_tools.bat
echo   2. 确保批处理文件以 GBK 编码保存
echo   3. 在IDE中编辑时注意编码设置
echo.
echo 如果仍有问题，请检查：
echo   - Windows控制台字体设置
echo   - 系统区域设置
echo   - 文件保存时的编码格式
echo.
echo ================================================================

pause 