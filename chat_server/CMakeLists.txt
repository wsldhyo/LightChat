
set(TARGET_NAME "chat_server")
find_package(jsoncpp REQUIRED)
add_executable(${TARGET_NAME}
            ../common/config_manager.cc
            ../common/toolfunc.cc
            ../common/io_context_pool.cc
            ../common/tcp_msg_node.cc
            src/server.cc
            src/session.cc
            src/logic_system.cc
            src/main.cc
)

target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/common
    ${HIREDIS_INCLUDE_DIRS}  #redis
)

target_link_directories(${TARGET_NAME} PRIVATE ${MYSQL_CONCPP_LIB})

target_link_libraries(${TARGET_NAME} PRIVATE 
    proto_lib
    pthread
    jsoncpp
    ${HIREDIS_LIBRARIES}  #redis
    mysqlcppconn
    )



  #设置可执行文件生成路径
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)