set(TARGET_NAME "client")
# 启用 Qt 自动处理相关文件
set(CMAKE_AUTOUIC ON)   # 自动生成 ui_*.h 文件
set(CMAKE_AUTOMOC ON)   # 自动处理 MOC
set(CMAKE_AUTORCC ON)   # 自动处理 Qt 资源文件


# 告诉 AutoUic 在项目根目录搜索 .ui 文件
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

find_package(Qt5 COMPONENTS Widgets Core Network REQUIRED)


#生成qrc文件
# 获取 icons 目录下所有 png 文件
file(GLOB ICON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png")

# 生成一个包含所有图标的 qrc 文件
set(QRC_CONTENT "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n  <qresource prefix=\"/icons\">\n")
foreach(icon ${ICON_FILES})
  # 转换为相对路径，例如 "icons/icon1.png"
  file(RELATIVE_PATH rel_icon ${CMAKE_CURRENT_SOURCE_DIR} ${icon})
  set(QRC_CONTENT "${QRC_CONTENT}    <file>${rel_icon}</file>\n")
endforeach()
#将内容写入qrc
set(QRC_CONTENT "${QRC_CONTENT}  </qresource>\n</RCC>\n")

# 写入生成的 qrc , qrc放在当前子目录，方便QtDesigner查找g
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/generated_icons.qrc "${QRC_CONTENT}")

set(TARGET_NAME "client")
add_executable(${TARGET_NAME} 
  src/main.cc 
  src/login_dlg.cc
  src/mainwindow.cc
  src/register_dlg.cc
  src/global.cc
  ../common/http_manager.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/generated_icons.qrc)


set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# 添加构建目录到包含目录，确保能找到自动生成的 ui_*.h 文件
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR})
# 拷贝qss文件到bin目录下
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/stylesheet/stylesheet.qss ${CMAKE_BINARY_DIR}/bin/stylesheet.qss COPYONLY)
# 拷贝客户端配置到bin目录下
target_compile_options(${TARGET_NAME} PRIVATE -g)

target_link_libraries(${TARGET_NAME} PRIVATE 
      Qt5::Widgets 
      Qt5::Core 
      Qt5::Network
      proto_lib
      )