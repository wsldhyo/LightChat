set(TARGET_NAME "gate_server")

add_executable(${TARGET_NAME}
           main.cc
           server.cc
           logic_system.cc
           httpconn.cc
           vertify_rpc_client.cc
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
    proto_lib
    pthread
    jsoncpp_lib
    # mysql-jdbc API # 现代 X DevAPI： mysql::concpp
    ${HIREDIS_LIBRARIES}
    common_pool
    common_manager
    common_utility
    common_utility_headers
)

# 设置可执行文件生成路径
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)
