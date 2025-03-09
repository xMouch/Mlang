#ifndef TYPER_H
#define TYPER_H

#include "ast.h"
#include "typer_table.h"


String get_text_from_token_to_token(Token* t1, Token* t2)
{
    if(t1->line != t2->line)
    {
        return t1->text;
    }
    
    if(t2->column < t1->column)
    {
        Token* tmp = t1;
        t1 = t2;
        t2 = tmp;
    }
    return substring(t1->line_text, t1->column-1-count_heading_whitespace_token_line(t1),
                     t2->column+t2->text.length - t1->column);
    
}

void token_combine(Token* t1, Token* t2)
{
    String str = get_text_from_token_to_token(t1, t2);
    if(!cmp_string(t1->text, str))
    {
        t1->text = str;
        t1->type = TOKEN_COMBINED;
        t1->column = s64_min(t1->column, t2->column);
    }
    
}

void typer_error(AST* ast, Node* node, c8* f_msg, ...)
{
    Token* t = node->info.token;
    ast->has_error = true;
    if(ast_node_has_error_in_tree(node))
    {
        node->info.has_error = true;
        return;
    }
    node->info.has_error = true;
    fprintf(stderr, "ERROR(%llu:%llu):\n", t->line, t->column);
    va_list valist;
    va_start(valist, f_msg);
    vfprintf(stderr, f_msg, valist);
    va_end(valist);
    fprintf(stderr, "\n%.*s", t->line_text);
    msi heading_whitespace = count_heading_whitespace_token_line(t);
    for(msi i = 1 ; i < t->column - heading_whitespace; ++i)
    {
        fprintf(stderr, " ");   
    }
    fprintf(stderr, "^\n");
}

void typer_warning(AST* ast, Node* node, c8* f_msg, ...)
{
    Token* t = node->info.token;
    fprintf(stderr, "WARNING(%llu:%llu):\n", t->line, t->column);
    va_list valist;
    va_start(valist, f_msg);
    vfprintf(stderr, f_msg, valist);
    va_end(valist);
    fprintf(stderr, "\n%.*s", t->line_text);
    msi heading_whitespace = count_heading_whitespace_token_line(t);
    for(msi i = 1 ; i < t->column - heading_whitespace; ++i)
    {
        fprintf(stderr, " ");   
    }
    fprintf(stderr, "^\n");
}

#define T_CON_VAL(constant) (data_type_is_floating_point((constant).type) ? (constant).f_value : (constant).s_value)
void typer_cast_const(Node* const_node, Type new_type, AST* ast)
{
    
    switch(new_type)
    {
        case TYPE_U8:{const_node->con.s_value =  s64_abs((u8)T_CON_VAL(const_node->con));}break;
        case TYPE_U16:{const_node->con.s_value = s64_abs((u16)T_CON_VAL(const_node->con));}break;
        case TYPE_U32:{const_node->con.s_value = s64_abs((u32)T_CON_VAL(const_node->con));}break;
        case TYPE_U64:{const_node->con.s_value = s64_abs((u64)T_CON_VAL(const_node->con));}break;
        case TYPE_MSI:{const_node->con.s_value = s64_abs((u64)T_CON_VAL(const_node->con));}break;
        case TYPE_S8:{const_node->con.s_value = (s8)T_CON_VAL(const_node->con);}break;
        case TYPE_S16:{const_node->con.s_value = (s16)T_CON_VAL(const_node->con);}break;
        case TYPE_S32:{const_node->con.s_value = (s32)T_CON_VAL(const_node->con);}break;
        case TYPE_S64:{const_node->con.s_value = (s64)T_CON_VAL(const_node->con);}break;
        case TYPE_F32:{const_node->con.f_value = (f32)T_CON_VAL(const_node->con);}break;
        case TYPE_F64:{const_node->con.f_value = (f64)T_CON_VAL(const_node->con);}break;
        case TYPE_B8:{const_node->con.s_value = s64_abs((u8)T_CON_VAL(const_node->con));}break;
        case TYPE_VOID:
        {
            typer_error(ast, const_node, "Cannot cast to void!");
            return;
        }
        default:
        {
            typer_error(ast, const_node, "Cannot cast a literal to non number type!");
            return;
        }
    }
    
    const_node->con.type = new_type;
}

