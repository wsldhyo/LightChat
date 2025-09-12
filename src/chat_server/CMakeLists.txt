
set(TARGET_NAME "chat_server")

add_executable(${TARGET_NAME}
            main.cc
            server.cc
            session.cc
            logic_system.cc
            chat_rpc_server.cc
            user_manager.cc
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
    pthread
    jsoncpp_lib
    ${HIREDIS_LIBRARIES}
    mysql::concpp-jdbc # mysql-jdbc API # 现代 X DevAPI： mysql::concpp
    common_pool
    common_manager
    common_utility
    common_rpc_client
)

# 设置可执行文件生成路径
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)
