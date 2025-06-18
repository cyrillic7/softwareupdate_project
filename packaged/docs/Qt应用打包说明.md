# Qt应用程序打包指南

## 概述
本项目提供了完整的Qt应用程序编译和打包解决方案，包括编译脚本和打包脚本，使应用程序能在没有安装Qt开发环境的计算机上运行。

### 最新更新：无控制台版本
- **问题解决**：QtProject.exe启动时不再显示控制台窗口
- **配置优化**：项目配置已更新为Windows应用程序模式
- **编译脚本**：提供专门的无控制台编译脚本

## 编译脚本

### 1. build_no_console.bat - qmake无控制台编译
使用qmake编译QtProject，确保不显示控制台窗口。

**使用方法：**
```batch
build_no_console.bat
```

### 2. build_cmake_no_console.bat - CMake无控制台编译
使用CMake编译QtProject，确保不显示控制台窗口。

**使用方法：**
```batch
build_cmake_no_console.bat
```

## 打包脚本

### 1. deploy_qt_apps.bat - 批量打包所有应用
自动打包bin目录下的所有exe文件。

**使用方法：**
```batch
deploy_qt_apps.bat
```

**功能：**
- 自动检测bin目录下的所有exe文件
- 为每个应用创建独立的打包目录
- 自动复制Qt依赖库
- 复制相关配置文件

### 2. deploy_single_app.bat - 单个应用打包
打包指定的单个应用程序。

**使用方法：**
```batch
deploy_single_app.bat <exe文件名>
```

**示例：**
```batch
deploy_single_app.bat QtProject.exe
deploy_single_app.bat 680_AuthTool.exe
deploy_single_app.bat GenerateAuth.exe
deploy_single_app.bat GetMachineCode.exe
```

## 当前可打包的应用程序

| 应用程序 | 文件名 | 说明 |
|---------|--------|------|
| 文件上传工具 | QtProject.exe | 通过SFTP/SCP上传文件到Linux服务器 |
| 授权工具 | 680_AuthTool.exe | 软件授权管理工具 |
| 生成授权 | GenerateAuth.exe | 生成授权文件 |
| 获取机器码 | GetMachineCode.exe | 获取计算机硬件标识码 |

## 打包要求

### 系统要求
- Windows 10/11
- Qt 5.12.12 (MinGW 7.3.0 64-bit)
- 安装路径：`G:\qt5.12.12\5.12.12\mingw73_64`

### 依赖工具
- windeployqt.exe (Qt部署工具)
- MinGW 7.3.0 64-bit 编译器

## 打包过程

1. **环境检查**
   - 验证Qt安装路径
   - 检查windeployqt工具是否存在
   - 确认exe文件存在

2. **依赖分析**
   - 自动分析Qt库依赖
   - 复制必要的DLL文件
   - 包含平台插件

3. **文件复制**
   - 复制应用程序exe文件
   - 复制Qt核心库
   - 复制配置文件和资源

4. **打包优化**
   - 排除不必要的翻译文件
   - 排除调试符号
   - 优化包体积

## 打包结果

### 批量打包 (deploy_qt_apps.bat)
```
packaged/
├── QtProject/
│   ├── QtProject.exe
│   ├── Qt5Core.dll
│   ├── Qt5Gui.dll
│   ├── Qt5Widgets.dll
│   ├── Qt5Network.dll
│   ├── platforms/
│   ├── upload_settings.json
│   └── main.qss
├── 680_AuthTool/
│   ├── 680_AuthTool.exe
│   ├── Qt5Core.dll
│   └── ...
└── 授权使用说明.md
```

### 单个应用打包 (deploy_single_app.bat)
```
packaged_QtProject/
├── QtProject.exe
├── Qt5Core.dll
├── Qt5Gui.dll
├── Qt5Widgets.dll
├── Qt5Network.dll
├── platforms/
├── upload_settings.json
└── main.qss
```

## 部署到目标机器

1. **复制打包文件夹**
   将整个打包目录复制到目标计算机

2. **运行应用**
   直接双击exe文件即可运行

3. **注意事项**
   - 确保目标机器安装了Visual C++ Redistributable
   - 保持目录结构完整
   - 不要单独复制exe文件

## 常见问题

### 1. 找不到windeployqt.exe
- 检查Qt安装路径是否正确
- 确认使用的是完整的Qt安装包

### 2. 应用无法启动
- 检查是否缺少Visual C++ Redistributable
- 确认platforms目录存在
- 检查目录结构是否完整

### 3. 依赖库错误
- 重新运行打包脚本
- 检查原始exe文件是否正常

## 高级选项

### windeployqt参数说明
- `--release`: 部署发布版本
- `--no-translations`: 不包含翻译文件
- `--no-system-d3d-compiler`: 不包含系统D3D编译器
- `--no-opengl-sw`: 不包含软件OpenGL渲染器

### 自定义打包
如需自定义打包选项，可以修改脚本中的`windeployqt`参数。

---

**版本信息：** Qt 5.12.12 MinGW 7.3.0 64-bit  
**更新日期：** 2024年 