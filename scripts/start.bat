@echo off
REM /**
REM  * @File Name: start.bat
REM  * @brief  680图像机软件工具套件主启动菜单，提供统一的程序入口
REM  * @Author : chency email:121888719@qq.com
REM  * @Version : 1.0
REM  * @Creat Date : 2025
REM  *
REM  */

chcp 65001 >nul
title 680 Image Machine Software Suite
color 0B

:start
cls
echo ================================================================
echo                680 Image Machine Software Suite
echo ================================================================
echo.
echo Please select which application to run:
echo.
echo [1] Main Application - 680 Update Tool
echo     (File upload, server connection, authorization check)
echo.
echo [2] Machine Code Viewer (For Customers)
echo     (View machine code, copy, save to file)
echo.
echo [3] Auth File Generator (For Vendors)
echo     (Input machine code, generate auth files)
echo.
echo [4] Comprehensive Auth Tool (Original Tool)
echo     (Machine code management, auth file management)
echo.
echo [5] Exit
echo.
echo ================================================================

set /p choice="Enter your choice (1-5): "

if "%choice%"=="1" goto :main_app
if "%choice%"=="2" goto :machine_code
if "%choice%"=="3" goto :generate_auth
if "%choice%"=="4" goto :auth_tool  
if "%choice%"=="5" goto :exit
echo Invalid choice! Please select 1-5.
pause
goto :start

:main_app
echo.
echo Starting Main Application...
call run.bat
goto :end

:machine_code
echo.
echo Starting Machine Code Viewer...
call run_machine_code.bat
goto :end

:generate_auth
echo.
echo Starting Auth File Generator...
call run_generate_auth.bat
goto :end

:auth_tool
echo.
echo Starting Comprehensive Auth Tool...
call run_auth.bat
goto :end

:exit
echo.
echo Goodbye!
goto :end

:end
echo.
pause 