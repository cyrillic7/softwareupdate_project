@echo off

REM 不要在开头设置chcp，先检查当前编码状态

echo ================================================================
echo                Encoding Diagnosis Tool
echo ================================================================
echo.
echo Current Code Page Check:
chcp
echo.

echo Testing different code pages:
echo.

echo --- Test 1: Current Code Page ---
echo Chinese test: 测试中文显示
echo Special chars: ★☆●◆■□
echo.

echo --- Test 2: Setting to 936 (GBK) ---
chcp 936 >nul 2>&1
echo Chinese test: 测试中文显示
echo Special chars: ★☆●◆■□
echo.

echo --- Test 3: Setting to 65001 (UTF-8) ---
chcp 65001 >nul 2>&1
echo Chinese test: 测试中文显示
echo Special chars: ★☆●◆■□
echo.

echo --- Test 4: Back to 936 (GBK) ---
chcp 936 >nul 2>&1
echo Chinese test: 测试中文显示
echo Special chars: ★☆●◆■□
echo.

echo ================================================================
echo                Diagnosis Complete
echo ================================================================
echo.
echo Please check which test shows Chinese characters correctly.
echo The correct code page will be used for fixing the batch files.
echo.
echo Recommendations:
echo - If Test 2 (936 GBK) works: Use run_tools_ansi.bat
echo - If Test 3 (65001 UTF-8) works: Files need UTF-8 encoding
echo - If none works: Check system locale settings
echo.

pause

REM Reset to default
chcp 936 >nul 2>&1 