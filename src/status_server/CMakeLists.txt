
set(TARGET_NAME "status_server")


# Hiredis 配置
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)


add_executable(${TARGET_NAME}
           main.cc
           status_server.cc
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
    pthread
    proto_lib
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
