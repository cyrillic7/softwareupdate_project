# 680图像机软件工具套件

一个基于Qt的综合软件工具套件，包含文件传输、授权管理、设备检测等多种功能。

## 🚀 快速开始

### 1. 一键编译所有工具
```bash
.\scripts\build_all_tools.bat
```

### 2. 部署Qt依赖（必需步骤）
```bash
# 编译后必须部署Qt运行库，否则程序无法启动
.\scripts\deploy_qt_apps.bat
```

### 3. 快速启动主程序
```bash
.\run.bat
```

### 4. 启动工具选择器
```bash
# CMD/批处理环境
.\scripts\run_tools.bat

# PowerShell环境（推荐）
.\scripts\run_tools.ps1
```

## ❗ 重要提示

**首次使用必须完成以下步骤：**
1. 编译所有工具
2. **部署Qt依赖** - 这是关键步骤！
3. 然后才能正常启动程序

**程序无法启动？**
- 确保已运行 `.\scripts\deploy_qt_apps.bat`
- 检查bin目录下是否有Qt5Core.dll等Qt库文件
- PowerShell用户使用 `.\scripts\run_tools.ps1`

## 📦 工具套件

### 主要工具
- **680SoftwareUpdate.exe** - 680图像机软件升级工具（主程序）
  - SSH文件传输与校验
  - 远程命令执行终端
  - 完整的设置管理系统
  
### 授权管理工具
- **GetMachineCode.exe** - 机器码查看工具
- **GenerateAuth.exe** - 授权文件生成工具  
- **680_AuthTool.exe** - 综合授权工具

### 测试工具
- **test_double_compass.exe** - 测试工具

## 📁 目录结构

```
680update_software/
├── src/         # 源代码
├── scripts/     # 构建和运行脚本
├── docs/        # 文档
├── bin/         # 可执行文件
└── build/       # 构建临时文件
```

## 🔧 环境要求

- **Qt 5.12.12** with MinGW
- **Windows 操作系统**
- **MinGW 7.3.0 编译器**

## 📖 详细文档

- [项目结构说明](docs/项目结构说明.md)
- [完整设置功能说明](docs/完整设置功能说明.md)
- [远程命令执行功能说明](docs/远程命令执行功能说明.md)

## 🛠️ 开发指南

### 构建单个工具
```bash
# 构建主程序
.\build.bat

# 构建特定版本
.\scripts\build_complete_settings.bat
```

### 运行工具
```bash
# 快速启动主程序
.\run.bat

# 使用图形化启动器
.\scripts\run_tools.bat

# 直接运行
.\bin\680SoftwareUpdate.exe
```

## 💡 使用场景

- **设备维护**: 远程SSH连接进行文件传输和命令执行
- **授权管理**: 生成和验证设备授权文件
- **系统检测**: 获取设备硬件信息和机器码
- **批量操作**: 一键编译和部署多个工具

## 📄 许可证

Copyright (C) 2025 - Qt示例项目

---

**版本**: 2.0  
**作者**: chency  
**邮箱**: 121888719@qq.com 