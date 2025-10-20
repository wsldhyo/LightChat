-- 创建好友申请列表
-- 如果表 friend_apply 已经存在，则删除（避免重复建表时报错）
DROP TABLE IF EXISTS `friend_apply`;
-- 创建好友申请表
CREATE TABLE `friend_apply` (
  `id` bigint NOT NULL AUTO_INCREMENT,
  -- 主键，自增 ID，唯一标识一条记录
  `from_uid` int NOT NULL,
  -- 发起申请的用户ID
  `to_uid` int NOT NULL,
  -- 接收申请的用户ID
  `status` smallint NOT NULL DEFAULT 0,
  -- 申请状态（0=待处理，1=同意，2=拒绝...）
  PRIMARY KEY (`id`) USING BTREE,
  -- 主键索引，基于BTREE存储，加快按id查询
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE -- 唯一索引，限制同一用户(from_uid)不能对同一个用户(to_uid)重复申请好友
) ENGINE = InnoDB -- 存储引擎：InnoDB，支持事务和行级锁
AUTO_INCREMENT = 1 -- 自增主键的起始值为 68
CHARACTER SET = utf8mb4 -- 字符集：utf8mb4，支持 emoji
COLLATE = utf8mb4_unicode_ci -- 排序规则：不区分大小写的 Unicode 排序
ROW_FORMAT = Dynamic;
-- 行格式：动态，适合变长字段存储
-- ----------------------------
-- Table structure for friend
-- ----------------------------
DROP TABLE IF EXISTS `friend`;
CREATE TABLE `friend` (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `self_id` int NOT NULL,
  `friend_id` int NOT NULL,
  `back` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `self_friend`(`self_id` ASC, `friend_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 89 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;


--聊天消息表
-- message_id：全局自增主键，唯一标识一条消息。
-- thread_id：会话（单聊、群聊）ID，同一会话下的所有消息共用一个 thread_id。
-- sender_id：发送者用户 ID，指向用户表的主键。
-- recv_id : 接收者用户ID，指向用户表主键
-- content：消息正文，TEXT 类型，适合存储普通文字。
-- created_at：消息创建时间，自动记录插入时刻。
-- updated_at：消息更新时间，可用于标记“撤回”（status 变更）、编辑等操作。
-- status：消息状态，用于标记未读/已读/撤回等（也可扩展更多状态）
-- 索引相关
-- 1. 主键索引：PRIMARY KEY (message_id) 用于唯一检索消息。
-- 2. 会话+时间索引：KEY (thread_id, created_at) 支持按会话分页、按时间范围查询。
-- 3. 会话+消息ID 索引：KEY (thread_id, message_id) 支持按 message_id 
--      做增量拉取（WHERE thread_id=… AND message_id > since_id）

CREATE TABLE IF NOT EXISTS `chat_message` (
  `message_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `thread_id` BIGINT UNSIGNED NOT NULL,
  `sender_id` BIGINT UNSIGNED NOT NULL,
  `recv_id` BIGINT UNSIGNED NOT NULL,
  `content` TEXT NOT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `status` TINYINT  NOT NULL DEFAULT 0 COMMENT '0未读 1已读 2撤回', 
  PRIMARY KEY(`message_id`),
  KEY `idx_thread_created` (`thread_id`, `created_at`),
  KEY `idx_thread_message` (`thread_id`, `message_id`)
)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 私聊表 user1_id和user2_id确定私聊双方
-- 索引
-- thread_id，会话id
-- uniq_private_thread:保证每对用户只能有一个私聊会话, user1_id是较小的id，保证有序对唯一
-- idx_private_user1_thread: 用于加速下列查询，查询user_id参与的所有私聊会话thread_id
--            SELECT thread_id FROM private_chat WHERE user1_id = 12345;
-- idx_private_user2_thread: 同上
CREATE TABLE IF NOT EXISTS `private_chat` (
  `thread_id`   BIGINT UNSIGNED NOT NULL COMMENT '引用chat_thread.id',
  `user1_id`    BIGINT UNSIGNED NOT NULL,
  `user2_id`    BIGINT UNSIGNED NOT NULL,
  `created_at`  TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`thread_id`),
  UNIQUE KEY `uniq_private_thread` (`user1_id`, `user2_id`), 
  KEY `idx_private_user1_thread` (`user1_id`, `thread_id`),
  KEY `idx_private_user2_thread` (`user2_id`, `thread_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- 全局聊天线程表，记录所有聊天会话（包括私聊和群聊）的id
CREATE TABLE IF NOT EXISTS `chat_thread` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `type` ENUM('private', 'group') NOT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id)
);

-- 群聊成员表，存储群聊中各成员的信息（包括角色、加入时间、禁言等
CREATE TABLE `group_chat_member` (
  `thread_id`  BIGINT UNSIGNED NOT NULL COMMENT '引用 group_chat_thread.thread_id',
  `user_id`    BIGINT UNSIGNED NOT NULL COMMENT '引用 user.user_id',
  `role`       TINYINT       NOT NULL DEFAULT 0 COMMENT '0=普通成员,1=管理员,2=创建者',
  `joined_at`  TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `muted_until` TIMESTAMP    NULL COMMENT '如果被禁言，可存到什么时候',
  PRIMARY KEY (`thread_id`, `user_id`),
  KEY `idx_user_threads` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;