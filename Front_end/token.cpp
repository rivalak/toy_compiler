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

int Numeric_Val; // 数值
std::string Identifier_string; // 字符串名字

// 忽略标点符号、定界符（通常是空格、换行）
// TODO: file
int get_token() {
    static int LastChar = ' ';

    // 忽略空格
    while ( isspace(LastChar) ) 
        LastChar = fgetc(file);

    // get 标识符名  注：标识符不能以数字开头
    if ( isalpha(LastChar) ) {
        Identifier_string = LastChar;
        while ( isalnum( ( LastChar = fgetc(file) ) ) )
            Identifier_string += LastChar;

        if (Identifier_string == "def") {
            // 1 def
            current_token = Identifier_string;
            return DEF_TOKEN;

        }
        
        // 2 标识符
        current_token = Identifier_string;
        return IDENTIFIER_TOKEN;
    }

    if ( isdigit(LastChar) )
    {
        std::string NumStr;
        do
        {
            NumStr += LastChar;
            LastChar = fgetc(file);
        } while (isdigit(LastChar) );
        
        Numeric_Val = strtod(NumStr.c_str(), 0);
        // 3 数值 
        current_token = NumStr;
        return NUMERIC_TOKEN;
    }

    if ( LastChar == '#' )
    {
        do LastChar = fgetc( file );
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF) 
            return get_token();
    }
        
    if (LastChar == EOF)
    {
        return EOF_TOKEN;
    }

    int ThisChar = LastChar;
    LastChar = fgetc(file);

    // 4 '(' or ')'
    current_token = ThisChar + '0' - 48;
    return ThisChar;
}
