#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"

// add passes
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"

// JIT support
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include "genIR.h"
#include "parser.h"
#include "token.h"

using namespace llvm;
using namespace std;

using namespace frontEnd;

llvm::LLVMContext MyGlobalContext;
llvm::Module *Module_Ob;

ExecutionEngine *TheExecutionEngine;

FILE *file; // 分析的文件

legacy::FunctionPassManager *Global_FP;

void HandleDefn() {
  if (FunctionDefnAST *F = func_defn_parser()) {
    if (Function *LF = F->Codegen()) {
      /* code */
    }
  } else {
    next_token();
  }
}

void HandleTopExpression() {
  if (FunctionDefnAST *F = top_level_parser()) {
    if (Function *LF = F->Codegen()) {
      // JIT
      LF->dump();
      void *FPtr = TheExecutionEngine->getPointerToFunction(LF);
      double (*FP)() = (double (*)())(intptr_t)FPtr;
      fprintf(stderr, "Evaluated to %f\n", FP());
    }
  } else {
    next_token();
  }
}

/**
 * 驱动
 */
void Driver() {
  while (1) {
    switch (current_token_type) {
    case EOF_TOKEN:
      return;
    case ';':
      next_token();
      break;
    case DEF_TOKEN:
      HandleDefn();
      break;
    default:
      HandleTopExpression();
      break;
    }
  }
}

int main(int argc, char const *argv[]) {
  LLVMContext &Context = MyGlobalContext;

  init_precedence();
  std::unique_ptr<Module> Owner = make_unique<Module>("my compiler", Context);
  Module_Ob = Owner.get();

  std::string ErrStr;
  TheExecutionEngine =
      EngineBuilder(std::move(Owner))
          .setErrorStr(&ErrStr)
          .setMCJITMemoryManager(llvm::make_unique<SectionMemoryManager>())
          .create();

  file = fopen(argv[1], "r");
  if (file == 0) {
    printf("Could not open file\n");
  }

  next_token();

  Module_Ob = new Module(argv[1], Context);
  legacy::FunctionPassManager My_FP(Module_Ob);

  // My_FP.add(createBasicAliasAnalysisPass());
  My_FP.add(createCostModelAnalysisPass());
  My_FP.add(createInstructionCombiningPass());
  My_FP.add(createReassociatePass());
  My_FP.add(createNewGVNPass());
  My_FP.add(createCFGSimplificationPass());

  My_FP.doInitialization();

  Global_FP = &My_FP;

  Driver();

  Global_FP = 0;

  Module_Ob->dump();

  return 0;
}
