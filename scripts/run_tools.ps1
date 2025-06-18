# PowerShell版本的680图像机软件工具套件启动脚本
# 避免批处理文件的中文编码问题

param()

# 设置控制台编码为UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$Host.UI.RawUI.WindowTitle = "680图像机软件工具套件"

# 设置Qt环境变量
$env:QTDIR = "G:\qt5.12.12\5.12.12\mingw73_64"
$env:MINGW_PATH = "G:\qt5.12.12\Tools\mingw730_64\bin"
$env:PATH = "$($env:QTDIR)\bin;$($env:MINGW_PATH);$($env:PATH)"

# 切换到项目根目录
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location (Join-Path $scriptPath "..")

function Show-Menu {
    Clear-Host
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host "             680图像机软件工具套件 - 工具启动器" -ForegroundColor Cyan
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "当前目录: $(Get-Location)" -ForegroundColor Yellow
    Write-Host "Qt路径: $($env:QTDIR)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "请选择要运行的工具：" -ForegroundColor White
    Write-Host ""
    Write-Host "[1] 680图像机软件升级工具 (主程序)" -ForegroundColor Green
    Write-Host "[2] 机器码查看工具" -ForegroundColor Green
    Write-Host "[3] 授权文件生成工具" -ForegroundColor Green
    Write-Host "[4] 综合授权工具" -ForegroundColor Green
    Write-Host "[5] 测试工具" -ForegroundColor Green
    Write-Host ""
    Write-Host "[8] 编译所有工具" -ForegroundColor Magenta
    Write-Host "[9] 检查所有工具状态" -ForegroundColor Blue
    Write-Host "[0] 退出" -ForegroundColor Red
    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Cyan
}

function Start-Program {
    param([string]$ProgramPath, [string]$ProgramName)
    
    Write-Host ""
    Write-Host "[信息] 启动 $ProgramName..." -ForegroundColor Yellow
    
    if (Test-Path $ProgramPath) {
        Start-Process $ProgramPath
        Write-Host "[成功] 程序已启动" -ForegroundColor Green
    } else {
        Write-Host "[错误] 未找到 $ProgramPath" -ForegroundColor Red
        Write-Host "[提示] 请先运行编译脚本构建项目" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Read-Host "按回车键继续"
}

function Build-AllTools {
    Write-Host ""
    Write-Host "[信息] 开始编译所有工具..." -ForegroundColor Yellow
    
    $buildScript = "scripts\build_all_tools.bat"
    if (Test-Path $buildScript) {
        & cmd /c $buildScript
    } else {
        Write-Host "[错误] 未找到编译脚本: $buildScript" -ForegroundColor Red
    }
    
    Write-Host ""
    Read-Host "按回车键继续"
}

function Show-Status {
    Clear-Host
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host "                     工具状态检查" -ForegroundColor Cyan
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "检查可执行文件状态：" -ForegroundColor White
    Write-Host ""
    
    $tools = @(
        @{Name="680图像机软件升级工具"; Path="bin\680SoftwareUpdate.exe"},
        @{Name="机器码查看工具"; Path="bin\GetMachineCode.exe"},
        @{Name="授权文件生成工具"; Path="bin\GenerateAuth.exe"},
        @{Name="综合授权工具"; Path="bin\680_AuthTool.exe"},
        @{Name="测试工具"; Path="bin\test_double_compass.exe"}
    )
    
    foreach ($tool in $tools) {
        if (Test-Path $tool.Path) {
            Write-Host "[√] $($tool.Name): $($tool.Path)" -ForegroundColor Green
        } else {
            Write-Host "[×] $($tool.Name): 未编译" -ForegroundColor Red
        }
    }
    
    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host "如需编译所有工具，请选择选项 [8]" -ForegroundColor Yellow
    Write-Host "================================================================" -ForegroundColor Cyan
    Write-Host ""
    Read-Host "按回车键继续"
}

# 主循环
do {
    Show-Menu
    $choice = Read-Host "请输入选项 (0-9)"
    
    switch ($choice) {
        "1" { Start-Program "bin\680SoftwareUpdate.exe" "680图像机软件升级工具" }
        "2" { Start-Program "bin\GetMachineCode.exe" "机器码查看工具" }
        "3" { Start-Program "bin\GenerateAuth.exe" "授权文件生成工具" }
        "4" { Start-Program "bin\680_AuthTool.exe" "综合授权工具" }
        "5" { Start-Program "bin\test_double_compass.exe" "测试工具" }
        "8" { Build-AllTools }
        "9" { Show-Status }
        "0" { 
            Write-Host ""
            Write-Host "[信息] 感谢使用680图像机软件工具套件！" -ForegroundColor Green
            Write-Host "[信息] 程序即将退出..." -ForegroundColor Yellow
            Start-Sleep -Seconds 2
            break
        }
        default {
            Write-Host "[错误] 无效选项，请重新选择..." -ForegroundColor Red
            Start-Sleep -Seconds 2
        }
    }
} while ($true) 