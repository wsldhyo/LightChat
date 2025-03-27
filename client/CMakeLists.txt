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
file(GLOB ICON_FILES 
"${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png"
"${CMAKE_CURRENT_SOURCE_DIR}/icons/*.jpg"
"${CMAKE_CURRENT_SOURCE_DIR}/icons/*.jpeg"
)

# 生成一个包含所有图标的 qrc 文件
set(QRC_CONTENT "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n  <qresource prefix=\"/\">\n")
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
  src/timer_btn.cc
  src/clickable_lbl.cc
  src/reset_pwd_dlg.cc
  ../common/http_manager.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/generated_icons.qrc
)


set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#使用qrc
#add_custom_command(
#    TARGET ${TARGET_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory 
#        ${CMAKE_CURRENT_SOURCE_DIR}/icons
#        ${CMAKE_BINARY_DIR}/bin/icons
#)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/timer_btn.hpp DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/clickable_lbl.hpp DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/stylesheet/stylesheet.qss DESTINATION ${CMAKE_BINARY_DIR}/bin)


# 添加构建目录到包含目录，确保能找到自动生成的 ui_*.h 文件
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR})
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/timer_btn.hpp ${CMAKE_BINARY_DIR}/timer_btn.hpp COPYONLY)
# 拷贝客户端配置到bin目录下
target_compile_options(${TARGET_NAME} PRIVATE -g)

target_link_libraries(${TARGET_NAME} PRIVATE 
      Qt5::Widgets 
      Qt5::Core 
      Qt5::Network
      proto_lib
      )