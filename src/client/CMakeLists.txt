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


# ==================== 拷贝文件到构建目录的函数 ====================
function(copy_heads target)
    set(dst_files "")

    # 先创建 resources 目录，保证后续拷贝不会失败
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/resources)

    foreach(header IN LISTS ARGN)
        get_filename_component(fname ${header} NAME)
        set(dst ${CMAKE_BINARY_DIR}/bin/resources/${fname})

        add_custom_command(
            OUTPUT ${dst}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${header} ${dst}
            DEPENDS ${header}
            COMMENT "Copying ${header} -> ${dst}"
        )
        list(APPEND dst_files ${dst})
    endforeach()

    add_custom_target(${target} ALL DEPENDS ${dst_files})
endfunction()

copy_heads(copy_my_heads
    ${CMAKE_SOURCE_DIR}/resources/icons/head_1.jpg
)

# ==================== 可执行文件 ====================
add_executable(${TARGET_NAME} 
  ${CLIENT_RESOURCES}  # 使用 qt_add_resources 自动生成的资源
  main.cc 
  mainwindow.cc
  login_dialog.cc
  register_dialog.cc
  reset_dialog.cc
  client_globalvar.cc 
  http_manager.cc
  tcp_manager.cc
  usermgr.cc
  clicked_label.cc
  timer_btn.cc
  clicked_btn.cc
  chat_dialog.cc
  customize_edit.cc
  chat_user_list.cc
  chat_user_wid.cc
  list_item_base.cc
  loading_dialog.cc
  chat_page.cc
  chat_view.cc
  chat_item_base.cc
  bubble_frame.cc
  text_bubble.cc
  picture_bubble.cc
  msg_text_edit.cc
  state_widget.cc
  add_user_item.cc
  search_list.cc
  find_success_dlg.cc
  clicked_once_label.cc
  friend_label.cc
  apply_friend_dialog.cc
  contact_user_item.cc
  group_tip_item.cc
  contact_user_list.cc
  user_data.cc
  new_friend_apply_page.cc
  new_friend_apply_list.cc
  new_friend_apply_item.cc
  find_failed_dlg.cc
  authen_friend_dlg.cc
)

# 包含构建目录，以便找到 ui_*.h
target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_BINARY_DIR} #Qt Designer生成的ui_xxx.h路径
    ${CMAKE_CURRENT_SOURCE_DIR} #Qt Designer提升组件时，告知其到源码路径寻找所需头文件   
)

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


