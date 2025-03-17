set(TARGET_NAME "server")
find_package(jsoncpp REQUIRED)
add_executable(${TARGET_NAME} 
            src/http_connection.cc
            src/server.cc
            src/config_manager.cc
            src/config_manager.cc
            src/logic_system.cc
            src/grpc_vertify_code_client.cc
            src/main.cc
            src/rpc_connection_pool.cc
            src/io_context_pool.cc
            ../common/toolfunc.cc
)
target_link_libraries(${TARGET_NAME} PRIVATE 
    proto_lib
    pthread
    jsoncpp
    )

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common)