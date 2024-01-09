#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"
//#include "llvm/IR/LegacyPassManager.h"
//#include "llvm/Target/TargetOptions.h"


#include <llvm/Support/TargetSelect.h>

#include <llvm/Support/Host.h>



#include <system_error>
#include <iostream>

using namespace llvm;

int main() {
//    InitializeNativeTarget();
//    InitializeNativeTargetAsmPrinter();
//    InitializeNativeTargetAsmParser();

    LLVMContext context;
    StringRef module_name = "test";
    auto *module = new Module(module_name, context);
    IntegerType *int_ty = Type::getInt32Ty(context);
    FunctionType *func_ty = FunctionType::get(int_ty, false);
    StringRef func_name = "main";
    Function *func = Function::Create(func_ty, Function::ExternalLinkage, func_name, *module);
    StringRef block_name = "entry";
    BasicBlock *basic_block = BasicBlock::Create(context, block_name, func);

    Value *x1 = ConstantInt::get(int_ty, 10, false);
    Value *x2 = ConstantInt::get(int_ty, 10, false);

    Instruction *add = BinaryOperator::Create(Instruction::Add, x1, x2, "add_result");
    add->insertInto(basic_block, basic_block->end());

    ReturnInst::Create(context, add)->insertInto(basic_block, basic_block->end());

    raw_ostream &outputStream = outs();
    AssemblyAnnotationWriter *annotationWriter = nullptr;
    module->print(outputStream, annotationWriter, false, false);

    std::error_code EC;
    raw_fd_ostream OS("main.bc", EC, sys::fs::OF_None);
    if (EC)
        return -1;
    WriteBitcodeToFile(*module, OS);

    delete module;
    printf("Done\n");
    return 0;
}
