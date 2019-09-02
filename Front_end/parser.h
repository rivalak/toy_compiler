#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <string>

// local
#include "ast.h"

extern int current_token_type;
extern std::string current_token;

// 从词法分析器的输入流获得下一个 token
void next_token();

// 定义一个泛型函数，来根据由词法分析器确定的 token 类型调用特定解析函数
frontend_ast::BaseAST* Base_Parser();

// 解析数值表达式
frontend_ast::BaseAST *numeric_parser();


// 解析标识符表达式（包括变量和函数调用），函数调用后会跟括号
frontend_ast::BaseAST* identifier_parser();
    
// 解析函数声明
frontend_ast::FunctionDeclAST *func_decl_parser();

// 解析函数定义
frontend_ast::FunctionDefnAST* func_defn_parser();

// 用一个map 来存储运算符优先级
// 此处只考虑 '- < + < / < *'
extern std::map<char, int>Operator_Precedence;

frontend_ast::BaseAST* expression_parser();

// 初始化优先级
void init_precedence();

// helper function: 返回已定义的二元运算符的优先级
int getBinOpPrecedence();

// 解析binary 运算符的解析器
frontend_ast::BaseAST* binary_op_parser(int Old_Prec, frontend_ast::BaseAST *LHS);

// 括号 
frontend_ast::BaseAST* paran_parser();

/**
 * 高层函数封装这些parser 函数
 */

frontend_ast::FunctionDefnAST *top_level_parser();

// void HandleDefn();
// 
// void HandleTopExpression();

#endif
