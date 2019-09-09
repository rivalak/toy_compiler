#include "genIR.h"
#include "parser.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Analysis/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"

#include <iostream>
#include <map>
#include <vector>

using namespace frontEnd;

using namespace std;
using namespace llvm;

IRBuilder<> Builder(MyGlobalContext);
map<std::string, llvm::Value *> Named_Values;

// 为变量表达式生成IR
Value *VariableAST::Codegen() {
  Value *V = Named_Values[Var_Name];
  return V ? V : 0;
}

// 数值变量生成 IR
Value *NumericAST::Codegen() {
  return ConstantInt::get(Type::getInt32Ty(MyGlobalContext), numeric_val);
}

// 二元表达式生成IR
Value *BinaryAST::Codegen() {
  Value *L = LHS->Codegen();
  Value *R = RHS->Codegen();
  if (L == 0 || R == 0)
    return 0;

  switch (std::stoi(Bin_Operator)) {
  case '+':
    return Builder.CreateAdd(L, R, "addtmp");
  case '-':
    return Builder.CreateSub(L, R, "subtmp");
  case '*':
    return Builder.CreateMul(L, R, "multmp");
  case '/':
    return Builder.CreateUDiv(L, R, "divtmp");
  case '<':
    L = Builder.CreateICmpULT(L, R, "cmptmp");
    return Builder.CreateZExt(L, Type::getInt32Ty(MyGlobalContext), "booltmp");
  default:
    break;
  }

  Function *F = Module_Ob->getFunction(std::string("binary") + Bin_Operator);
  assert(F && "binary operator not found!");

  Value *Ops[] = {L, R};
  return Builder.CreateCall(F, Ops, "binop");
}

// 生成函数声明的 LLVM IR
Function *FunctionDeclAST::Codegen() {
  std::vector<Type *> Integers(Arguments.size(),
                               Type::getInt32Ty(MyGlobalContext));
  FunctionType *FT =
      FunctionType::get(Type::getInt32Ty(MyGlobalContext), Integers, false);
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, Func_Name, Module_Ob);

  // some mistakes
  if (F->getName() != Func_Name) {
    F->eraseFromParent();
    F = Module_Ob->getFunction(Func_Name);

    if (!F->empty())
      return 0;
    if (F->arg_size() != Arguments.size())
      return 0;
  }

  unsigned Idx = 0;
  for (Function::arg_iterator Arg_It = F->arg_begin(); Idx != Arguments.size();
       ++Arg_It, ++Idx) {
    Arg_It->setName(Arguments[Idx]);
    // what is this used for?
    Named_Values[Arguments[Idx]] = Arg_It;
  }

  return F;
}

// 生成函数定义的 LLVM IR
Function *FunctionDefnAST::Codegen() {
  Named_Values.clear();

  Function *TheFunction = Func_Decl->Codegen();
  if (TheFunction == 0)
    return 0;

  if (Func_Decl->isBinaryOp())
    Operator_Precedence[Func_Decl->getOperatorName()] =
        Func_Decl->getBinaryPrecedence();

  BasicBlock *BB = BasicBlock::Create(MyGlobalContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);

  if (Value *RetVal = Body->Codegen()) {
    Builder.CreateRet(RetVal);

    verifyFunction(*TheFunction);

    // run Global_FP passes
    Global_FP->run(*TheFunction);

    return TheFunction;
  }

  TheFunction->eraseFromParent();

  if (Func_Decl->isBinaryOp())
    Operator_Precedence.erase(Func_Decl->getOperatorName());
  return 0;
}

// 生成函数调用的 IR
Value *FunctionCallAST::Codegen() {
  Function *CalleeF = Module_Ob->getFunction(Function_Callee);
  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Function_Arguments.size(); i != e; ++i) {
    ArgsV.push_back(Function_Arguments[i]->Codegen());
    if (ArgsV.back() == 0) {
      return 0;
    }
  }

  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// if / then / else
Value *ExprIfAST::Codegen() {
  Value *CondV = Cond->Codegen();
  if (CondV == 0)
    return 0;

  CondV = Builder.CreateICmpNE(
      CondV, ConstantInt::get(Type::getInt32Ty(MyGlobalContext), 0), "ifcond");

  Function *TheFunction = Builder.GetInsertBlock()->getParent();

  BasicBlock *ThenBB = BasicBlock::Create(MyGlobalContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(MyGlobalContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(MyGlobalContext, "ifcont");

  Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  Builder.SetInsertPoint(ThenBB);

  Value *ThenV = Then->Codegen();
  if (ThenV == 0)
    return 0;

  Builder.CreateBr(MergeBB);
  ThenBB = Builder.GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);

  Value *ElseV = Else->Codegen();
  if (ElseV == 0)
    return 0;

  Builder.CreateBr(MergeBB);
  ElseBB = Builder.GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  PHINode *PN =
      Builder.CreatePHI(Type::getInt32Ty(MyGlobalContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

// for
Value *ExprForAST::Codegen() {

  Value *StartVal = Start->Codegen();
  if (StartVal == 0)
    return 0;

  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder.GetInsertBlock();
  BasicBlock *LoopBB = BasicBlock::Create(MyGlobalContext, "loop", TheFunction);

  Builder.CreateBr(LoopBB);

  Builder.SetInsertPoint(LoopBB);

  PHINode *Variable =
      Builder.CreatePHI(Type::getInt32Ty(MyGlobalContext), 2, Var_Name.c_str());
  Variable->addIncoming(StartVal, PreheaderBB);

  Value *OldVal = Named_Values[Var_Name];
  Named_Values[Var_Name] = Variable;

  if (Body->Codegen() == 0)
    return 0;

  Value *StepVal;
  if (Step) {
    StepVal = Step->Codegen();
    if (StepVal == 0)
      return 0;
  } else {

    StepVal = ConstantInt::get(Type::getInt32Ty(MyGlobalContext), 1);
  }

  Value *NextVar = Builder.CreateAdd(Variable, StepVal, "nextvar");

  Value *EndCond = End->Codegen();
  if (EndCond == 0)
    return EndCond;

  EndCond = Builder.CreateICmpNE(
      EndCond, ConstantInt::get(Type::getInt32Ty(MyGlobalContext), 0),
      "loopcond");

  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB =
      BasicBlock::Create(MyGlobalContext, "afterloop", TheFunction);

  Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

  Builder.SetInsertPoint(AfterBB);

  Variable->addIncoming(NextVar, LoopEndBB);

  if (OldVal)
    Named_Values[Var_Name] = OldVal;
  else
    Named_Values.erase(Var_Name);

  return Constant::getNullValue(Type::getInt32Ty(MyGlobalContext));
}

// Unary
Value *ExprUnaryAST::Codegen() {
  Value *OperandV = Operand->Codegen(); // TODO: virtual func

  if (OperandV == 0) { return 0; }

  Function *F = Module_Ob->getFunction(std::string("unary") + Opcode);

  if (F == 0) { return 0; }

  return Builder.CreateCall(F, OperandV, "unop");
}
