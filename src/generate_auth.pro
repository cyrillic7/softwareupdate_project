QT += core widgets gui

TARGET = generate_auth
TEMPLATE = app

SOURCES += generate_auth.cpp \
           crypto_utils.cpp

HEADERS += crypto_utils.h

# 设置应用程序图标和版本信息
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Windows特定设置
win32 {
    TARGET = GenerateAuth
    CONFIG -= console
    CONFIG += windows
    # 如果有图标文件可以取消注释下一行
    # RC_ICONS = icon.ico
}

# 输出目录（输出到上级目录的bin文件夹）
DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build/obj/generate_auth
MOC_DIR = $$PWD/../build/moc/generate_auth
RCC_DIR = $$PWD/../build/rcc/generate_auth
UI_DIR = $$PWD/../build/ui/generate_auth

# 创建必要的目录
!exists($$DESTDIR): system(mkdir $$shell_path($$DESTDIR))
!exists($$OBJECTS_DIR): system(mkdir $$shell_path($$OBJECTS_DIR))
!exists($$MOC_DIR): system(mkdir $$shell_path($$MOC_DIR))
!exists($$RCC_DIR): system(mkdir $$shell_path($$RCC_DIR))

# 编译器设置
CONFIG += c++11 