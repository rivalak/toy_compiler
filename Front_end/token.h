#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>

// define token type
enum Token_Type {
    EOF_TOKEN = 0,    // 文件的结束
    NUMERIC_TOKEN = 1,    // 当前token 是数值类型的
    IDENTIFIER_TOKEN = 2, // 当前token 是标识符
    PARAN_TOKEN = 3,      // 括号
    DEF_TOKEN = 4,        // def 声明
};


extern int Numeric_Val; // 数值
extern std::string Identifier_string; // 字符串名字
extern FILE *file;

// 忽略标点符号、定界符（通常是空格、换行）
// TODO: file
int get_token();

#endif
