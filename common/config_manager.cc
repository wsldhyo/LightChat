#include "config_manager.hpp"
#include "toolfunc.hpp"
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
std::string &SectionInfo::operator[](std::string const &_key) {
  auto it = section_datas.find(_key);
  if (it == section_datas.end()) {
    static std::string null_str("");
    return null_str;
  }
  return it->second;
}

ConfigManager::ConfigManager() {
}
ErrorCode ConfigManager::parse(){

    std::string exe_path_str;  
    auto error_code = get_executable_path(exe_path_str);
    if (error_code != ErrorCode::NO_ERROR) {
        std::cout << "read config path error" << std::endl;
        return error_code;
    }

    
    std::filesystem::path config_path(exe_path_str);
    config_path = config_path.parent_path();
    config_path /= "config.ini";
    std::cout << "exe path " << exe_path_str << std::endl;
    std::cout << "config path " << config_path.string() << std::endl;
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);

    // 遍历INI文件中的所有section  
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;
        const boost::property_tree::ptree& section_tree = section_pair.second;
        // 对于每个section，遍历其所有的key-value对  
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;
            const std::string& value = key_value_pair.second.get_value<std::string>();
            section_config[key] = value;
        }
        SectionInfo sectionInfo;
        sectionInfo.section_datas = std::move(section_config);
        // 将section的key-value对保存到config_map中  
        config_map_[section_name] = std::move(sectionInfo);
    }
    // 输出所有的section和key-value对  
    for (const auto& section_entry : config_map_) {
        const std::string& section_name = section_entry.first;
        SectionInfo section_config = section_entry.second;
        std::cout << "[" << section_name << "]" << std::endl;
        for (const auto& key_value_pair : section_config.section_datas) {
            std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
        }
    }
    return ErrorCode::NO_ERROR;
  }

SectionInfo &ConfigManager::operator[](std::string const &_section) {
  auto it = config_map_.find(_section);
  if (it == config_map_.end()) {
    static SectionInfo null_section;
    return null_section;
  }
  return it->second;
}