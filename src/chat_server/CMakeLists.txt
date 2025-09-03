
set(TARGET_NAME "chat_server")


# Hiredis 配置
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

#jsoncpp
find_package(jsoncpp REQUIRED)

#jsoncpp 使用现代CMake配置，只提供了 jsoncpp_lib 目标， 没有直接提供JSONCPP_XXX等变量
#可查看jsoncpp-targets.cmake查看是否有jsoncpp_lib 目标
# 添加详细检查
if(TARGET jsoncpp_lib)
    message(STATUS "Found jsoncpp target: jsoncpp_lib")
    get_target_property(JSONCPP_INC_DIRS jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(JSONCPP_LIB_TYPE jsoncpp_lib TYPE)
    get_target_property(JSONCPP_LOCATION jsoncpp_lib LOCATION)
    
    message(STATUS "jsoncpp include dirs: ${JSONCPP_INC_DIRS}")
    message(STATUS "jsoncpp library type: ${JSONCPP_LIB_TYPE}")
    message(STATUS "jsoncpp library location: ${JSONCPP_LOCATION}")
else()
    message(WARNING "jsoncpp_lib target not found!")
endif()




add_executable(${TARGET_NAME}
            main.cc
            server.cc
            session.cc
            logic_system.cc
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
    common_utility_headers
)

# 设置可执行文件生成路径
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 生成调试信息
target_compile_options(${TARGET_NAME} PRIVATE -g)
