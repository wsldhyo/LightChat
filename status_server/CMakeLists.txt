set(TARGET_NAME "status_server")


add_executable(${TARGET_NAME}
    src/status_server_impl.cc
)

target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/common
    ${HIREDIS_INCLUDE_DIRS}  #redis
)

target_link_libraries(${TARGET_NAME} PRIVATE 
    proto_lib
    pthread
    ${HIREDIS_LIBRARIES}  #redis
)

  #设置可执行文件生成路径
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)