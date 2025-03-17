#ifndef TOOL_FUNC_HPP
#define TOOL_FUNC_HPP
#include <string>
#include <string_view>
#include "constant.hpp"
/**
 * @brief 将整数转换为对应的 ASCII 十六进制字符 ('0'-'9', 'A'-'F')
 *
 * @param _num 要转换的四位二进制整数（范围：0-15）
 * @return unsigned char 转换后的十六进制字符
 */
unsigned char to_hex(int _num);

/**
 * @brief 将ASCII编码的十六进制字符转换为十进制整数
 *
 * @param ch 待转换的字符('0'-'9', 'a'-'f', ‘A’-'F')
 * @return int 转换后的十进制整数
 */
int from_hex(unsigned char ch);

/**
 * @brief 将字符串转换为标准的 URL 编码格式（Percent-Encoding）
 *
 * @param str 输入的原始字符串
 * @param _res [out] URL 编码后的字符串
 *
 * @note
 * - 依据现代web标准，str应当是UTF-8编码的字节流
 * - 转换时，直接保留的字符：字母 (a-z, A-Z)、数字 (0-9)、`-`、`_`、`.`、`~`。
 * - 空格' '转为'+'
 * - 其他字符（如中文、特殊符号）转换为 `%XX` 格式，其中 `XX`
 * 为该字符每个字节的十六进制表示（每个字节的高四位和低四位依次转换）。
 */
void url_encode(std::string_view const _str, std::string &_res);

/**
 * @brief 对 URL 进行解码，恢复原始字符串 (UTF-8字节流)
 * 
 * @param _str  输入的 URL 编码字符串
 * @param _res  [out] 解码后的字符串 (UTF-8 字节流)
 * @details
 *  - `+` 号转换为空格 `' '` (适用于 `application/x-www-form-urlencoded` 编码)
 *  - `%XX` 形式的字符转换为对应的字节 (支持 UTF-8 字符)
 *  - 其他字符保持不变
 * 
 * @note 该函数假设 `_str` 为合法的 URL 编码格式，调用时需确保输入正确
 * @example
 *  ```cpp
 *  std::string encoded = "Hello%20World%21+%E4%BD%A0%E5%A5%BD";
 *  std::string decoded;
 *  url_decode(encoded, decoded);
 *  std::cout << decoded;  // 输出: "Hello World! 你好"
 *  ```
 */
void url_decode(std::string_view _str, std::string& _res);


ErrorCode get_executable_path(std::string& _res);
#endif