#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <string>

// local
#include "genIR.h"

extern int current_token_type;
extern std::string current_token;

// 从词法分析器的输入流获得下一个 token
void next_token();

// 定义一个泛型函数，来根据由词法分析器确定的 token 类型调用特定解析函数
frontEnd::BaseAST *Base_Parser();

// 解析数值表达式
frontEnd::BaseAST *numeric_parser();

// 解析标识符表达式（包括变量和函数调用），函数调用后会跟括号
frontEnd::BaseAST *identifier_parser();

// 解析函数声明
frontEnd::FunctionDeclAST *func_decl_parser();

// 解析函数定义
frontEnd::FunctionDefnAST *func_defn_parser();

// 用一个map 来存储运算符优先级
// 此处只考虑 '- < + < / < *'
extern std::map<char, int> Operator_Precedence;

frontEnd::BaseAST *expression_parser();

// 初始化优先级
void init_precedence();

// helper function: 返回已定义的二元运算符的优先级
int getBinOpPrecedence();

// 解析binary 运算符的解析器
frontEnd::BaseAST *binary_op_parser(int Old_Prec,
                                        frontEnd::BaseAST *LHS);

// 括号
frontEnd::BaseAST *paran_parser();

frontEnd::FunctionDefnAST *top_level_parser();

frontEnd::BaseAST *If_parser();

frontEnd::BaseAST *For_parser();

frontEnd::BaseAST *unary_parser();

#endif
