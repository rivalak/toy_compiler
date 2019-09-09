#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>

extern int Numeric_Val; // 数值
extern std::string Identifier_string; // 字符串名字
extern FILE *file;

// define token type
enum Token_Type {
    EOF_TOKEN          = -1,    // 文件的结束
    NUMERIC_TOKEN      = -2,    // 当前token 是数值类型的
    IDENTIFIER_TOKEN   = -3, // 当前token 是标识符
    PARAN_TOKEN        = -4,      // 括号
    DEF_TOKEN          = -5,        // def 声明
    
    IF_TOKEN           = -6,  
    THEN_TOKEN         = -7,
    ELSE_TOKEN         = -8,

    FOR_TOKEN         = -9, 
    IN_TOKEN          = -10,

    BINARY_TOKEN      = -11,
    UNARY_TOKEN       = -12,
};



// 忽略标点符号、定界符（通常是空格、换行）
// TODO: file
int get_token();

#endif
