cmake_minimum_required(VERSION 3.16)

project(QtProject VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Qt安装路径
set(CMAKE_PREFIX_PATH "G:/qt5.12.12/5.12.12/mingw73_64")

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

# 自动处理Qt的MOC、UIC和RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 源文件
set(SOURCES
    main.cpp
    mainwindow.cpp
)

# 头文件
set(HEADERS
    mainwindow.h
)

# 资源文件
set(RESOURCES
    resources.qrc
)

# 创建可执行文件（Windows应用程序，不显示控制台）
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# Windows特定设置
if(WIN32)
    # 如果需要控制台窗口，取消注释下一行
    # set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)
    
    # 设置版本信息
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

# 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# 创建build目录
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin") 