#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP
#include <map>
#include <string>
#include "../../common/constant.hpp"
#include "../../common/singleton.hpp"
struct SectionInfo {
  ~SectionInfo() { section_datas.clear(); }
  std::map<std::string, std::string> section_datas;
  std::string &operator[](std::string const &_key) ;
};

class ConfigManager : public Singleton<ConfigManager> {
public:
  ErrorCode parse();
  ~ConfigManager() { config_map_.clear(); }
  SectionInfo &operator[](std::string const &_section); 

private:
friend class Singleton<ConfigManager>;
  ConfigManager();
  std::map<std::string, SectionInfo> config_map_;
};
#endif