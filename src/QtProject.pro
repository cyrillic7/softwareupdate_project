QT += core widgets network

CONFIG += c++11

TARGET = 680SoftwareUpdate
TEMPLATE = app

# 定义Qt安装路径
QTDIR = G:/qt5.12.12/5.12.12/mingw73_64

# 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    crypto_utils.cpp \
    settingsdialog.cpp

# 头文件
HEADERS += \
    mainwindow.h \
    crypto_utils.h \
    settingsdialog.h

# 资源文件
RESOURCES += \
    resources.qrc

# 编译器设置
win32 {
    CONFIG -= console
    CONFIG += windows
    QMAKE_CXXFLAGS += -std=c++11
    
    # 设置MinGW编译器路径（如果需要）
    QMAKE_CC = G:/qt5.12.12/Tools/mingw730_64/bin/gcc.exe
    QMAKE_CXX = G:/qt5.12.12/Tools/mingw730_64/bin/g++.exe
}

# 输出目录（输出到上级目录的bin文件夹）
DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build/obj
MOC_DIR = $$PWD/../build/moc
RCC_DIR = $$PWD/../build/rcc
UI_DIR = $$PWD/../build/ui

# 创建必要的目录
!exists($$DESTDIR): system(mkdir $$shell_path($$DESTDIR))
!exists($$OBJECTS_DIR): system(mkdir $$shell_path($$OBJECTS_DIR))
!exists($$MOC_DIR): system(mkdir $$shell_path($$MOC_DIR))
!exists($$RCC_DIR): system(mkdir $$shell_path($$RCC_DIR))

# 版本信息
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "680图像机软件公司"
QMAKE_TARGET_PRODUCT = "680图像机软件升级工具"
QMAKE_TARGET_DESCRIPTION = "680图像机软件升级和管理工具"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2025" 