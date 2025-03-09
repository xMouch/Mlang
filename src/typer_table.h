#ifndef TYPER_TABLE_H
#define TYPER_TABLE_H

#include "ast.h"

void typer_error(AST* ast, Node* node, c8* f_msg, ...);
void typer_warning(AST* ast, Node* node, c8* f_msg, ...);
Type typer_binary_operation_result(AST* ast, Node* node, Expr_Op_Type op_type, Type o1, Type o2, u8* promote_side)
{   
    if(!(o1 != TYPE_UNKNOWN && o1 < TYPE_VOID && o2 != TYPE_UNKNOWN && o2 < TYPE_VOID))
    {
        typer_error(ast, node, "Type of operants in binary operation unknown!");
        return TYPE_UNKNOWN;
    }
    if(promote_side) 
        *promote_side = 0;
    if(o1 == o2)
    {
        if(op_type >= EX_C_OR && op_type <= EX_C_GTEQ)
        {
            return TYPE_B8;   
        }
        else
        {
            return o1;
        }
    }
    
    b8 o1_is_signed = data_type_is_signed(o1);
    b8 o2_is_signed = data_type_is_signed(o2);
    b8 o1_is_unsigned = false;
    b8 o2_is_unsigned = false;
    b8 o1_is_floating_point = false;
    b8 o2_is_floating_point = false;
    u8 o1_size = data_type_size(o1); 
    u8 o2_size = data_type_size(o2);
    
    if(!o1_is_signed)
    {
        o1_is_unsigned = data_type_is_unsigned(o1);
        if(!o1_is_unsigned)
            o1_is_floating_point = data_type_is_floating_point(o1); 
    }
        
    
    if(!o2_is_signed)
    {
        o2_is_unsigned = data_type_is_unsigned(o2);
        if(!o2_is_unsigned)
            o2_is_floating_point = data_type_is_floating_point(o2);
    }
    
    if((o1_is_signed || o1_is_unsigned) && o2_is_floating_point)
    {
        typer_error(ast, node, 
                    "Trying to implicitly cast an integer type to a floating point type is not allowed!\n"
                    "Try casting it explictly with cast(%.*s)%.*s",
                    IR_EXP_STR(data_type_to_str(o2)),
                    IR_EXP_STR(node->children[0]->info.token->text));
        return TYPE_UNKNOWN;
            
    }
    else if((o2_is_signed || o2_is_unsigned) && o1_is_floating_point)
    {
        typer_error(ast, node, 
                    "Trying to implicitly cast an integer type to a floating point type is not allowed!\n"
                    "Try casting it explictly with cast(%.*s)%.*s",
                    IR_EXP_STR(data_type_to_str(o1)),
                    IR_EXP_STR(node->children[1]->info.token->text));
        return TYPE_UNKNOWN;
    }
    
    switch(op_type)
    {
        default:{ IR_ASSERT(false && "Non binary operation given!"); return TYPE_UNKNOWN;}
        case EX_B_ADD:
        case EX_B_SUB:
        case EX_B_MUL:
        case EX_B_DIV:
        case EX_B_MOD:
        case EX_B_SHIFTL:
        case EX_B_SHIFTR:
        case EX_B_AND:
        case EX_B_XOR:
        case EX_B_OR:
        {
            if((o1_is_signed && o2_is_signed) ||
               (o1_is_unsigned && o2_is_unsigned) ||
               (o1_is_floating_point && o2_is_floating_point))
            {
                if(o1_size > o2_size)
                {
                    if(promote_side) 
                        *promote_side = 2;
                    return o1;
                }
                else
                {
                    if(promote_side) 
                        *promote_side = 1;
                    return o2;
                }
            }
            else
            {
                if(o1_is_signed && o2_is_unsigned)
                {
                    if(o1_size > o2_size)
                    {
                        if(promote_side) 
                            *promote_side = 2;
                        return o1;
                    }
                    else
                    {
                        if(promote_side) 
                            *promote_side = 2;
                        typer_warning(ast, node, "Signed type is smaller in size than the unsigned type in operation!\n"
                                      "cast '%.*s' to '%.*s' to supress this warning.",
                                      node->children[1]->info.token->text, data_type_to_str(o1));
                        return o1;
                    }
                }
                else if(o1_is_unsigned && o2_is_signed)
                {
                    if(o2_size > o1_size)
                    {
                        if(promote_side) 
                            *promote_side = 1;
                        return o2;
                    }
                    else
                    {
                        if(promote_side) 
                            *promote_side = 1;
                        typer_warning(ast, node, "Signed type is smaller in size than the unsigned type in operation!\n"
                                      "cast '%.*s' to '%.*s' to supress this warning.",
                                      node->children[0]->info.token->text, data_type_to_str(o2));
                        return o2;
                    }
                }
                else
                {
                    IR_INVALID_CASE;   
                }
            }
            break;
        }
        case EX_C_OR:
        case EX_C_AND:
        case EX_C_EQ:
        case EX_C_NEQ:
        case EX_C_LT:
        case EX_C_LTEQ:
        case EX_C_GTEQ:
        {
            if((o1_is_signed && o2_is_signed) ||
               (o1_is_unsigned && o2_is_unsigned) ||
               (o1_is_floating_point && o2_is_floating_point))
            {
                if(o1_size > o2_size)
                {
                    if(promote_side) 
                        *promote_side = 2;
                    return TYPE_B8;
                }
                else
                {
                    if(promote_side) 
                        *promote_side = 1;
                    return TYPE_B8;
                }
            }
            else
            {
                if(o1_is_signed && o2_is_unsigned)
                {
                    if(o1_size > o2_size)
                    {
                        if(promote_side) 
                            *promote_side = 2;
                        return TYPE_B8;
                    }
                    else
                    {
                        if(promote_side) 
                            *promote_side = 2;
                        typer_warning(ast, node, "Signed type is smaller in size than the unsigned type in operation!\n"
                                      "cast '%.*s' to '%.*s' to supress this warning.",
                                      node->children[1]->info.token->text, data_type_to_str(o1));
                        return TYPE_B8;
                    }
                }
                else if(o1_is_unsigned && o2_is_signed)
                {
                    if(o2_size > o1_size)
                    {
                        if(promote_side) 
                            *promote_side = 1;
                        return TYPE_B8;
                    }
                    else
                    {
                        if(promote_side) 
                            *promote_side = 1;
                        typer_warning(ast, node, "Signed type is smaller in size than the unsigned type in operation!\n"
                                      "cast '%.*s' to '%.*s' to supress this warning.",
                                      node->children[0]->info.token->text, data_type_to_str(o2));
                        return TYPE_B8;
                    }
                }
                else
                {
                    IR_INVALID_CASE;   
                }
            }
            break;
        }
    }
    
    IR_INVALID_CASE;
    return TYPE_UNKNOWN;
}


#endif //TYPER_TABLE_H
