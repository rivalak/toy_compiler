#include "ast.h"

using namespace::std;
using namespace::llvm;

using namespace::frontend_ast;

llvm::IRBuilder<> Builder(MyGlobalContext);
std::map<std::string, llvm::Value*> Named_Values;

BaseAST::~BaseAST() {}

VariableAST::VariableAST(std::string &name) : Var_Name(name) {}
// 为变量表达式生成IR
Value *VariableAST::Codegen() {
    Value *V = Named_Values[Var_Name];
    return V ? V : 0;
}

NumericAST::NumericAST(int val) : numeric_val(val) {}
// 数值变量生成 IR
Value *NumericAST::Codegen() {
    return ConstantInt::get(Type::getInt32Ty(MyGlobalContext ), numeric_val);
}

BinaryAST::BinaryAST(std::string op, BaseAST *lhs, BaseAST *rhs) : Bin_Operator(op), LHS(lhs), RHS(rhs) {}
// 二元表达式生成IR
Value *BinaryAST::Codegen() {
    Value *L = LHS->Codegen();
    Value *R = RHS->Codegen();
    if (L == 0 || R == 0)
    {
        return 0;
    }

    switch (atoi(Bin_Operator.c_str() ) )
    {
    case '+' : 
        return Builder.CreateAdd(L, R, "addtmp");
    case '-' : 
        return Builder.CreateAdd(L, R, "subtmp");
    case '*' : 
        return Builder.CreateAdd(L, R, "multmp");
    case '/' : 
        return Builder.CreateAdd(L, R, "divtmp");
    default:
        return 0;
    }
    
}

FunctionDeclAST::FunctionDeclAST(const std::string &name, const std::vector<std::string> &args) : Func_Name(name), Arguments(args) {}
///
Function *FunctionDeclAST::Codegen() {
    std::vector<Type*> Integers(Arguments.size(), Type::getInt32Ty(MyGlobalContext) );
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(MyGlobalContext), Integers, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, Func_Name, Module_Ob);

    if (F->getName() != Func_Name)
    {
        F->eraseFromParent();
        F = Module_Ob->getFunction(Func_Name);

        if (!F->empty() ) return 0;
        if (F->arg_size() != Arguments.size() ) return 0;
    }
    
    unsigned Idx = 0;
    for (Function::arg_iterator Arg_It = F->arg_begin(); Idx != Arguments.size(); ++Arg_It, ++Idx)
    {
        Arg_It->setName(Arguments[Idx]);
        Named_Values[Arguments[Idx]] = Arg_It;
    }
    
    return F;
}

FunctionDefnAST::FunctionDefnAST(FunctionDeclAST *proto, BaseAST *body) : Func_Decl(proto), Body(body) {}
///
Function *FunctionDefnAST::Codegen() {
    Named_Values.clear();

    Function *TheFunction = Func_Decl->Codegen();

    if (TheFunction == 0)
    {
        return 0;
    }

    BasicBlock *BB = BasicBlock::Create(MyGlobalContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    if (Value *RetVal = Body->Codegen() )
    {
        Builder.CreateRet(RetVal);
        verifyFunction(*TheFunction);
        return TheFunction;
    }
    
    TheFunction->eraseFromParent();
    return 0;
}

FunctionCallAST::FunctionCallAST(const std::string &callee, std::vector<BaseAST*> &args) : Function_Callee(callee), Function_Arguments(args) {}
///
Value *FunctionCallAST::Codegen() {
    Function *CalleeF = Module_Ob->getFunction(Function_Callee);
    std::vector<Value*> ArgsV;
    for (unsigned i =0, e = Function_Arguments.size(); i != e; ++i)
    {
        ArgsV.push_back(Function_Arguments[i]->Codegen() );
        if (ArgsV.back() == 0)
        {
            return 0;
        }
    }
    
    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
} 