void typer_const_expr(Node* node, AST* ast)
{
    IR_ASSERT(node->children && node->children[0]->type == N_CONSTANT);
    
    
    Expr_Op_Type optype = node->exp.type;
    
    if(optype == EX_U_CAST)
    {
        Type cast_type = node->exp.result_type;
        typer_cast_const(node->children[0], cast_type, ast);
        
        node->type = N_CONSTANT;
        node->con.type = cast_type;
        node->con.s_value = node->children[0]->con.s_value;
    }
    else
    {
        if(data_type_is_floating_point(node->children[0]->con.type) ||
           (ARR_LEN(node->children) == 2 && data_type_is_floating_point(node->children[1]->con.type)))
        {
            node->con.type = TYPE_F64;
            if((optype >= EX_B_MOD && optype <= EX_B_OR ) || optype == EX_U_BIN_INV)
            {
                typer_error(ast, node, "'%.*s' Operation is invalid for floating point operants!", node->info.token->text);
                return;
            }
        }
        else
        {
            node->con.type = TYPE_S64;  
        }
        
        node->type = N_CONSTANT;
    }
    
    switch(optype)
    {
        case EX_B_ADD:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)+T_CON_VAL(node->children[1]->con);
        }else {
            node->con.f_value = T_CON_VAL(node->children[0]->con)+T_CON_VAL(node->children[1]->con);
        }break;
        case EX_B_SUB:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)-T_CON_VAL(node->children[1]->con);
        }else {
            node->con.f_value = T_CON_VAL(node->children[0]->con)-T_CON_VAL(node->children[1]->con);
        }break;
        case EX_B_MUL:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)*T_CON_VAL(node->children[1]->con);
        }else {
            node->con.f_value = T_CON_VAL(node->children[0]->con)*T_CON_VAL(node->children[1]->con);
        }break;
        case EX_B_DIV:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)/T_CON_VAL(node->children[1]->con);
        }else {
            node->con.f_value = T_CON_VAL(node->children[0]->con)/T_CON_VAL(node->children[1]->con);
        }break;
        case EX_B_MOD:{node->con.s_value = node->children[0]->con.s_value % node->children[1]->con.s_value;}break;
        case EX_B_SHIFTL:{node->con.s_value = node->children[0]->con.s_value << node->children[1]->con.s_value;}break;
        case EX_B_SHIFTR:{node->con.s_value = node->children[0]->con.s_value >> node->children[1]->con.s_value;}break;
        case EX_B_AND:{node->con.s_value = node->children[0]->con.s_value & node->children[1]->con.s_value;}break;
        case EX_B_XOR:{node->con.s_value = node->children[0]->con.s_value ^ node->children[1]->con.s_value;}break;
        case EX_B_OR:{node->con.s_value = node->children[0]->con.s_value | node->children[1]->con.s_value;}break;
        case EX_C_OR:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)||T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)||T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_AND:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)&&T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)&&T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;;
        case EX_C_EQ:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)==T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)==T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_NEQ:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)!=T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)!=T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_LT:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)<T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)<T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_LTEQ:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)<=T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)<=T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_GT:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)>T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)>T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_C_GTEQ:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con)>=T_CON_VAL(node->children[1]->con);
        }else {
            node->con.s_value = T_CON_VAL(node->children[0]->con)>=T_CON_VAL(node->children[1]->con);
            node->con.type = TYPE_S64;
        }break;
        case EX_U_ADD:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = T_CON_VAL(node->children[0]->con);
        }else {
            node->con.f_value = T_CON_VAL(node->children[0]->con);
        }break;
        case EX_U_SUB:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = -T_CON_VAL(node->children[0]->con);
        }else {
            node->con.f_value = -T_CON_VAL(node->children[0]->con);
        }break;
        case EX_U_PREINC:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = ++node->children[0]->con.s_value;
        }else {
            node->con.f_value = ++node->children[0]->con.f_value;
        }break;
        case EX_U_PREDEC:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = --node->children[0]->con.s_value;
        }else {
            node->con.f_value = --node->children[0]->con.f_value;
        }break;
        case EX_U_LOGIC_INV:
        if(node->con.type == TYPE_S64){   
            node->con.s_value = !node->children[0]->con.s_value;
        }else {
            node->con.f_value = !node->children[0]->con.f_value;
        }break;
        case EX_U_BIN_INV:{ node->con.s_value = ~node->children[0]->con.s_value;}break;
        
        case EX_U_CAST:
        {
            //NOTE(Michael) Already handled in the beginning!
            break;
        }
        case EX_UNKNOWN:
        default:
        {     
            typer_error(ast, node, "Unknown Subexpression type in typing stage found!");
            break;   
        }
    }
    
    while(Node* child = (ARR_LEN(node->children) ? ARR_LAST(node->children) : nullptr))
    {
        token_combine(node->info.token, child->info.token);
        ast_remove_node(child, ast);
    }
}
#undef T_CON_VAL

