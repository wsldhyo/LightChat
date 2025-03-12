cmake_minimum_required(VERSION 3.28)
project(LightChat VERSION 1.0.0.0 LANGUAGES CXX)

# Qt相关设置,
# 自动处理Qt的.ui文件
set(CMAKE_AUTOUIC ON)
# 启用Qt MOC
set(CMAKE_AUTOMOC ON)
# Qt资源系统
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#导出编译配置给clangd使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

#qt_standard_project_setup()
add_executable(${PROJECT_NAME} src/main.cc src/test.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core)