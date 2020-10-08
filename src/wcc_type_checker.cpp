#include "./wcc_type_checker.hpp"

void Type_Checker::check_types_of_func_def(Func_Def *func_def)
{
    reporter.warn(func_def->offset,
                  "TODO: Type_Checker::check_types_of_func_def() is not implemented");
}

void Type_Checker::check_types_of_module(Module *module)
{
    auto top_level_def = module->top_level_defs;
    while (top_level_def) {
        check_types_of_func_def(&top_level_def->func_def);
        top_level_def = top_level_def->next;
    }
}
