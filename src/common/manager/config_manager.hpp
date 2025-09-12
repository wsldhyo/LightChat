#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP
#include "utility/constant.hpp"
#include "utility/singleton.hpp"
#include <optional>
#include <string>
#include <unordered_map>
/**
 * @brief 表示一个 INI 配置文件中的 section 信息
 *
 * SectionInfo 管理一个 section 内的键值对，可以通过 operator[]
 * 直接访问或插入新键值， 也可以通过 get
 * 方法以更安全的方式获取值（避免插入空字符串）。
 */
struct SectionInfo {
  /**
   * @brief 通过 key 访问或插入新的键值
   *
   * 如果 key 不存在，会在 section_datas 中插入一个空字符串。
   * @param _key 键名
   * @return std::string& 对应的值引用
   */
  std::string &operator[](std::string const &_key);

  /**
   * @brief 根据 key 获取对应的值（安全访问）
   *
   * @param key 键名
   * @return std::optional<std::string> 如果 key 不存在返回
   * std::nullopt，否则返回对应值
   */
  std::optional<std::string> get(const std::string &key) const;

  /// 存储 section 内的 key-value，析构时自动释放
  std::unordered_map<std::string, std::string> section_datas;
};

/**
 * @brief 配置管理器，负责解析并提供对配置文件的访问
 *
 * 单例模式实现，通过 Singleton<ConfigManager> 获取全局实例。
 */
class ConfigManager : public Singleton<ConfigManager> {
public:
  /**
   * @brief 解析配置文件并加载到内存
   *
   * 默认读取可执行文件目录下的 "config.ini" 文件。
   *
   * @return ErrorCodes 解析结果，ErrorCodes::NO_ERROR 表示成功
   */
  ErrorCodes parse(std::string_view path);

  /**
   * @brief 通过 section 名称访问或创建 SectionInfo
   *
   * 注意：如果 section 不存在，会自动创建新的 SectionInfo，
   * 可能导致误用。
   *
   * @param section section 名称
   * @return SectionInfo& 对应的 section 引用
   */
  SectionInfo &operator[](const std::string &section);

  /**
   * @brief 更安全的访问 section
   *
   * 如果 section 不存在，则返回 nullopt。
   *
   * @param section section 名称
   * @return std::optional<std::reference_wrapper<const SectionInfo>>
   */
  std::optional<std::reference_wrapper<const SectionInfo>>
  get(const std::string &section) const;

  /**
   * @brief 一次性获取配置值，如果不存在则返回默认值
   *
   * @param section section 名称
   * @param key key 名称
   * @param default_val key 不存在时返回的默认值，默认空字符串
   * @return std::string 配置值或默认值
   */
  
  /**
   * @brief 打印map中存储的所有值
   * @param void
   * @return void
  */
  void print();
  std::string get_value(const std::string &section, const std::string &key,
                        const std::string &default_val = "") const;

private:
  friend class Singleton<ConfigManager>;
  ConfigManager();
  /// 存储所有 section 信息，析构时自动释放
  std::unordered_map<std::string, SectionInfo> config_map_;
};
#endif