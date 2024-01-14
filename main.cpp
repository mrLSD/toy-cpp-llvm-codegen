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
#include "llvm/IR/IRBuilder.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
//#include "llvm/IR/LegacyPassManager.h"
//#include "llvm/Target/TargetOptions.h"
#include <system_error>

using namespace llvm;
using namespace llvm::orc;

void test_module() {
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

    /*
    std::error_code EC;
    raw_fd_ostream OS("main.bc", EC, sys::fs::OF_None);
    if (EC)
        return -1;
    WriteBitcodeToFile(*module, OS);

    legacy::PassManager passManager;
    passManager.add(llvm::createSomePass());
    passManager.run(*module);
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        llvm::errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    module->setDataLayout(TargetMachine->createDataLayout());

    std::error_code EC2;
    llvm::raw_fd_ostream dest("output.o", EC, llvm::sys::fs::OF_None);

    if (EC2) {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*module);
    dest.flush();
*/
//    auto builder = Builder();
//    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), builder.getInt8Ty()->getPointerTo(), true);
//    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);


    delete module;
}

ExitOnError ExitOnErr;

ThreadSafeModule createDemoModule() {
    auto Context = std::make_unique<LLVMContext>();
    auto M = std::make_unique<Module>("test", *Context);

    // Create the add1 function entry and insert this entry into module M.  The
    // function will have a return type of "int" and take an argument of "int".
    Function *Add1F =
            Function::Create(FunctionType::get(Type::getInt32Ty(*Context),
                                               {Type::getInt32Ty(*Context)}, false),
                             Function::ExternalLinkage, "add1", M.get());

    // Add a basic block to the function. As before, it automatically inserts
    // because of the last argument.
    BasicBlock *BB = BasicBlock::Create(*Context, "EntryBlock", Add1F);

    // Create a basic block builder with default parameters.  The builder will
    // automatically append instructions to the basic block `BB'.
    IRBuilder<> builder(BB);

    // Get pointers to the constant `1'.
    Value *One = builder.getInt32(1);

    // Get pointers to the integer argument of the add1 function...
    assert(Add1F->arg_begin() != Add1F->arg_end()); // Make sure there's an arg
    Argument *ArgX = &*Add1F->arg_begin();          // Get the arg
    ArgX->setName("AnArg"); // Give it a nice symbolic name for fun.

    // Create the add instruction, inserting it into the end of BB.
    Value *Add = builder.CreateAdd(One, ArgX);

    // Create the return instruction and add it to the basic block
    builder.CreateRet(Add);

    return ThreadSafeModule(std::move(M), std::move(Context));
}

int main(int argc, char *argv[]) {
    // Initialize LLVM.
    InitLLVM X(argc, argv);

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    cl::ParseCommandLineOptions(argc, argv, "HowToUseLLJIT");
    ExitOnErr.setBanner(std::string(argv[0]) + ": ");

    // Create an LLJIT instance.
    auto J = ExitOnErr(LLJITBuilder().create());
    auto M = createDemoModule();

    ExitOnErr(J->addIRModule(std::move(M)));

    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto Add1Addr = ExitOnErr(J->lookup("add1"));
    int (*Add1)(int) = Add1Addr.toPtr<int(int)>();

    int Result = Add1(42);
    outs() << "add1(42) = " << Result << "\n";

    return 0;
}