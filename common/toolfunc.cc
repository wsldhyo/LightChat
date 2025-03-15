#include "toolfunc.hpp"
#include <cassert>
#include <string>
#include <string_view>

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

void url_decode(std::string_view _str, std::string &_res) {
  size_t length = _str.length();
  for (size_t i = 0; i < length; i++) {
    if (_str[i] == '+') {
    // 还原+为空
      _res += ' ';
    }
    else if (_str[i] == '%' && i + 2 < length) {
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