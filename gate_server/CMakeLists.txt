set(TARGET_NAME "gate_server")
# 设置mysql-connector-cpp安装路径
set(MYSQL_CONCPP_ROOT "/usr/local/mysql/connector-c++-9.2.0")
# mysql-connector-cpp动态库路径
set(MYSQL_CONCPP_LIB ${MYSQL_CONCPP_ROOT}/lib64)
find_package(jsoncpp REQUIRED)
add_executable(${TARGET_NAME}
            ../common/config_manager.cc
            ../common/rpc_connection_pool.cc
            ../common/io_context_pool.cc
            ../common/redis_connection_pool.cc
            ../common/redis_connection_manager.cc
            ../common/mysql_manager.cc
            ../common/mysql_dao.cc
            ../common/toolfunc.cc
            ../common/grpc_status_connection_pool.cc
            src/server.cc
            src/logic_system.cc
            src/grpc_vertify_code_client.cc
            src/main.cc
            src/grpc_status_client.cc
            src/http_connection.cc
)

target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/common
    ${HIREDIS_INCLUDE_DIRS}  #redis
    ${MYSQL_CONCPP_ROOT}/include
    ${MYSQL_CONCPP_ROOT}/include/jdbc
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