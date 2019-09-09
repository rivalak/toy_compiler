#ifndef AST_H
#define AST_H

/**
 * 定义抽象语法树数据结构
 * 并生成 LLVM IR
 */

#include <iostream>
#include <map>
#include <vector>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Analysis/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/DataLayout.h"

extern llvm::Module *Module_Ob;
extern llvm::LLVMContext MyGlobalContext;
extern llvm::IRBuilder<> Builder;
extern std::map<std::string, llvm::Value *> Named_Values;
extern llvm::legacy::FunctionPassManager *Global_FP;

namespace frontEnd {
// bass 类解析表达式
class BaseAST {
public:
  virtual ~BaseAST() {}

  virtual llvm::Value *Codegen() = 0;
};

// 用于变量
class VariableAST : public BaseAST {
  std::string Var_Name;

public:
  VariableAST(std::string &name) : Var_Name(name) {}

  virtual llvm::Value *Codegen();
};

// 用于数值
class NumericAST : public BaseAST {
  int numeric_val;

public:
  NumericAST(int val) : numeric_val(val) {}

  virtual llvm::Value *Codegen();
};

// 用于二元运算组成的表达式
class BinaryAST : public BaseAST {
  std::string Bin_Operator;
  BaseAST *LHS, *RHS;

public:
  BinaryAST(std::string op, BaseAST *lhs, BaseAST *rhs)
      : Bin_Operator(op), LHS(lhs), RHS(rhs) {}

  virtual llvm::Value *Codegen();
};

// 用于函数声明 及 运算符重载
class FunctionDeclAST {
  std::string Func_Name;
  std::vector<std::string> Arguments;

  // for operator overloading
  bool isOperator;
  unsigned Precedence;

public:
  FunctionDeclAST(const std::string &name, const std::vector<std::string> &args,
                  bool isoperator = false, unsigned prec = 0)
      : Func_Name(name), Arguments(args), isOperator(isoperator),
        Precedence(prec) {}

  bool isUnaryOp() const { return isOperator && Arguments.size() == 1; }
  bool isBinaryOp() const { return isOperator && Arguments.size() == 2; }
  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Func_Name[Func_Name.size() - 1];
  }
  unsigned getBinaryPrecedence() const { return Precedence; }

  llvm::Function *Codegen();
};

// 用于函数定义
class FunctionDefnAST {
  FunctionDeclAST *Func_Decl;
  BaseAST *Body;

public:
  FunctionDefnAST(FunctionDeclAST *proto, BaseAST *body)
      : Func_Decl(proto), Body(body) {}

  llvm::Function *Codegen();
};

// 用于函数调用
class FunctionCallAST : public BaseAST {
  std::string Function_Callee;
  std::vector<BaseAST *> Function_Arguments;

public:
  FunctionCallAST(const std::string &callee, std::vector<BaseAST *> &args)
      : Function_Callee(callee), Function_Arguments(args) {}

  virtual llvm::Value *Codegen();
};

/**
 * 处理 if then else 控制流指令
 */
class ExprIfAST : public BaseAST {
  BaseAST *Cond, *Then, *Else;

public:
  ExprIfAST(BaseAST *cond, BaseAST *then, BaseAST *else_st)
      : Cond(cond), Then(then), Else(else_st) {}

  llvm::Value *Codegen() override;
};

// 处理 For
class ExprForAST : public BaseAST {
  std::string Var_Name;
  BaseAST *Start, *End, *Step, *Body;

public:
  ExprForAST(const std::string &varname, BaseAST *start, BaseAST *end,
             BaseAST *step, BaseAST *body)
      : Var_Name(varname), Start(start), End(end), Step(step), Body(body) {}

  llvm::Value *Codegen() override;
};

class ExprUnaryAST : public BaseAST {
  char Opcode;
  BaseAST *Operand;

public:
  ExprUnaryAST(char opcode, BaseAST *operand)
      : Opcode(opcode), Operand(operand) {}
  llvm::Value *Codegen() override;
};

} // namespace frontEnd

#endif // MACROA(AST_H)
