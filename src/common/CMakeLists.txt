# =========================
# Utility 模块（含 cpp 库）
# =========================
add_library(common_utility STATIC
    utility/defer.cc
    utility/toolfunc.cc
    utility/status_rpc_client.cc
    utility/tcp_msg_node.cc
)
target_include_directories(common_utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HIREDIS_INCLUDE_DIRS} ${PROTO_GENERATED_DIR})
target_link_libraries(common_utility PRIVATE ${HIREDIS_LIBRARIES} proto_lib)

# =========================
# Utility header-only 模块（纯头文件）
# =========================
add_library(common_utility_headers INTERFACE)
target_include_directories(common_utility_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# =========================
# Pool 模块
# =========================
add_library(common_pool STATIC
    pool/iocontext_pool.cc
    pool/redis_conn_pool.cc
    pool/mysql_conn_pool.cc
    pool/rpc_conn_pool.cc
    pool/status_server_conn_pool.cc
)

#PUBLIC 让 链接该库的子项目可以访问头文件
# redis_conn_pool需要Redis头文件，需要额外包含
target_include_directories(common_pool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HIREDIS_INCLUDE_DIRS})
# rpc_conn_pool需要grpc生成的文件
target_link_libraries(common_pool PRIVATE common_utility ${HIREDIS_LIBRARIES} mysql::concpp-jdbc)




# =========================
# Manager 模块
# =========================
add_library(common_manager STATIC
    manager/config_manager.cc
    manager/mysql_manager.cc
    manager/redis_manager.cc
    manager/mysql_dao.cc
)
#redis_manager需要Redis头文件
target_include_directories(common_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_manager PRIVATE common_pool mysql::concpp-jdbc )

# =========================
# 使用示例（在子项目 CMakeLists.txt）
# =========================
# target_link_libraries(SubProjectA PRIVATE
#     common_pool
#     common_manager
#     common_utility
#     common_utility_headers
# )

# 子项目文件里include如下即可, 不需要指定相对路径
#  #include "pool/iocontext_pool.hpp"