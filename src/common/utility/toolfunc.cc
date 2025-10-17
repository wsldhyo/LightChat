#include "toolfunc.hpp"
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <limits.h>
#include <unistd.h>
#include <array>
#include <filesystem>
#endif
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

unsigned char to_hex(int _num) { return _num > 9 ? _num + 55 : _num + 48; }

int from_hex(unsigned char _ch) {
  unsigned char res;
  if (_ch >= 'A' && _ch <= 'Z')
    res = _ch - 'A' + 10;
  else if (_ch >= 'a' && _ch <= 'z')
    res = _ch - 'a' + 10;
  else if (_ch >= '0' && _ch <= '9')
    res = _ch - '0';
  else
    assert(0);
  return res;
}

void url_encode(std::string_view const _str, std::string &_res) {
  _res = "";
  size_t length = _str.length();
  for (size_t i = 0; i < length; i++) {
    // 判断是否仅有数字和字母构成
    if (isalnum((unsigned char)_str[i]) || (_str[i] == '-') ||
        (_str[i] == '_') || (_str[i] == '.') || (_str[i] == '~'))
      _res += _str[i];
    else if (_str[i] == ' ') // 为空字符
      _res += "+";
    else {
      // 其他字符需要提前加%并且高四位和低四位分别转为16进制
      _res += '%';
      _res += to_hex((unsigned char)_str[i] >> 4);
      _res += to_hex((unsigned char)_str[i] & 0x0F);
    }
  }
}

void url_decode(std::string_view const _str, std::string &_res) {
  size_t length = _str.length();
  for (size_t i = 0; i < length; i++) {
    if (_str[i] == '+') {
      // 还原+为空
      _res += ' ';
    } else if (_str[i] == '%' && i + 2 < length) {
      // 遇到%将后面的两个字符从16进制转为char再拼接
      auto high = from_hex((unsigned char)_str[++i]);
      auto low = from_hex((unsigned char)_str[++i]);
      _res += high * 16 + low;
    } else {
      // 其他字符字节添加到字节流
      _res += _str[i];
    }
  }
}

ErrorCodes get_executable_path(std::string &_res) {
#ifdef _WIN32
  wchar_t exePath[MAX_PATH];
  if (GetModuleFileNameW(NULL, exePath, MAX_PATH) > 0) {
    std::error_code ec;
    _res =
        std::filesystem::canonical(std::filesystem::path(exePath), ec).string();
    return ec ? ErrorCode::PATH_DO_NOT_EXIST : ErrorCode::NO_ERROR;
  }
  return ErrorCode::PATH_DO_NOT_EXIST;

#elif __APPLE__
  char exePath[PATH_MAX];
  uint32_t size = sizeof(exePath);
  if (_NSGetExecutablePath(exePath, &size) == 0) {
    std::error_code ec;
    _res = std::filesystem::canonical(exePath, ec).string();
    return ec ? ErrorCode::PATH_DO_NOT_EXIST : ErrorCode::NO_ERROR;
  }
  return ErrorCode::PATH_DO_NOT_EXIST;

#elif __linux__
  std::array<char, 4096> exePath; // 避免 PATH_MAX 兼容性问题
  ssize_t len = readlink("/proc/self/exe", exePath.data(), exePath.size() - 1);
  if (len != -1) {
    exePath[len] = '\0';
    std::error_code ec;
    _res = std::filesystem::canonical(std::string(exePath.data()), ec).string();
    return ec ? ErrorCodes::PATH_DO_NOT_EXIST : ErrorCodes::NO_ERROR;
  }
  return ErrorCodes::PATH_DO_NOT_EXIST;

#else
  return ErrorCode::PLATFORM_NOT_SUPPORT;
#endif
}




std::string generate_unique_string() {
  // 创建UUID对象
  boost::uuids::uuid uuid = boost::uuids::random_generator()();

  // 将UUID转换为字符串
  std::string unique_string = to_string(uuid);
  return unique_string;
}