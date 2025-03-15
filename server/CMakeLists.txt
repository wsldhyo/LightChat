set(TARGET_NAME "server")
find_package(jsoncpp REQUIRED)
add_executable(${TARGET_NAME} 
            src/http_connection.cc
            src/server.cc
            src/logic_system.cc
            src/main.cc
            ../common/toolfunc.cc
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(${TARGET_NAME} PRIVATE -pthread -g)
target_link_libraries(${TARGET_NAME} PRIVATE pthread jsoncpp)