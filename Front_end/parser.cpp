/*
 * 语法分析器
 */

// local
#include "parser.h"
#include "token.h"
#include "ast.h"

#include <iostream> 
#include <vector> 
#include <map> 
#include <string> 

using namespace frontend_ast;
using namespace std;

map<char, int>Operator_Precedence;

// 持有当前 token 类型（来自词法分析器）的静态全局变量
int current_token_type;
string current_token;

// 从词法分析器的输入流获得下一个 token
void next_token() {
    current_token_type = get_token();
}

// 定义一个泛型函数，来根据由词法分析器确定的 token 类型调用特定解析函数
BaseAST* Base_Parser() {
    switch (current_token_type)
    {
    case IDENTIFIER_TOKEN :
        return identifier_parser();
    case NUMERIC_TOKEN : 
        return numeric_parser();
    case '(' :
        return paran_parser();
    default:
        return 0;
    }
}


/*
 * 解析简单的表达式
 */

// 解析数值表达式
BaseAST *numeric_parser() {
    BaseAST *Result = new NumericAST(Numeric_Val);
    next_token();
    return Result;
}

// 解析标识符表达式（包括变量和函数调用），函数调用后会跟括号
BaseAST *identifier_parser() {
    std::string IdName = Identifier_string;

    next_token();

    // 变量
    if ( current_token_type != '(' )
    {
        return new VariableAST( IdName );
    }
    
    next_token();

    // 这时是括号里的参数
    std::vector<BaseAST*> Args;
    if ( current_token_type != ')' )
    // 说明有参数
    {
        while (1)
        {
            BaseAST* Arg = expression_parser();
            if (!Arg) 
                return 0;
            Args.push_back(Arg);

            if (current_token_type == ')') break;
            
            if (current_token_type != ',')
                return 0;
            next_token();
        }
    }
    next_token();   
    
    return new FunctionCallAST( IdName, Args );
}

// 解析函数声明
FunctionDeclAST *func_decl_parser() {
    if (current_token_type != IDENTIFIER_TOKEN )
    {
        return 0;
    }

    std::string FnName = Identifier_string;
    next_token();

    if (current_token_type != '(')
    {
        return 0;
    }

    std::vector<std::string> Function_Argument_Names;
    next_token();
    while (current_token_type == IDENTIFIER_TOKEN)
    {
        Function_Argument_Names.push_back(Identifier_string);
        next_token();
    }

    if (current_token_type != ')')
    {
        return 0;
    }

    next_token();

    return new FunctionDeclAST( FnName, Function_Argument_Names );
}

// 解析函数定义
FunctionDefnAST *func_defn_parser() {
    next_token();

    FunctionDeclAST *Decl = func_decl_parser();
    if (Decl == 0)
    {
        return 0;
    }

    if (BaseAST *Body = expression_parser() )
    {
        return new FunctionDefnAST( Decl, Body );
    }
    
    return 0;
}

BaseAST* expression_parser() {
    BaseAST *LHS = Base_Parser();
    if (!LHS)
    {
        return 0;
    }
    
    return binary_op_parser(0, LHS);
}

// 初始化优先级
void init_precedence() {
    Operator_Precedence['-'] = 1;
    Operator_Precedence['+'] = 2;
    Operator_Precedence['/'] = 3;
    Operator_Precedence['*'] = 4;
}

// helper function: 返回已定义的二元运算符的优先级
int getBinOpPrecedence() {
    if (!isascii(current_token_type) )
    {
        return -1;
    }

    int TokPrec = Operator_Precedence[current_token_type];
    if (TokPrec <= 0 ) return -1;

    return TokPrec;
}

// 解析binary 运算符的解析器
BaseAST* binary_op_parser(int Old_Prec, BaseAST *LHS) {
    while (1) {
        int Operator_Prec = getBinOpPrecedence();
        if (Operator_Prec < Old_Prec )
        {
            return LHS;
        }

        int BinOp = current_token_type;
        next_token();

        BaseAST* RHS = Base_Parser();
        if (!RHS)
        {
            return 0;
        }

        int Next_Prec = getBinOpPrecedence();
        if (Operator_Prec < Next_Prec )
        {
            RHS = binary_op_parser(Operator_Prec + 1, RHS );
            if (RHS == 0) return 0;
        }
        
        LHS = new BinaryAST(std::to_string(BinOp), LHS, RHS);
    }
}

// 括号 
BaseAST* paran_parser() {
    next_token();
    BaseAST* V = expression_parser();
    if (!V)
    {
        return 0;
    }
    
    if (current_token_type != ')')
    {
        return 0;
    }
    
    return V;
} 

FunctionDefnAST *top_level_parser() {
  if (BaseAST *E = expression_parser()) {
    FunctionDeclAST *Func_Decl =
        new FunctionDeclAST("", std::vector<std::string>());
    return new FunctionDefnAST(Func_Decl, E);
  }
  return 0;
}
