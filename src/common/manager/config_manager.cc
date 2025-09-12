#include "config_manager.hpp"
#include "utility/toolfunc.hpp"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <iostream>


std::string &SectionInfo::operator[](const std::string &key) {
  return section_datas[key];
}

std::optional<std::string> SectionInfo::get(const std::string &key) const {
  auto it = section_datas.find(key);
  if (it == section_datas.end())
    return std::nullopt;
  return it->second;
}

ConfigManager::ConfigManager() {}

SectionInfo &ConfigManager::operator[](const std::string &section) {
  return config_map_[section];
}

std::optional<std::reference_wrapper<const SectionInfo>>
ConfigManager::get(const std::string &section) const {
  auto it = config_map_.find(section);
  if (it == config_map_.end())
    return std::nullopt;
  return it->second;
}

std::string ConfigManager::get_value(const std::string &section,
                                     const std::string &key,
                                     const std::string &default_val) const {
  auto sec = get(section);
  if (!sec)
    return default_val;
  auto val = sec->get().get(key);
  return val.value_or(default_val);
}
#include <map>

ErrorCodes ConfigManager::parse(std::string_view path) {
  // 获取可执行文件路径下ini的路径
  std::string exe_path_str;
  auto error_code = get_executable_path(exe_path_str);
  if (error_code != ErrorCodes::NO_ERROR) {
    std::cerr << "read config path error" << '\n';
    return error_code;
  }

  std::filesystem::path config_path(exe_path_str);
  config_path = config_path.parent_path() / path;

  std::cout << "ConfigMgr parse ini:" << config_path << '\n';
  // 使用boost库解析ini文件
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::read_ini(config_path.string(), pt);
  } catch (std::exception &e) {
    std::cerr << "Failed to read config: " << e.what() << '\n';
    return ErrorCodes::READ_CONFIG_ERROR;
  }

  // 将解析得到的值存储map中
  for (const auto &section_pair : pt) {
    const std::string &section_name = section_pair.first;
    const boost::property_tree::ptree &section_tree = section_pair.second;

    SectionInfo sectionInfo; // 创建新的 SectionInfo
    for (const auto &key_value_pair : section_tree) {
      sectionInfo.section_datas[key_value_pair.first] =
          key_value_pair.second.get_value<std::string>();
    }

    // 将section保存到config_map_，一次性移动，避免多次内存分配
    config_map_[section_name] = std::move(sectionInfo);
  }
  return ErrorCodes::NO_ERROR;
}

void ConfigManager::print() {
  // 输出所有的section和key-value对
  for (const auto &section_entry : config_map_) {
    const std::string &section_name = section_entry.first;
    SectionInfo const &section_config = section_entry.second;
    std::cout << "[" << section_name << "]" << '\n';
    for (const auto &key_value_pair : section_config.section_datas) {
      std::cout << key_value_pair.first << "=" << key_value_pair.second
                << '\n';
    }
  }
}