# SCP密码交互调试脚本
# 使用方法: .\debug_scp.ps1 "用户名" "IP地址" "端口" "密码" "本地文件" "远程路径"

param(
    [string]$Username,
    [string]$IPAddress, 
    [string]$Port,
    [string]$Password,
    [string]$LocalFile,
    [string]$RemotePath
)

if (-not $Username -or -not $IPAddress -or -not $Port -or -not $Password -or -not $LocalFile -or -not $RemotePath) {
    Write-Host "使用方法: .\debug_scp.ps1 '用户名' 'IP地址' '端口' '密码' '本地文件路径' '远程路径'"
    Write-Host "示例: .\debug_scp.ps1 'alinx' '1.13.80.192' '6001' 'mypassword' 'C:\test.txt' '/home/alinx/'"
    exit 1
}

$ErrorActionPreference = 'Continue'

try {
    Write-Host "=== SCP密码交互调试开始 ==="
    Write-Host "用户名: $Username"
    Write-Host "服务器: $IPAddress:$Port"
    Write-Host "本地文件: $LocalFile"
    Write-Host "远程路径: $RemotePath"
    Write-Host "密码长度: $($Password.Length)"
    Write-Host ""

    $remoteTarget = "$Username@$IPAddress`:$RemotePath"
    Write-Host "远程目标: $remoteTarget"
    
    # 创建SCP进程
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = 'scp'
    $psi.Arguments = "-o ConnectTimeout=30 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o PasswordAuthentication=yes -P $Port `"$LocalFile`" `"$remoteTarget`""
    $psi.UseShellExecute = $false
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    
    Write-Host "SCP命令: scp $($psi.Arguments)"
    Write-Host ""
    
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    
    Write-Host "启动SCP进程..."
    $started = $proc.Start()
    if (-not $started) { 
        throw "无法启动SCP进程"
    }
    
    Write-Host "SCP进程已启动，PID: $($proc.Id)"
    Write-Host "开始监听SSH交互..."
    Write-Host ""
    
    $timeout = 60
    $elapsed = 0
    $passwordSent = $false
    $hostConfirmed = $false
    
    while (-not $proc.HasExited -and $elapsed -lt $timeout) {
        Start-Sleep -Milliseconds 500
        $elapsed += 0.5
        
        # 主动发送策略
        if ($elapsed -gt 3 -and -not $hostConfirmed) {
            Write-Host "[自动] 3秒后主动发送 'yes' 确认主机..."
            try {
                $proc.StandardInput.WriteLine('yes')
                $proc.StandardInput.Flush()
                $hostConfirmed = $true
            } catch {
                Write-Host "[错误] 发送主机确认失败: $($_.Exception.Message)"
            }
        }
        
        if ($elapsed -gt 6 -and -not $passwordSent) {
            Write-Host "[自动] 6秒后主动发送密码..."
            try {
                $proc.StandardInput.WriteLine($Password)
                $proc.StandardInput.Flush()
                $passwordSent = $true
            } catch {
                Write-Host "[错误] 发送密码失败: $($_.Exception.Message)"
            }
        }
        
        # 检查标准错误输出
        if ($proc.StandardError.Peek() -ge 0) {
            $errorLine = $proc.StandardError.ReadLine()
            Write-Host "[STDERR] $errorLine" -ForegroundColor Red
            
            # 检测主机确认
            if ($errorLine -match 'continue connecting' -and -not $hostConfirmed) {
                Write-Host "[检测] 发现主机确认提示，发送 'yes'..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine('yes')
                $proc.StandardInput.Flush()
                $hostConfirmed = $true
            }
            elseif ($errorLine -match 'yes/no' -and -not $hostConfirmed) {
                Write-Host "[检测] 发现yes/no提示，发送 'yes'..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine('yes')
                $proc.StandardInput.Flush()
                $hostConfirmed = $true
            }
            # 检测密码提示
            elseif (($errorLine -match 'password:' -or $errorLine -match 'Password:' -or $errorLine -match 'assword') -and -not $passwordSent) {
                Write-Host "[检测] 发现密码提示，发送密码..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine($Password)
                $proc.StandardInput.Flush()
                $passwordSent = $true
            }
            # 检测错误
            elseif ($errorLine -match 'Permission denied') {
                Write-Host "[错误] 认证失败 - 密码错误" -ForegroundColor Red
                break
            }
            elseif ($errorLine -match 'Connection refused') {
                Write-Host "[错误] 连接被拒绝" -ForegroundColor Red
                break
            }
        }
        
        # 检查标准输出
        if ($proc.StandardOutput.Peek() -ge 0) {
            $outputLine = $proc.StandardOutput.ReadLine()
            Write-Host "[STDOUT] $outputLine" -ForegroundColor Green
            
            # 同样的检测逻辑
            if ($outputLine -match 'continue connecting' -and -not $hostConfirmed) {
                Write-Host "[检测] 标准输出中发现主机确认提示..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine('yes')
                $proc.StandardInput.Flush()
                $hostConfirmed = $true
            }
            elseif ($outputLine -match 'yes/no' -and -not $hostConfirmed) {
                Write-Host "[检测] 标准输出中发现yes/no提示..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine('yes')
                $proc.StandardInput.Flush()
                $hostConfirmed = $true
            }
            elseif (($outputLine -match 'password:' -or $outputLine -match 'Password:' -or $outputLine -match 'assword') -and -not $passwordSent) {
                Write-Host "[检测] 标准输出中发现密码提示..." -ForegroundColor Yellow
                $proc.StandardInput.WriteLine($Password)
                $proc.StandardInput.Flush()
                $passwordSent = $true
            }
        }
        
        # 状态报告
        if ($elapsed % 5 -eq 0) {
            Write-Host "[状态] $elapsed 秒 - 主机确认: $hostConfirmed, 密码发送: $passwordSent" -ForegroundColor Cyan
        }
    }
    
    # 关闭输入流
    try { $proc.StandardInput.Close() } catch { }
    
    # 等待进程结束
    if (-not $proc.HasExited) {
        Write-Host "等待进程完成..."
        $proc.WaitForExit(30000)
    }
    
    $exitCode = $proc.ExitCode
    Write-Host ""
    Write-Host "=== 进程完成，退出码: $exitCode ==="
    
    # 读取剩余输出
    try {
        $remainingOutput = $proc.StandardOutput.ReadToEnd()
        if ($remainingOutput) { 
            Write-Host "[最终输出] $remainingOutput" -ForegroundColor Green
        }
        $remainingError = $proc.StandardError.ReadToEnd()
        if ($remainingError) { 
            Write-Host "[最终错误] $remainingError" -ForegroundColor Red
        }
    } catch { }
    
    if ($exitCode -eq 0) {
        Write-Host "✓ SCP上传成功！" -ForegroundColor Green
    } else {
        Write-Host "✗ SCP上传失败！" -ForegroundColor Red
    }
    
} catch {
    Write-Host "PowerShell异常: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} 