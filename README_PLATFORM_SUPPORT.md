# 680图像机软件升级工具 - 平台兼容性说明

## 支持的平台

本软件现已优化，完全支持以下平台：

### ✅ 支持的操作系统
- **Windows** (7/8/10/11)
- **Linux** (Ubuntu, Debian, CentOS, Fedora, openSUSE等)
- **macOS** (10.14+)

### 🏗️ 支持的处理器架构
- **x86_64** (Intel/AMD 64位)
- **ARM64/AArch64** (ARM 64位)
- **ARMv7** (ARM 32位)

## 平台特性

### Linux平台优化

#### 🔧 终端程序智能检测
自动检测并支持以下Linux终端：
- GNOME Terminal (gnome-terminal)
- KDE Konsole (konsole)
- XFCE Terminal (xfce4-terminal)
- MATE Terminal (mate-terminal)
- LXTerminal (lxterminal)
- QTerminal (qterminal)
- Alacritty
- Terminator
- XTerm (通用回退)

#### 🖥️ 桌面环境支持
- GNOME
- KDE Plasma
- XFCE
- MATE
- LXDE/LXQt
- Unity

#### 📦 包管理器兼容
- APT (Debian/Ubuntu)
- YUM/DNF (RedHat/Fedora)
- Zypper (openSUSE)
- Pacman (Arch Linux)

### ARM平台优化

#### ⚡ 性能优化
- 针对ARM架构的编译器优化
- 增加的超时时间适配
- 内存使用优化
- 进程执行优化

#### 🛠️ 支持的ARM设备
- Raspberry Pi (3/4/5)
- Orange Pi
- Rock64
- Jetson Nano
- 各类ARM开发板
- ARM服务器

## 依赖要求

### 基础依赖
- Qt5 (5.9+) 或 Qt6
- OpenSSH客户端
- Python 3.6+

### Linux特定依赖

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install qt5-default libqt5widgets5 libqt5network5 openssh-client python3
```

#### CentOS/RHEL/Fedora
```bash
# CentOS/RHEL
sudo yum install qt5-qtbase qt5-qtbase-devel openssh-clients python3

# Fedora
sudo dnf install qt5-qtbase qt5-qtbase-devel openssh-clients python3
```

#### Arch Linux
```bash
sudo pacman -S qt5-base openssh python
```

#### openSUSE
```bash
sudo zypper install libqt5-qtbase libqt5-qtbase-devel openssh python3
```

## 构建说明

### 通用构建 (CMake)

```bash
# 克隆项目
git clone <repository-url>
cd 680update_software

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 安装 (Linux)
sudo make install
```

### ARM平台构建

#### 交叉编译 (推荐)
```bash
# 设置交叉编译工具链
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++

# 配置CMake
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-aarch64.cmake ..

# 编译
make -j$(nproc)
```

#### 本地编译 (ARM设备上)
```bash
# 在ARM设备上直接编译
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 特定发行版构建

#### Ubuntu/Debian 包构建
```bash
# 安装构建工具
sudo apt install build-essential devscripts debhelper

# 构建DEB包
mkdir build && cd build
cmake ..
make package
```

#### RPM包构建
```bash
# 安装构建工具
sudo yum install rpm-build rpm-devel

# 构建RPM包
mkdir build && cd build
cmake ..
make package
```

## 运行时环境配置

### Linux环境变量
```bash
# 添加到 ~/.bashrc 或 ~/.profile
export PATH="/usr/local/bin:$PATH"
export QT_QPA_PLATFORM="xcb"
export LC_ALL="C.UTF-8"
```

### SSH配置优化
```bash
# 编辑SSH客户端配置
nano ~/.ssh/config

# 添加常用配置
Host *
    ServerAliveInterval 60
    ServerAliveCountMax 3
    ConnectTimeout 30
    TCPKeepAlive yes
```

## 故障排除

### 常见问题

#### Qt应用程序无法启动
```bash
# 检查Qt库
ldd ./680SoftwareUpdate

# 安装缺失的Qt库
sudo apt install qt5-default  # Ubuntu/Debian
```

#### 终端无法打开
```bash
# 手动指定终端程序
export TERMINAL=gnome-terminal

# 或者安装xterm作为回退
sudo apt install xterm
```

#### ARM设备性能问题
```bash
# 检查CPU信息
cat /proc/cpuinfo

# 优化系统性能
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

#### Python脚本执行问题
```bash
# 确保Python3可用
which python3

# 安装必要的Python模块
pip3 install paramiko  # 如果使用SSH Python脚本
```

### 调试模式

启用调试输出：
```bash
# 设置调试环境变量
export QT_LOGGING_RULES="*=true"
export QT_DEBUG_PLUGINS=1

# 运行程序
./680SoftwareUpdate
```

## 平台特定功能

### Linux特有功能
- ✅ 自动终端检测
- ✅ 桌面文件集成
- ✅ 系统托盘支持
- ✅ 原生文件对话框
- ✅ D-Bus集成

### ARM优化功能
- ✅ CPU架构检测
- ✅ 内存使用优化
- ✅ 进程超时调整
- ✅ 编译器优化标志

## 贡献指南

如果您想为特定平台或架构贡献代码：

1. Fork本项目
2. 创建平台特定分支：`git checkout -b feature/platform-xyz`
3. 添加平台检测代码到 `PlatformHelper` 类
4. 更新CMake配置文件
5. 添加平台特定测试
6. 提交Pull Request

## 技术支持

如遇到平台兼容性问题，请提供以下信息：

1. 操作系统版本：`uname -a`
2. Qt版本：`qmake --version`
3. 处理器架构：`arch` 或 `uname -m`
4. 发行版信息：`cat /etc/os-release`
5. 错误日志和调试输出

## 更新历史

### v1.0.0
- ✅ 添加完整的Linux支持
- ✅ 添加ARM架构支持  
- ✅ 优化平台检测逻辑
- ✅ 改进终端程序检测
- ✅ 增强Python解释器检测
- ✅ 添加跨平台构建系统

---

**注意**: 本文档持续更新中，最新版本请查看项目仓库。 