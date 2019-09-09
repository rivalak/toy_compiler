/*
 * 词法分析器
 */

#include "token.h"
#include "parser.h"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

int Numeric_Val;               // 数值
std::string Identifier_string; // 字符串名字

// 忽略标点符号、定界符（通常是空格、换行）
// TODO: file
int get_token() {
  static int LastChar = ' ';

  // 忽略空格
  while (isspace(LastChar))
    LastChar = fgetc(file);

  // get 标识符名  注：标识符不能以数字开头
  if (isalpha(LastChar)) {
    Identifier_string = LastChar;
    while (isalnum((LastChar = fgetc(file))))
      Identifier_string += LastChar;

    current_token = Identifier_string;

    if (Identifier_string == "def")
      return DEF_TOKEN;
    if (Identifier_string == "if")
      return IF_TOKEN;
    if (Identifier_string == "then")
      return THEN_TOKEN;
    if (Identifier_string == "else")
      return ELSE_TOKEN;
    if (Identifier_string == "for")
      return FOR_TOKEN;
    if (Identifier_string == "in")
      return IN_TOKEN;
    if (Identifier_string == "binary")
      return BINARY_TOKEN;
    if (Identifier_string == "unary")
      return UNARY_TOKEN;

    // 2 标识符
    return IDENTIFIER_TOKEN;
  }

  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = fgetc(file);
    } while (isdigit(LastChar) || LastChar == '.');

    Numeric_Val = strtod(NumStr.c_str(), 0);
    // 3 数值
    current_token = NumStr;
    return NUMERIC_TOKEN;
  }

  if (LastChar == '#') {
    do
      LastChar = fgetc(file);
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return get_token();
  }

  if (LastChar == EOF) {
    return EOF_TOKEN;
  }

  int ThisChar = LastChar;
  LastChar = fgetc(file);

  // 4 '(' or ')'
  current_token = ThisChar + '0' - 48;
  return ThisChar;
}