void typer_expr(Node* node, AST* ast)
{
    switch(node->exp.type)
    {
        
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
        case EX_C_OR:
        case EX_C_AND:
        case EX_C_EQ:
        case EX_C_NEQ:
        case EX_C_LT:
        case EX_C_LTEQ:
        case EX_C_GT:
        case EX_C_GTEQ:
        {
            if(ARR_LEN(node->children) != 2)
            {
                typer_error(ast, node, "Binary operator has != 2 operants!: %llu", ARR_LEN(node->children));
                return;
            }
            
            u8 promote_side = 0;
            Node* left_node = node->children[0];
            Node* right_node = node->children[1];
            
            if(left_node->type == N_CONSTANT && right_node->type == N_CONSTANT)
            {
                typer_const_expr(node, ast);
                break;   
            }
            
            Type left_type;
            Type right_type;
            
            if(left_node->type == N_CONSTANT)
            {
                left_type = left_node->con.type; 
                
            }
            else if(left_node->type == N_VAR)
            {
                left_type = left_node->var->type;   
            }
            else
            {
                left_type = left_node->exp.result_type;   
            }
            
            if(right_node->type == N_CONSTANT)
            {
                right_type = right_node->con.type; 
            }
            else if(right_node->type == N_VAR)
            {
                right_type = right_node->var->type;   
            }
            else
            {
                right_type = right_node->exp.result_type;   
            }
            
            node->exp.result_type =
                typer_binary_operation_result(ast, node, node->exp.type, left_type, right_type, &promote_side);
            
            
            if(promote_side == 1)
            {
                Node* cast_node = ast_create_node(left_node->scope, left_node->info.token, ast);
                cast_node->type = N_EXPR;
                cast_node->exp.result_type = right_type;
                cast_node->exp.type = EX_U_CAST;
                cast_node->info.implicit =true;
                ast_insert_between(node, 0, cast_node, ast);
            }
            else if(promote_side == 2)
            {
                Node* cast_node = ast_create_node(right_node->scope, right_node->info.token, ast);
                cast_node->type = N_EXPR;
                cast_node->exp.result_type = left_type;
                cast_node->exp.type = EX_U_CAST;
                cast_node->info.implicit =true;
                ast_insert_between(node, 1, cast_node, ast);
            }     
            
            break;   
        }
        case EX_U_ADD:
        case EX_U_SUB:
        case EX_U_PREINC:
        case EX_U_PREDEC:
        case EX_U_LOGIC_INV:
        case EX_U_BIN_INV:
        {
            if(ARR_LEN(node->children) != 1)
            {
                typer_error(ast, node, "Unary operator has != 1 operants!: %llu", ARR_LEN(node->children));
                return;
            }
            
            Node* operant_node = node->children[0];
            
            if(operant_node->type == N_CONSTANT)
            {
                typer_const_expr(node, ast);
                break;
            }
            
            if(operant_node->type == N_VAR)
            {
                node->exp.result_type = operant_node->var->type;
                token_combine(node->info.token, operant_node->info.token);
            }
            else
            {
                node->exp.result_type = operant_node->exp.result_type;
            }
            
            break;
        }
        case EX_U_CAST:
        {
            if(ARR_LEN(node->children) != 1)
            {
                typer_error(ast, node, "Cast operator has != 1 operants!: %llu", ARR_LEN(node->children));
                return;
            }
            
            Node* operant_node = node->children[0];
            if(operant_node->type == N_CONSTANT)
            {
                typer_const_expr(node, ast);
                break;
            }
            
            //TODO(Michael): Check if cast is allowed
            
            break;
        }
        case EX_UNKNOWN:
        default:
        {     
            typer_error(ast, node, "Unknown Subexpression type in typing stage found!");
            break;   
        }
    }
}

