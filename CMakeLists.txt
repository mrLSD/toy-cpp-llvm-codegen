cmake_minimum_required(VERSION 3.27)
project(llvm_exmpl)

set(CMAKE_CXX_STANDARD 23)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

if (${LLVM_PACKAGE_VERSION} VERSION_LESS "17.0")
    message(FATAL_ERROR "LLVM version must be at least 17.0")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(llvm_exmpl main.cpp)

llvm_map_components_to_libnames(llvm_libs
    aarch64asmparser aarch64codegen aarch64disassembler
    x86asmparser x86codegen x86disassembler x86targetmca
    orcjit
)
target_link_libraries(llvm_exmpl ${llvm_libs})
