cmake_minimum_required(VERSION 3.16)

project(680SoftwareUpdate VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台检测
if(WIN32)
    message(STATUS "配置Windows平台构建")
    set(PLATFORM_NAME "Windows")
elseif(UNIX AND NOT APPLE)
    message(STATUS "配置Linux平台构建")
    set(PLATFORM_NAME "Linux")
    
    # 检测ARM架构
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        message(STATUS "检测到ARM架构: ${CMAKE_SYSTEM_PROCESSOR}")
        set(ARM_BUILD ON)
        add_definitions(-DARM_PLATFORM)
    endif()
elseif(APPLE)
    message(STATUS "配置macOS平台构建")
    set(PLATFORM_NAME "macOS")
endif()

# 查找Qt组件
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network
    ProcessHandler
)

# 设置Qt自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/settingsdialog.cpp
    src/auth_tool.cpp
    src/get_machine_code.cpp
    src/test_double_compass.cpp
)

# 头文件
set(HEADERS
    src/mainwindow.h
    src/settingsdialog.h
    src/platformhelper.h
)

# 资源文件
set(RESOURCES
    # 如果有资源文件，在这里添加
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)

# 平台特定的编译选项
if(WIN32)
    # Windows特定设置
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Windows图标和版本信息
    # target_sources(${PROJECT_NAME} PRIVATE resources/app.rc)
    
elseif(UNIX AND NOT APPLE)
    # Linux特定设置
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        LINUX_PLATFORM
    )
    
    # Linux系统库
    find_package(PkgConfig REQUIRED)
    
    # ARM特定优化
    if(ARM_BUILD)
        target_compile_options(${PROJECT_NAME} PRIVATE 
            -mcpu=native
            -mtune=native
        )
        message(STATUS "启用ARM优化编译选项")
    endif()
    
    # 安装规则（Linux）
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
    
    # 创建桌面文件
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/680softwareupdate.desktop.in
        ${CMAKE_BINARY_DIR}/680softwareupdate.desktop
        @ONLY
    )
    
    install(FILES ${CMAKE_BINARY_DIR}/680softwareupdate.desktop
        DESTINATION share/applications
    )
    
elseif(APPLE)
    # macOS特定设置
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        MACOS_PLATFORM
    )
    
    # macOS应用程序包
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist.in
    )
endif()

# 调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
    message(STATUS "调试构建模式")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE RELEASE_BUILD)
    message(STATUS "发布构建模式")
endif()

# 编译时显示详细信息
message(STATUS "=== 构建配置信息 ===")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "版本: ${PROJECT_VERSION}")
message(STATUS "平台: ${PLATFORM_NAME}")
message(STATUS "架构: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Qt版本: ${Qt5_VERSION}")
if(ARM_BUILD)
    message(STATUS "ARM优化: 启用")
endif()
message(STATUS "===================")

# 复制必要的文件到构建目录
if(EXISTS ${CMAKE_SOURCE_DIR}/install_ssh_key.py)
    configure_file(
        ${CMAKE_SOURCE_DIR}/install_ssh_key.py
        ${CMAKE_BINARY_DIR}/install_ssh_key.py
        COPYONLY
    )
endif()

# 创建版本信息头文件
configure_file(
    ${CMAKE_SOURCE_DIR}/src/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
    @ONLY
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_BINARY_DIR}/generated
)

# CPack配置（用于创建安装包）
include(CPack)
set(CPACK_PACKAGE_NAME "680图像机软件升级工具")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "跨平台SSH文件传输和系统升级工具")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default,openssh-client,python3")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt5-qtbase,openssh-clients,python3")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
endif() 