void typer_depth_first(Node* node, AST* ast)
{
    if(node->children)
    {
        for(msi i = 0; i < ARR_LEN(node->children); ++i)
        {
            typer_depth_first(node->children[i], ast);
        }
    }
    
    switch(node->type)
    {
        case N_EXPR:
        {
            typer_expr(node, ast);
            break;   
        }
        case N_ASSIGN:
        {
            IR_ASSERT(ARR_LEN(node->children) == 2);
            Node* left = node->children[0];
            Node* right = node->children[1];
            IR_ASSERT(left->type == N_VAR || left->type == N_VAR_DECL);
            Type lt = left->var->type;
            Type rt = TYPE_UNKNOWN;
            switch(right->type)
            {
                case N_EXPR: { rt = right->exp.result_type; } break;
                case N_VAR: { rt = right->var->type; } break;
                case N_CONSTANT: 
                { 
                    rt = right->con.type;
                    if(left->var->type != rt)
                    {
                        typer_cast_const(right, left->var->type, ast);
                        rt = right->con.type;
                    }
                } break;
                default: { IR_INVALID_CASE; }
            }
            
            if(left->var->type != rt)
            {
                
                b8 lt_is_signed = data_type_is_signed(lt);
                b8 rt_is_signed = data_type_is_signed(rt);
                b8 lt_is_unsigned = false;
                b8 rt_is_unsigned = false;
                b8 lt_is_floating_point = false;
                b8 rt_is_floating_point = false;
                u8 lt_size = data_type_size(lt); 
                u8 rt_size = data_type_size(rt);
                
                if(!lt_is_signed)
                {
                    lt_is_unsigned = data_type_is_unsigned(lt);
                    if(!lt_is_unsigned)
                        lt_is_floating_point = data_type_is_floating_point(lt); 
                }
                
                
                if(!rt_is_signed)
                {
                    rt_is_unsigned = data_type_is_unsigned(rt);
                    if(!rt_is_unsigned)
                        rt_is_floating_point = data_type_is_floating_point(rt);
                }
                
                if((lt_is_signed || lt_is_unsigned) && rt_is_floating_point)
                {
                    typer_error(ast, node, 
                                "Trying to implicitly cast a floating point type to an integer type is not allowed!\n"
                                "Try casting it explictly with cast(%.*s)%.*s",
                                IR_EXP_STR(data_type_to_str(lt)),
                                IR_EXP_STR(node->children[1]->info.token->text));
                    break;
                }
                else if((rt_is_signed || rt_is_unsigned) && lt_is_floating_point)
                {
                    typer_error(ast, node, 
                                "Trying to implicitly cast an integer type to a floating point type is not allowed!\n"
                                "Try casting it explictly with cast(%.*s)%.*s",
                                IR_EXP_STR(data_type_to_str(lt)),
                                IR_EXP_STR(node->children[1]->info.token->text));
                    break;
                }
                else if((lt_is_signed && rt_is_signed) ||
                        (lt_is_unsigned && rt_is_unsigned) ||
                        (lt_is_floating_point && rt_is_floating_point))
                {
                    Node* cast_node = ast_create_node(node->scope, node->info.token, ast);
                    cast_node->type = N_EXPR;
                    cast_node->exp.result_type = lt;
                    cast_node->exp.type = EX_U_CAST;
                    cast_node->info.implicit =true;
                    ast_insert_between(node, 1, cast_node, ast);
                    break;
                }  
            }
            break; 
        }
        case N_RETURN:
        {
            
            break;   
        }
        default: break;
    };   
}

void typer(AST* ast)
{
    typer_depth_first(ast->root, ast);
    if(ast->has_error)
    {
        exit(EXIT_FAILURE);
    }
}

#endif //TYPER_H
