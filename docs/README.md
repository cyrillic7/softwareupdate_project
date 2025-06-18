# Qt项目 - 手动创建控件示例

这是一个Qt5项目示例，展示了如何手动创建控件（不使用.ui文件）并使用QSS样式文件美化界面。

## 项目特点

- ✅ 不使用.ui文件，所有控件手动创建
- ✅ 使用QSS样式文件美化界面
- ✅ 现代化的UI设计
- ✅ 完整的菜单栏和状态栏
- ✅ 响应式布局设计
- ✅ 完整的信号槽连接

## 项目结构

```
QtProject/
├── main.cpp           # 程序入口
├── mainwindow.h       # 主窗口头文件
├── mainwindow.cpp     # 主窗口实现
├── main.qss          # QSS样式文件
├── resources.qrc     # Qt资源文件
├── QtProject.pro     # Qt项目文件
├── CMakeLists.txt    # CMake构建文件
├── build.bat         # Windows快速构建脚本
└── README.md         # 项目说明
```

## 环境要求

- Qt 5.12.12
- MinGW 7.3.0 64位编译器
- Qt安装路径: G:\qt5.12.12\5.12.12\mingw73_64

## 构建方法

### 方法1: 使用批处理脚本（推荐）
双击运行 `build.bat` 文件

### 方法2: 手动使用qmake
```cmd
# 设置环境变量
set QTDIR=G:\qt5.12.12\5.12.12\mingw73_64
set PATH=%QTDIR%\bin;G:\qt5.12.12\Tools\mingw730_64\bin;%PATH%

# 生成Makefile
qmake QtProject.pro

# 编译
mingw32-make
```

### 方法3: 使用CMake
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## 功能特性

### 主要控件
- **标题标签**: 渐变背景的欢迎标题
- **输入框**: 带提示文本的输入框
- **按钮组**: 主要、次要、危险三种样式的按钮
- **文本编辑器**: 多行文本编辑区域
- **列表控件**: 可选择的列表项

### 菜单栏
- **文件菜单**: 新建、打开、保存、退出
- **编辑菜单**: 复制、粘贴
- **帮助菜单**: 关于

### 样式特点
- 现代化扁平设计
- 蓝色主题配色方案
- 圆角边框设计
- 悬停和点击效果
- 自定义滚动条样式

## 运行效果

程序运行后将显示一个现代化的Qt应用程序窗口，包含：
1. 渐变背景的标题区域
2. 输入框和三个不同样式的按钮
3. 文本编辑区域
4. 列表控件
5. 完整的菜单栏和状态栏

## 技术要点

### 手动创建控件
```cpp
// 创建控件
QPushButton *button = new QPushButton("按钮文本", this);
button->setObjectName("buttonName");  // 设置对象名用于QSS选择器

// 创建布局
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(button);
```

### QSS样式应用
```cpp
// 加载QSS文件
QFile file(":/styles/main.qss");
if (file.open(QFile::ReadOnly)) {
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);
}
```

### 信号槽连接
```cpp
// 连接信号和槽
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
```

## 自定义和扩展

1. **修改样式**: 编辑 `main.qss` 文件来改变界面外观
2. **添加控件**: 在 `mainwindow.cpp` 的 `setupUI()` 方法中添加新控件
3. **添加功能**: 创建新的槽函数并连接相应信号

## 故障排除

1. **编译错误**: 检查Qt安装路径是否正确
2. **样式不生效**: 确保QSS文件已正确添加到资源文件中
3. **运行时错误**: 检查所有信号槽连接是否正确

## 许可证

本项目仅供学习和参考使用。 