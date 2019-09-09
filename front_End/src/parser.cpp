/*
 * 语法分析器
 */

// local
#include "parser.h"
#include "genIR.h"
#include "token.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cctype>

using namespace frontEnd;
using namespace std;

map<char, int> Operator_Precedence;

// 持有当前 token 类型（来自词法分析器）的静态全局变量
int current_token_type;
std::string current_token;

// 从词法分析器的输入流获得下一个 token
void next_token() { current_token_type = get_token(); }

// 定义一个泛型函数，来根据由词法分析器确定的 token 类型调用特定解析函数
BaseAST *Base_Parser() {
  switch (current_token_type) {
  case IDENTIFIER_TOKEN:
    return identifier_parser();
  case NUMERIC_TOKEN:
    return numeric_parser();
  case IF_TOKEN:
    return If_parser();
  case '(':
    return paran_parser();
  case FOR_TOKEN:
    return For_parser();
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
  if (current_token_type != '(') {
    return new VariableAST(IdName);
  }

  next_token();

  // 这时是括号里的参数
  std::vector<BaseAST *> Args;
  if (current_token_type != ')')
  // 说明有参数
  {
    while (1) {
      BaseAST *Arg = expression_parser();
      if (!Arg)
        return 0;
      Args.push_back(Arg);

      if (current_token_type == ')')
        break;

      if (current_token_type != ',')
        return 0;
      next_token();
    }
  }
  next_token();

  return new FunctionCallAST(IdName, Args);
}

// 解析函数声明
FunctionDeclAST *func_decl_parser() {
  std::string FnName;

  unsigned Kind = 0;
  unsigned BinaryPrecedence = 30; // TODO:

  switch (current_token_type) {
  default:
    return 0;
  case IDENTIFIER_TOKEN:
    FnName = Identifier_string;
    Kind = 0;
    next_token();
    break;
  case UNARY_TOKEN:
    next_token();
    if (!isascii(current_token_type))
      return 0;
    FnName = "unary";
    FnName += (char)current_token_type;
    Kind = 1;
    next_token();
    break;
  case BINARY_TOKEN:
    next_token();
    if (!isascii(current_token_type))
      return 0;
    FnName = "binary";
    FnName += (char)current_token_type;
    Kind = 2;
    next_token();

    if (current_token_type == NUMERIC_TOKEN) {
      if (Numeric_Val < 1 || Numeric_Val > 100)
        return 0;
      BinaryPrecedence = (unsigned)Numeric_Val;
      next_token();
    }
    break;
  }

  if (current_token_type != '(')
    return 0;

  vector<std::string> ArgNames;
  next_token();
  while (current_token_type == IDENTIFIER_TOKEN) {
    ArgNames.push_back(Identifier_string);
    next_token();
  }

  // only supports identifiers as args at this present
  if (current_token_type != ')')
    return 0;

  next_token();

  // error
  if (Kind && ArgNames.size() != Kind)
    return 0;

  return new FunctionDeclAST(FnName, ArgNames, Kind != 0, BinaryPrecedence);
}

BaseAST *expression_parser() {
  BaseAST *LHS = unary_parser();
  if (!LHS) {
    return 0;
  }

  return binary_op_parser(0, LHS);
}

// 初始化优先级
void init_precedence() {
  Operator_Precedence['<'] = 1;
  Operator_Precedence['-'] = 2;
  Operator_Precedence['+'] = 3;
  Operator_Precedence['/'] = 4;
  Operator_Precedence['*'] = 5;
}

// helper function: 返回已定义的二元运算符的优先级
int getBinOpPrecedence() {
  if (!isascii(current_token_type)) {
    return -1;
  }

  int TokPrec = Operator_Precedence[current_token_type];
  if (TokPrec <= 0)
    return -1;

  return TokPrec;
}

// 解析 binary 运算符的解析器
BaseAST *binary_op_parser(int Old_Prec, BaseAST *LHS) {
  while (1) {
    int Operator_Prec = getBinOpPrecedence();
    if (Operator_Prec < Old_Prec) {
      return LHS;
    }
    int BinOp = current_token_type;

    next_token();

    BaseAST *RHS = unary_parser();
    if (!RHS) {
      return 0;
    }

    int Next_Prec = getBinOpPrecedence();
    if (Operator_Prec < Next_Prec) {
      RHS = binary_op_parser(Operator_Prec + 1, RHS);
      if (RHS == 0)
        return 0;
    }

    LHS = new BinaryAST(std::to_string(BinOp), LHS, RHS);
  }
}

// 括号
BaseAST *paran_parser() {
  next_token();
  BaseAST *V = expression_parser();
  if (!V) {
    return 0;
  }

  if (current_token_type != ')') {
    return 0;
  }

  return V;
}

FunctionDefnAST *func_defn_parser() {
  next_token();
  FunctionDeclAST *Func_Decl = func_decl_parser();
  if (Func_Decl == 0)
    return 0;

  if (BaseAST *E = expression_parser())
    return new FunctionDefnAST(Func_Decl, E);
  return 0;
}

FunctionDefnAST *top_level_parser() {
  if (BaseAST *E = expression_parser()) {
    FunctionDeclAST *Func_Decl =
        new FunctionDeclAST("", std::vector<std::string>());
    return new FunctionDefnAST(Func_Decl, E);
  }
  return 0;
}

/**
 * 处理 if then else 控制流指令
 */
BaseAST *If_parser() {
  next_token();

  BaseAST *Cond = expression_parser();
  if (!Cond)
    return 0;

  if (current_token_type != THEN_TOKEN)
    return 0;
  next_token();

  BaseAST *Then = expression_parser();
  if (Then == 0)
    return 0;

  if (current_token_type != ELSE_TOKEN)
    return 0;

  next_token();

  BaseAST *Else = expression_parser();
  if (!Else)
    return 0;

  return new ExprIfAST(Cond, Then, Else);
}

BaseAST *For_parser() {
  next_token();

  if (current_token_type != IDENTIFIER_TOKEN)
    return 0;

  std::string IdName = Identifier_string;
  next_token();

  if (current_token_type != '=')
    return 0;
  next_token();

  // 初始
  BaseAST *Start = expression_parser();
  if (Start == 0)
    return 0;
  if (current_token_type != ',')
    return 0;
  next_token();

  // 终止条件
  BaseAST *End = expression_parser();
  if (End == 0)
    return 0;

  // 步长
  BaseAST *Step = 0;
  if (current_token_type == ',') {
    next_token();
    Step = expression_parser();
    if (Step == 0)
      return 0;
  }

  if (current_token_type != IN_TOKEN)
    return 0;
  next_token();
  // for body
  BaseAST *Body = expression_parser();
  if (Body == 0)
    return 0;

  return new ExprForAST(IdName, Start, End, Step, Body);
}

BaseAST *unary_parser() {
  // ! 
  if (!isascii(current_token_type) || current_token_type == '(' || current_token_type == ',') return Base_Parser();

  int Op = current_token_type;
  next_token();
  if (BaseAST *Operand = unary_parser())
    return new ExprUnaryAST(Op, Operand);
  return 0;
}
