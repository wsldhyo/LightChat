# =========================
# Utility 模块（含 cpp 库）
# =========================
add_library(common_utility STATIC
    utility/defer.cc
    utility/toolfunc.cc
    utility/tcp_msg_node.cc
)

# 暴露 src/common 路径给需要的模块
# 子模块可以直接 #include "utility/xxx.hpp"
# 如果写的是 ${CMAKE_CURRENT_SOURCE_DIR}/utility，那么可以直接#include "xxx.hpp"
target_include_directories(common_utility
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# =========================
# Pool 模块
# =========================
add_library(common_pool STATIC
    pool/iocontext_pool.cc
    pool/redis_conn_pool.cc
    pool/mysql_conn_pool.cc
    pool/rpc_conn_pool.cc
    pool/status_server_conn_pool.cc
    pool/chat_server_conn_pool.cc
)

# include Redis & grpc 生成路径
target_include_directories(common_pool PUBLIC
    ${HIREDIS_INCLUDE_DIRS}
    ${PROTO_GENERATED_DIR}
)

# 链接依赖库
target_link_libraries(common_pool
    PRIVATE common_utility           # pool 内部使用 utility
    PRIVATE ${HIREDIS_LIBRARIES}
    PRIVATE mysql::concpp-jdbc
)

# =========================
# Manager 模块
# =========================
add_library(common_manager STATIC
    manager/config_manager.cc
    manager/mysql_manager.cc
    manager/redis_manager.cc
    manager/mysql_dao.cc
    manager/distributed_lock.cc
)

# 链接 pool 模块
# common_manager 直接 include utility 头文件，保留 PRIVATE
target_link_libraries(common_manager
    PUBLIC common_pool    # 对外暴露给链接 common_manager 的模块
    PRIVATE common_utility
    PRIVATE mysql::concpp-jdbc
)

# =========================
# RPC Client 模块
# =========================
add_library(common_rpc_client STATIC
    rpc_client/status_rpc_client.cc
    rpc_client/chat_rpc_client.cc  
    rpc_client/vertify_rpc_client.cc
)

target_link_libraries(common_rpc_client
    PRIVATE common_utility
    PRIVATE common_pool
    PRIVATE proto_lib
)

# =========================
# 子项目使用示例
# =========================
# target_link_libraries(SubProjectA PRIVATE
#     common_rpc_client
#     common_manager
#     common_pool
#     common_utility
# )
#
# 子项目 include 头文件直接：
#   #include "pool/iocontext_pool.hpp"
#   #include "utility/constant.hpp"
