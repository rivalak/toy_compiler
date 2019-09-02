/*
 * 定义抽象语法树数据结构
 */

#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"

extern llvm::Module *Module_Ob;
extern llvm::LLVMContext MyGlobalContext;
extern llvm::IRBuilder<> Builder;
extern std::map<std::string, llvm::Value*> Named_Values;

namespace frontend_ast {
    // bass 类解析表达式
    class BaseAST {

        public:
        virtual ~BaseAST();
        virtual llvm::Value* Codegen() = 0;
    };

    // 用于变量表达式
    class VariableAST : public BaseAST {
        std::string Var_Name;
        // 定义string 对象用作存储变量名

        public:
        // 变量AST 类的含参构造函数，由传入构造函数的字符串初始化
        VariableAST(std::string &name);
        virtual llvm::Value* Codegen();
    };

    // 用于数值表达式
    class NumericAST : public BaseAST {
        int numeric_val;

        public:
        NumericAST(int val);
        virtual llvm::Value* Codegen();
    };

    // 用于二元运算组成的表达式
    class BinaryAST : public BaseAST {
        std::string Bin_Operator; // 用于存储二元运算符的string 对象
        // 由于 LHS 和 RHS 二元操作可以是任何类型，因此用BaseAST 对象存储
        BaseAST *LHS, *RHS; // 用于存储一个二元表达式的LHS 和 RHS 的对象

        public:
        // 初始化二元运算符、二元表达式的 LHS 和 RHS
        BinaryAST(std::string op, BaseAST *lhs, BaseAST *rhs);
        virtual llvm::Value* Codegen();
    };

    // 用于函数声明
    class FunctionDeclAST {
        std::string Func_Name;
        std::vector<std::string> Arguments;
        
        public:
        FunctionDeclAST(const std::string &name, const std::vector<std::string> &args);
        llvm::Function *Codegen();
    };

    // 用于函数定义
    class FunctionDefnAST {
        FunctionDeclAST *Func_Decl;
        BaseAST *Body;

        public:
        FunctionDefnAST(FunctionDeclAST *proto, BaseAST *body);
        llvm::Function *Codegen();
    };

    // 用于函数调用
    class FunctionCallAST : public BaseAST {
        std::string Function_Callee;
        std::vector<BaseAST*> Function_Arguments;

        public:
        FunctionCallAST(const std::string &callee, std::vector<BaseAST*> &args);
        virtual llvm::Value* Codegen();
    };
} // namespace: frontend_ast

#endif // MACROA(AST_H)
