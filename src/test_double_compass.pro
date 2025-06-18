QT += core network

TARGET = test_double_compass
TEMPLATE = app

SOURCES += test_double_compass.cpp \
           crypto_utils.cpp

HEADERS += crypto_utils.h

# 输出目录（输出到上级目录的bin文件夹）
DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build/obj/test_double_compass
MOC_DIR = $$PWD/../build/moc/test_double_compass
RCC_DIR = $$PWD/../build/rcc/test_double_compass
UI_DIR = $$PWD/../build/ui/test_double_compass

# 创建必要的目录
!exists($$DESTDIR): system(mkdir $$shell_path($$DESTDIR))
!exists($$OBJECTS_DIR): system(mkdir $$shell_path($$OBJECTS_DIR))
!exists($$MOC_DIR): system(mkdir $$shell_path($$MOC_DIR))
!exists($$RCC_DIR): system(mkdir $$shell_path($$RCC_DIR))

CONFIG += console
CONFIG += c++11 