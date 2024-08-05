cmake_minimum_required(VERSION 3.25)

project (CCompiler)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(CCompiler compiler.c lexer.c char_buffer.c parser.c)

target_include_directories(CCompiler PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(CCompiler PRIVATE ${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs core)

target_link_libraries(CCompiler PRIVATE ${llvm_libs})

