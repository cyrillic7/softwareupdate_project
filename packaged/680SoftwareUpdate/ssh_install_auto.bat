@echo off
title SSH��������
color 0a
echo ========================================
echo        SSH������������
echo ========================================
echo.
echo ��: E:\winpro\680update_software\bin
echo ��: %~dp0
echo.
echo ���쨨SSH������...
echo ��: python install_ssh_key.py --host 1.13.80.192 --port 22 --user ubuntu --key-file C:\Users\Administrator\.ssh\id_rsa.pub --password "CCY19900327ld"
echo.
echo [�쨨] ����...
echo.
python install_ssh_key.py --host 1.13.80.192 --port 22 --user ubuntu --key-file C:\Users\Administrator\.ssh\id_rsa.pub --password "CCY19900327ld"
echo.
echo ========================================
if %ERRORLEVEL% EQU 0 (
    echo [] SSH����
    echo.
    echo �㡧����SSH������
    echo ssh -p 22 ubuntu@1.13.80.192
) else (
    echo [�����] SSH�������������: %ERRORLEVEL%
    echo.
    echo ����
    echo 1. ��
    echo 2. ����
    echo 3. ����
)
echo ========================================
echo.
echo ����...
pause >nul
del "%~f0" >nul 2>&1
