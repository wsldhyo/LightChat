#ifndef TOOL_FUNC_HPP
#define TOOL_FUNC_HPP
#include "constant.hpp"
#include <charconv>
#include <string>
#include <iostream>

/**
  @brief 将整数转换为对应的 ASCII 十六进制字符 ('0'-'9', 'A'-'F')

  @param _num 要转换的4bit二进制整数（即0-15内的整数）
  @return unsigned char 转换后的十六进制字符
 */
unsigned char to_hex(int _num);

/**
  @brief 将ASCII编码的十六进制字符转换为十进制整数

  @param ch 待转换的字符('0'-'9', 'a'-'f', ‘A’-'F')
  @return int 转换后的十进制整数
 */
int from_hex(unsigned char ch);

/**
  @brief 将字符串转换为标准的 URL 编码格式（Percent-Encoding）

  @param str 输入的原始字符串
  @param _res [out] URL 编码后的字符串

  @note
  - 依据现代web标准，str应当是UTF-8编码的字节流
  - 转换时，直接保留的字符：字母 (a-z, A-Z)、数字 (0-9)、`-`、`_`、`.`、`~`。
  - 空格' '转为'+'
  - 其他字符（如中文、特殊符号）转换为 `%XX` 格式，其中 `XX`
  为该字符每个字节的十六进制表示（每个字节的高四位和低四位依次转换）。
 */
void url_encode(std::string_view const _str, std::string &_res);

/**
  @brief 对 URL 进行解码，恢复原始字符串 (UTF-8字节流)
         HTTP请求中，GET 请求的数据通常放在 URL 中，也就是查询字符串（query
  string。 这些数据被?分割为两部分，前一部分为url，后一部分为参数。
         这里解码的是参数部分

  @param _str  输入的编码后的参数字符串
  @param _res  [out] 解码后的字符串 (UTF-8 字节流)
  @details
   - `+` 号转换为空格 `' '` (适用于 `application/x-www-form-urlencoded` 编码)
   - `%XX` 形式的字符转换为对应的字节 (支持 UTF-8 字符)
   - 其他字符保持不变

  @note 该函数假设 `_str` 为合法的 URL 编码格式，调用时需确保输入正确
  @example
   ```cpp
   std::string encoded = "Hello%20World%21+%E4%BD%A0%E5%A5%BD";
   std::string decoded;
   url_decode(encoded, decoded);
   std::cout << decoded;  // 输出: "Hello World! 你好"
   ```
 */
void url_decode(std::string_view const _str, std::string &_res);

/**
 * @brief 获取当前进程的可执行文件的绝对路径。
 *
 * 根据不同平台调用对应的系统 API：
 *   - Windows: 使用 GetModuleFileNameW()
 *   - macOS: 使用 _NSGetExecutablePath()
 *   - Linux: 读取 /proc/self/exe
 *
 * 获取到路径后，会调用 std::filesystem::canonical 对路径进行规范化，
 * 解析符号链接并转换为绝对路径，结果保存在成员变量 `_res` 中。
 *
 * @return ErrorCode
 *   - ErrorCode::NO_ERROR 成功获取到路径
 *   - ErrorCode::PATH_DO_NOT_EXIST 路径不存在或 canonical 失败
 *   - ErrorCode::PLATFORM_NOT_SUPPORT 当前平台不支持
 */
ErrorCodes get_executable_path(std::string &_res);

/**
 * @brief 将字符串安全转换为整数类型
 *
 * @tparam Int 整数类型（int, long, long long, unsigned 等）
 * @param str 要转换的字符串
 * @param out 转换后的整数结果（输出参数）
 * @return ErrorCodes
 *    -
 *    -
 */
template <std::integral Int>
ErrorCodes string_to_int(const std::string &str, Int &out) {
  if(str.empty())
  {
    std::cout << "arg is nullstr\n";
    return ErrorCodes::INVALID_ARGUMENT;
  }
  const char *begin = str.data();
  const char *end = str.data() + str.size();

  auto result = std::from_chars(begin, end, out);

  if (result.ec == std::errc()) {
    // 成功
    return ErrorCodes::NO_ERROR;
  } else if (result.ec == std::errc::result_out_of_range) {
    return ErrorCodes::OUT_OF_RANGE;
    std::cout << "charconv out of range:" << str << '\n';
  } else {
    return ErrorCodes::INVALID_ARGUMENT;
    std::cout << "charconv invalid argument" << str << '\n';
  }
}


std::string generate_unique_string();
#endif