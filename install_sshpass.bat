@echo off
chcp 65001 >nul
echo ========================================
echo     680图像机软件升级工具 - 环境检查
echo ========================================
echo.

echo [检查] 正在验证系统环境...
echo.

:: 检查PowerShell版本
echo [1/4] 检查PowerShell...
powershell -Command "Write-Output ('PowerShell版本: ' + $PSVersionTable.PSVersion.ToString())" 2>nul
if %errorlevel%==0 (
    echo [✓] PowerShell 可用
) else (
    echo [×] PowerShell 不可用
    echo [错误] PowerShell是Windows必需组件，请检查系统配置
    goto :error
)
echo.

:: 检查SCP命令
echo [2/4] 检查SCP命令...
scp -V >nul 2>&1
if %errorlevel%==0 (
    echo [✓] SCP 命令可用
    scp 2>&1 | findstr "usage:" >nul
    if %errorlevel%==0 (
        echo [✓] SCP 工具正常
    )
) else (
    echo [×] SCP 命令不可用
    echo.
    echo [解决方案] 请安装SSH客户端工具：
    echo   方法1: Windows设置 → 应用 → 可选功能 → 添加功能 → OpenSSH客户端
    echo   方法2: 下载并安装 Git for Windows (https://git-scm.com/download/win)
    echo   方法3: 下载并安装 PuTTY 套件 (https://www.putty.org/)
    echo.
    echo [注意] 程序将使用PowerShell方法作为备用方案
)
echo.

:: 检查SSH命令
echo [3/4] 检查SSH命令...
ssh -V >nul 2>&1
if %errorlevel%==0 (
    echo [✓] SSH 命令可用
) else (
    echo [×] SSH 命令不可用 (与SCP使用相同的客户端)
)
echo.

:: 测试PowerShell执行能力
echo [4/4] 测试PowerShell执行策略...
powershell -Command "Get-ExecutionPolicy" >nul 2>&1
if %errorlevel%==0 (
    echo [✓] PowerShell 执行策略正常
) else (
    echo [×] PowerShell 执行策略受限
    echo [建议] 如果上传失败，可能需要调整PowerShell执行策略
)
echo.

echo ========================================
echo              环境检查完成
echo ========================================
echo.

echo [使用说明]
echo • 程序在Windows系统上优先使用PowerShell方法进行SCP传输
echo • PowerShell方法可以自动处理SSH密码输入和主机验证
echo • 如果遇到问题，请查看程序日志获取详细错误信息
echo.

echo [常见问题解决]
echo.
echo 问题1: "卡在密码输入步骤"
echo 解决: 确保用户名密码正确，检查网络连接
echo.
echo 问题2: "连接被拒绝"
echo 解决: 检查服务器IP、端口和SSH服务状态
echo.
echo 问题3: "权限被拒绝"
echo 解决: 验证用户名密码，确认用户有远程目录写权限
echo.
echo 问题4: "PowerShell执行失败"
echo 解决: 以管理员身份运行程序，或调整PowerShell执行策略
echo.

echo [测试建议]
echo 1. 先使用"测试连接"功能验证SSH连接
echo 2. 从小文件开始测试上传功能
echo 3. 查看操作日志了解详细过程
echo.

goto :end

:error
echo.
echo [致命错误] 系统环境不满足运行要求
echo 请按照上述提示安装必需组件后重新运行
echo.
pause
exit /b 1

:end
echo 按任意键继续...
pause >nul 