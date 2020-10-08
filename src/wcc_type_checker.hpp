#ifndef WCC_TYPE_CHECKER_HPP_
#define WCC_TYPE_CHECKER_HPP_

#include "./wcc_memory.hpp"
#include "./wcc_parser.hpp"

struct Type_Checker
{
    Linear_Memory *memory;
    String_View input;
    String_View filename;

    template <typename... Args>
    void inform(size_t offset, Args... args)
    {
        String_View input0 = input;
        for (size_t line_number = 1; input0.count > 0; ++line_number) {
            String_View line = input0.chop_by_delim('\n');

            if (offset <= line.count) {
                println(stderr, filename, ":", line_number, ":", offset + 1, ": ", args...);
                break;
            }

            offset -= line.count + 1;
        }
    }

    template <typename... Args>
    void warn(size_t offset, Args... args)
    {
        inform(offset, "warning: ", args...);
    }

    template <typename... Args>
    void fail(size_t offset, Args... args)
    {
        inform(offset, "error: ", args...);
        exit(1);
    }

    void check_types_of_func_def(Func_Def *func_def);
    void check_types_of_module(Module *module);
};

#endif  // WCC_TYPE_CHECKER_HPP_
