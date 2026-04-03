QT += core widgets gui

TARGET = encrypt_package
TEMPLATE = app

SOURCES += encrypt_package.cpp

# 设置应用程序图标和版本信息
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Windows特定设置
win32 {
    TARGET = EncryptPackage
    CONFIG -= console
    CONFIG += windows
    # 如果有图标文件可以取消注释下一行
    # RC_ICONS = icon.ico
}

# 输出目录（输出到上级目录的bin文件夹）
DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build/obj/encrypt_package
MOC_DIR = $$PWD/../build/moc/encrypt_package
RCC_DIR = $$PWD/../build/rcc/encrypt_package
UI_DIR = $$PWD/../build/ui/encrypt_package

# 创建必要的目录
!exists($$DESTDIR): system(mkdir $$shell_path($$DESTDIR))
!exists($$OBJECTS_DIR): system(mkdir $$shell_path($$OBJECTS_DIR))
!exists($$MOC_DIR): system(mkdir $$shell_path($$MOC_DIR))
!exists($$RCC_DIR): system(mkdir $$shell_path($$RCC_DIR))

# 编译器设置
CONFIG += c++11

# 支持UTF-8源文件编码
win32-g++ {
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
}
msvc {
    QMAKE_CXXFLAGS += /utf-8
}

