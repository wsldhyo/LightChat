set(TARGET_NAME "client")

# 启用 Qt 自动处理相关文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

find_package(Qt5 COMPONENTS Widgets Core Network REQUIRED)


# ==================== 资源文件设置 ==================== 
set(QSS_FILE ${CMAKE_SOURCE_DIR}/resources/style/stylesheet.qss)
set(ICONS_DIR ${CMAKE_SOURCE_DIR}/resources/icons)
 # qrc 文件放源码目录（推荐！） 
 set(QRC_FILE ${CMAKE_SOURCE_DIR}/resources/resources.qrc) 
 # 构建 qrc 内容 
 set(QRC_CONTENT "<RCC>\n") 
 # 样式表部分 
 set(QRC_CONTENT "${QRC_CONTENT} <qresource>\n") 
 file(RELATIVE_PATH rel_qss ${CMAKE_SOURCE_DIR}/resources "${QSS_FILE}") 
 set(QRC_CONTENT "${QRC_CONTENT} <file>${rel_qss}</file>\n") 
 set(QRC_CONTENT "${QRC_CONTENT} </qresource>\n") 
 # 图标部分
set(QRC_CONTENT "${QRC_CONTENT} <qresource>\n") 
file(GLOB ICON_FILES "${ICONS_DIR}/*.*") 
list(SORT ICON_FILES) 
foreach(ICON_FILE ${ICON_FILES}) 
  file(RELATIVE_PATH rel_icon ${CMAKE_SOURCE_DIR}/resources "${ICON_FILE}") 
  set(QRC_CONTENT "${QRC_CONTENT} <file>${rel_icon}</file>\n") 
endforeach() 
set(QRC_CONTENT "${QRC_CONTENT} </qresource>\n") 
set(QRC_CONTENT "${QRC_CONTENT}</RCC>\n") 

# 写入源码目录下的 qrc 
file(WRITE ${QRC_FILE} "${QRC_CONTENT}") 
# 添加到 Qt 资源 
qt_add_resources(CLIENT_RESOURCES ${QRC_FILE})




function(copy_headers target)
    set(dst_files "")
    foreach(header IN LISTS ARGN)
        get_filename_component(fname ${header} NAME)
        set(dst ${CMAKE_BINARY_DIR}/${fname})
        add_custom_command(
            OUTPUT ${dst}
            COMMAND ${CMAKE_COMMAND} -E copy ${header} ${dst}
            DEPENDS ${header}
        )
        list(APPEND dst_files ${dst})
    endforeach()
    add_custom_target(${target} ALL DEPENDS ${dst_files})
endfunction()

#拷贝到构建目录，让QtDebugger找到对应自定义类型，以在ui中提升类型
copy_headers(copy_my_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/client_constant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clickedlabel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timerbtn.hpp
)

# ==================== 可执行文件 ====================
add_executable(${TARGET_NAME} 
  main.cc 
  mainwindow.cc
  logindialog.cc
  registerdialog.cc
  resetdialog.cc
  client_globalvar.cc 
  httpmagager.cc
  tcp_manager.cc
  usermgr.cc
  clickedlabel.cc
  timerbtn.cc
  ${CLIENT_RESOURCES}  # 使用 qt_add_resources 自动生成的资源
)

# 包含构建目录，以便找到 ui_*.h
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR})

# 编译选项
target_compile_options(${TARGET_NAME} PRIVATE -g)

# 链接 Qt 库及其他依赖
target_link_libraries(${TARGET_NAME} PRIVATE 
        Qt5::Widgets 
        Qt5::Core 
        Qt5::Network
        proto_lib
        common_utility
)

# 设置输出目录
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/config/config.ini
            $<TARGET_FILE_DIR:${TARGET_NAME}>
)

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/config/config.ini
            $<TARGET_FILE_DIR:${TARGET_NAME}>
)