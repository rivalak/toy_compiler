
#include <iostream>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"

#include "token.h"
#include "ast.h"
#include "parser.h"

using namespace frontend_ast;
using namespace llvm;
using namespace std;

llvm::LLVMContext MyGlobalContext;
llvm::Module *Module_Ob;

FILE *file; // 分析的文件

// FunctionPassManager *Global_FP;

void HandleDefn() {
    if (FunctionDefnAST *F = func_defn_parser() )
    {
        if (Function* LF = F->Codegen() )
        {
            /* code */
        }
    }
    else {
        next_token();
    }
}

void HandleTopExpression() {
    if (FunctionDefnAST *F = top_level_parser() )
    {
        if (Function *LF = F->Codegen())
        {
            /* code */
        }
    }
    else {
        next_token();
    }
}

/**
 * 驱动
 */
void Driver() {
    while (1) {
        switch (current_token_type) {
            case EOF_TOKEN : 
                return;
            case ';' :
                next_token();
                break;
            case DEF_TOKEN :
                HandleDefn();
                break;
            default :
                HandleTopExpression();
                break;
        }
    }
}

int main(int argc, char const *argv[])
{
    LLVMContext &Context = MyGlobalContext;
    FunctionPassManager My_FP(Module_Ob);


    init_precedence();

    file = fopen(argv[1], "r");
    if (file == 0)
    {
        printf("Could not open file\n");
    }
    next_token();
    Module_Ob = new Module("my compiler", Context);
    // My_FP.add(createBasicAliasAnalysisPass());
    // My_FP.doInitialization();

    Driver();

    Module_Ob->dump();
    
    return 0;
}
