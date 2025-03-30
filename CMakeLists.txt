cmake_minimum_required(VERSION 3.28)
project(grpc_demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# 查找依赖项
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)   #redis
pkg_check_modules(HIREDIS REQUIRED hiredis)

# 设置 .proto  文件路径 
set(PROTO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")

# 定义生成文件输出目录

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
add_subdirectory(client)
add_subdirectory(gate_server)
add_subdirectory(chat_server)
add_subdirectory(status_server)
configure_file(${CMAKE_SOURCE_DIR}/common/config.ini ${CMAKE_BINARY_DIR}/bin/config.ini COPYONLY)