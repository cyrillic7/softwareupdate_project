@echo off
echo Installing Python dependencies for SSH key installation...
echo.

REM Check if python is available
python --version >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Python found, installing dependencies...
    python -m pip install -r "%~dp0requirements.txt"
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo [SUCCESS] Dependencies installed successfully
        echo You can now use the SSH key installation feature
    ) else (
        echo.
        echo [ERROR] Failed to install dependencies
        echo Please check your Python installation and try again
    )
) else (
    echo [ERROR] Python not found in PATH
    echo Please install Python 3.x and make sure it's in your PATH
    echo Download from: https://www.python.org/downloads/
)

echo.
echo Press any key to continue...
pause >nul 