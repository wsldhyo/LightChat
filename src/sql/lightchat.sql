
-- 创建好友申请列表
-- 如果表 friend_apply 已经存在，则删除（避免重复建表时报错）
DROP TABLE IF EXISTS `friend_apply`;

-- 创建好友申请表
CREATE TABLE `friend_apply`  (
  `id` bigint NOT NULL AUTO_INCREMENT,              -- 主键，自增 ID，唯一标识一条记录
  `from_uid` int NOT NULL,                          -- 发起申请的用户ID
  `to_uid` int NOT NULL,                            -- 接收申请的用户ID
  `status` smallint NOT NULL DEFAULT 0,             -- 申请状态（0=待处理，1=同意，2=拒绝...）

  PRIMARY KEY (`id`) USING BTREE,                   -- 主键索引，基于BTREE存储，加快按id查询
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE 
                                                    -- 唯一索引，限制同一用户(from_uid)不能对同一个用户(to_uid)重复申请好友
) ENGINE = InnoDB                                   -- 存储引擎：InnoDB，支持事务和行级锁
  AUTO_INCREMENT = 1                               -- 自增主键的起始值为 68
  CHARACTER SET = utf8mb4                           -- 字符集：utf8mb4，支持 emoji
  COLLATE = utf8mb4_unicode_ci                      -- 排序规则：不区分大小写的 Unicode 排序
  ROW_FORMAT = Dynamic;                             -- 行格式：动态，适合变长字段存储
