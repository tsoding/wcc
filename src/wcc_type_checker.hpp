#ifndef WCC_TYPE_CHECKER_HPP_
#define WCC_TYPE_CHECKER_HPP_

#include "./wcc_memory.hpp"
#include "./wcc_parser.hpp"

struct Type_Checker
{
    Linear_Memory *memory;
    Reporter reporter;

    void check_types_of_func_def(Func_Def *func_def);
    void check_types_of_module(Module *module);
};

#endif  // WCC_TYPE_CHECKER_HPP_
