#pragma once

#include <llvm-c/Object.h>

#include "token.h"

void parse(LLVMModuleRef llvm_module, TokenData *token_data);

