cmake_minimum_required(VERSION 3.28)
project(LightChat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# 查找依赖项
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)   #redis
pkg_check_modules(HIREDIS REQUIRED hiredis)

# 设置 .proto  文件路径 
set(PROTO_ROOT_DIR "${CMAKE_SOURCE_DIR}/proto") # 定义生成文件输出目录
set(PROTO_GENERATED_DIR "${CMAKE_BINARY_DIR}/proto")
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})
# 递归查找所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES 
    "${PROTO_ROOT_DIR}/*.proto"  # 匹配所有层级的子目录
)

foreach(PROTO_FILE ${PROTO_FILES})
    # 提取proto文件名和相对路径
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_RELATIVE_DIR ${PROTO_FILE} DIRECTORY)
    file(RELATIVE_PATH PROTO_RELATIVE_PATH ${PROTO_ROOT_DIR} ${PROTO_RELATIVE_DIR})

    # Protobuf基础代码生成
    add_custom_command(
        OUTPUT 
            "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.pb.cc"
            "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.pb.h"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS
            --proto_path=${PROTO_ROOT_DIR}
            --cpp_out=${PROTO_GENERATED_DIR}
            ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating Protobuf code for ${PROTO_FILE}"
        VERBATIM
    )

    # gRPC代码生成
    add_custom_command(
        OUTPUT
            "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.grpc.pb.cc"
            "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.grpc.pb.h"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS
            --proto_path=${PROTO_ROOT_DIR}
            --grpc_out=${PROTO_GENERATED_DIR}
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            ${PROTO_FILE}
        DEPENDS 
            ${PROTO_FILE}
            gRPC::grpc_cpp_plugin
        COMMENT "Generating gRPC code for ${PROTO_FILE}"
        VERBATIM
    )

    # 收集生成文件路径
    list(APPEND PROTO_SRCS 
        "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.pb.cc"
        "${PROTO_GENERATED_DIR}/${PROTO_RELATIVE_PATH}/${PROTO_NAME}.grpc.pb.cc"
    )
endforeach()


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


# Hiredis 配置
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)


#mysql-cppconn
find_package(mysql-concpp REQUIRED)
#和jsoncpp一样，不直接提供MYSQL_XXX变量，而是通过目标提供
if(TARGET mysql::concpp)
    message(STATUS "Found MySQL Connector/C++ target: mysql::concpp")
    get_target_property(_incDirs    mysql::concpp INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(_linkLibs   mysql::concpp INTERFACE_LINK_LIBRARIES)
    get_target_property(_libType    mysql::concpp TYPE)
    get_target_property(_runtimeDir mysql::concpp MYSQL_CONCPP_RUNTIME_LIBRARY_DIR)
    get_target_property(_location   mysql::concpp IMPORTED_LOCATION)
    get_target_property(_locationDbg mysql::concpp IMPORTED_LOCATION_DEBUG)

    message(STATUS "mysql-concpp include dirs:           ${_incDirs}")
    message(STATUS "mysql-concpp link libraries:        ${_linkLibs}")
    message(STATUS "mysql-concpp target type:           ${_libType}")
    message(STATUS "mysql-concpp runtime library dir:   ${_runtimeDir}")
    message(STATUS "mysql-concpp release library path:  ${_location}")
    message(STATUS "mysql-concpp debug library path:    ${_locationDbg}")
else()
    message(WARNING "mysql::concpp target not found!")
endif()



# 创建静态库
add_library(proto_lib STATIC
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
# 链接proto_lib的依赖库文件
target_link_libraries(proto_lib PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc
    utf8_validity  # 需要链接utf8_range的utf8_validity库 
)




# 设置proto_lib包含目录，public属性保证链接该库的目标继承它的包含目录 
target_include_directories(proto_lib PUBLIC
    ${PROTO_GENERATED_DIR}  
)
add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/gate_server)
add_subdirectory(src/status_server)
add_subdirectory(src/chat_server)

# 拷贝配置文件 
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

# 拷贝 config 目录内容到 bin
add_custom_target(copy_config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CONFIG_DIR} ${OUTPUT_DIR}
    COMMENT "Copying config files to ${OUTPUT_DIR}"
)