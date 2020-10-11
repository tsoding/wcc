#ifndef WCC_TYPE_CHECKER_HPP_
#define WCC_TYPE_CHECKER_HPP_

#include "./wcc_memory.hpp"
#include "./wcc_parser.hpp"
#include "./wcc_types.hpp"

struct Type_Checker
{
    struct Scope
    {
        Args_List *args_list;
        Scope *next;
    };


    Linear_Memory *memory;
    Reporter reporter;
    Scope *scope;

    Type type_of_name(size_t offset, String_View name) const;
    void push_scope(Args_List *args_list = nullptr);
    void push_var_def(Var_Def var_def);
    void pop_scope();

    Expression *cast_expression_to(Expression *expression, Type type);
    Expression *try_implicitly_cast_expression_to(Expression *expression, Type cast_type);
    Type check_types(size_t offset, Type expected_type, Type actual_type);

    Type check_types_of_local_var_def(Local_Var_Def *local_var_def);
    Type check_types_of_while(While *hwile);
    Type check_types_of_if(If *iph);
    Type check_types_of_assignment(Assignment *assignment);
    Type check_types_of_subtract_assignment(Subtract_Assignment *subtract_assignment);
    Type check_types_of_expression(Expression *expression);

    Type check_types_of_statement(Statement *statement);
    Type check_types_of_block(Block *block);
    void check_types_of_func_def(Func_Def *func_def);
    void check_types_of_module(Module *module);
};

#endif  // WCC_TYPE_CHECKER_HPP_
