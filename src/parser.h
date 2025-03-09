#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdarg.h>

#include "ast.h"

/*
EBNF

program = block ;

block = 
    {
        function |
        global_var_dec
    } ;

var_decl = basic_type, ident, ";" | ("=", expr, ";") ;

assign = ident, ("=" | "+=" | "^=" etc.), expr ";" ;

expr = c-like without assign and ternary.



function = basic_type, ident, "(", (parameter, {",", parameter}), ")", 
            "{",
            statement,
            "}";

parameter = basic_type, ident;

if_else = "if", expr, statement ("else", statement );

statement = var_decl | assign | return | "{", {statement} "}" ;

return = "return", expr, ";";


*/

struct Parser
{
    Token* t;
    Token* tokens;
    msi t_index;
    msi last_error_line;
    Scope* cur_scope;
    AST ast;
};

Scope* parser_ascend_scope(Parser* p)
{
    IR_NOT_NULL(p);
    IR_NOT_NULL(p->cur_scope->parent);
    if(p->cur_scope->parent)
    {
        p->cur_scope = p->cur_scope->parent;   
    }
    return p->cur_scope;
}

Scope* parser_create_scope_and_descend(Parser* p)
{
    IR_NOT_NULL(p);
    p->cur_scope = ast_create_scope(p->cur_scope, &p->ast);
    return p->cur_scope;
}

Token* next_token(Parser* p)
{
    Token* result = p->t;
    if(p->t->type != TOKEN_EOF)
    {
        ++p->t_index;
    }
    p->t = &p->tokens[p->t_index];
    return result;
}

Token* prev_token(Parser* p)
{
    if(p->t_index > 0)
    {
        --p->t_index;
    }
    p->t = &p->tokens[p->t_index];
    return p->t;
}

void parser_error( Parser* p, Token* t, c8* f_msg, ...)
{
    if(p->last_error_line ==  t->line)
    {
        exit(EXIT_FAILURE);   
    }
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
    p->last_error_line = t->line;
}


Token* peek_token(Parser* p)
{
    return &p->tokens[p->t_index];
}

Token* peek_token(Parser* p, msi i)
{
    msi len = ARR_LEN(p->tokens);
    if(len > p->t_index + i)
    {
        return &p->tokens[p->t_index + i];   
    }
    return &p->tokens[len -1];
}


Token* jmp_next_line(Parser* p)
{
    Token* t = peek_token(p);
    while(next_token(p)->line== t->line)
    {
        
    }
    return peek_token(p);;
}

Parser init_parser(Token* tokens, Heap_Allocator* heap)
{
    IR_NOT_NULL(tokens);
    Parser p = {};
    p.tokens = tokens;
    p.t = &tokens[0];
    p.ast.heap = heap;
    BA_INIT(p.ast.nodes_ba, 2, heap);
    BA_INIT(p.ast.functions_ba, 2, heap);
    BA_INIT(p.ast.scopes_ba, 2, heap);
    BA_INIT(p.ast.variables_ba, 2, heap);
    p.ast.global_scope = ast_create_scope(nullptr, &p.ast);
    p.cur_scope = p.ast.global_scope;
    
    return p;
}


b8 peek_pattern(Parser* p, int num_args,  ...)
{
    b8 result = true;
    va_list args;
    va_start(args, num_args);
    for(msi i = 0; i < num_args; ++i)
    {
        Token_Type next = va_arg(args, Token_Type);
        if(peek_token(p, i)->type != next)
        {
            result = false;
            break;
        }
    }
    va_end(args);
    
    return result;
}

String token_text(Token* t)
{
    String result = IR_CONSTZ("NULL");
    if(t)
    {
        result = t->text;
    }
    
    return result;
}

Type token_data_type(Token* t)
{
    Type result = TYPE_UNKNOWN;
    if(t)
    {
        result = t->data_type;
    }
    
    return result;
}

Token* accept(Token_Type t, Parser* p)
{
    Token* result = nullptr;
    if(t == p->t->type)
    {
        result = p->t;
        next_token(p);
    }
    return result;;
}

inline
Token* accept(c8 t, Parser* p)
{
    return accept((Token_Type)t, p);
}
inline
Token* expect(Token_Type t, Parser* p)
{
    Token* result = nullptr;
    result = accept(t, p);
    if(!result)
    {

        parser_error(p, peek_token(p),
              "Unexpected Token expected '%.*s' got '%.*s'",
              token_type_to_str(t).length, token_type_to_str(t).data,
              peek_token(p)->text.length, peek_token(p)->text.data);
    }
    
    return result;
}



inline
Token* expect(c8 t, Parser* p)
{
    return expect((Token_Type)t, p);
}

Node* parse_term(Parser* p);
Node* parse_expr(Parser* p, s64 cur_priority = -999)
{
    Node* result = nullptr;
    Node* left_expr = parse_term(p);
    
    while(left_expr)
    {
        s64 op_priority = -9999;
        Expr exp = {};        
        Token* t = peek_token(p);
        switch(t->type)
        {
            case '*': exp.type = EX_B_MUL;                  op_priority = 100; break;
            case '/': exp.type = EX_B_DIV;                  op_priority = 100; break;
            case '%': exp.type = EX_B_MOD;                  op_priority = 100; break;
            
            case '+': exp.type = EX_B_ADD;                  op_priority = 90;  break;   
            case '-': exp.type = EX_B_SUB;                  op_priority = 90;  break;  
            
            case TOKEN_SHIFT_L: exp.type = EX_B_SHIFTL;     op_priority = 80;  break;   
            case TOKEN_SHIFT_R: exp.type = EX_B_SHIFTR;     op_priority = 80;  break; 
            
            case '<': exp.type = EX_C_LT;                   op_priority = 70;  break;   
            case '>': exp.type = EX_C_GT;                   op_priority = 70;  break;   
            case TOKEN_LEQ: exp.type = EX_C_LTEQ;           op_priority = 70;  break;   
            case TOKEN_GEQ: exp.type = EX_C_GTEQ;           op_priority = 70;  break;
            
            case TOKEN_D_EQ: exp.type = EX_C_EQ;            op_priority = 60;  break;   
            case TOKEN_NOTEQ: exp.type = EX_C_NEQ;          op_priority = 60;  break;
            
            case '&': exp.type = EX_B_AND;                  op_priority = 50;  break; 
            case '^': exp.type = EX_B_XOR;                  op_priority = 40;  break;
            case '|': exp.type = EX_B_OR;                   op_priority = 30;  break;
            case TOKEN_AND: exp.type = EX_C_AND;            op_priority = 20;  break;
            case TOKEN_OR: exp.type = EX_C_OR;              op_priority = 10;  break;   
            default: break;    
        }
        
        if(exp.type != EX_UNKNOWN)
        {
            if(op_priority > cur_priority)
            {
                next_token(p);
                result = ast_create_node(p->cur_scope, t, &p->ast);
                result->type=N_EXPR;
                result->exp = exp;
                
                Node* right_expr = nullptr;
                right_expr = parse_expr(p, op_priority);
                
                ast_node_add_child(result, left_expr, &p->ast);
                ast_node_add_child(result, right_expr, &p->ast);
                
                if(!right_expr)
                {
                    parser_error(p, t, "Expected expression after '%.*s'", token_text(t));
                }
                
                left_expr = result;
            }
            else
            {
                result = left_expr;
                break;
            }
        }
        else
        {
            result = left_expr;
            break;
        }   
    }
    
    
    return result;
}

Node* parse_term(Parser* p)
{
    Node* result = nullptr;
    Token* t = peek_token(p);
    
    Expr_Op_Type unary_expr_type = EX_UNKNOWN;
    
    switch(t->type)
    {
        case '(':
        {
            next_token(p);
            result = parse_expr(p);
            if(!expect(')', p))
            {
                parser_error(p, t, "Missing ')' for subexpression!");   
            }
            break;
        }
        case '+':
        {
            next_token(p);
            result = parse_term(p);
            if(!result)
            {
                parser_error(p, t, "Expected term after Unary '%.*s'", t->text); 
            }
            break;
        }
        case '-':
        {
            next_token(p);
            unary_expr_type = EX_U_SUB;
            break;   
        }
        case TOKEN_D_PLUS:
        {
            next_token(p);
            unary_expr_type = EX_U_PREINC;
            break;   
        }
        case TOKEN_D_MINUS:
        {
            next_token(p);
            unary_expr_type = EX_U_PREDEC;
            break;   
        }
        case '!':
        {
            next_token(p);
            unary_expr_type = EX_U_LOGIC_INV;
            break;   
        }
        case '~':
        {
            next_token(p);
            unary_expr_type = EX_U_BIN_INV;
            break;   
        }
        case TOKEN_CAST:
        {
            next_token(p);
            expect('(', p);
            result = ast_create_node(p->cur_scope, t, &p->ast);
            result->type = N_EXPR;
            result->exp.type = EX_U_CAST;
            result->exp.result_type = token_data_type(expect(TOKEN_BASIC_TYPE, p));
            expect(')', p);
            
            Node* term = parse_term(p);
            if(!term)
            {
                parser_error(p, t, "Expected term or subexpression after cast!"); 
            }
            ast_node_add_child(result, term, &p->ast); 
            break;
        }
        case TOKEN_NUM:
        {
            //TODO(Michael) BETTER CONSTANTS!
            result = ast_create_node(p->cur_scope, t, &p->ast);
            result->type = N_CONSTANT;
            Token* num_tok = next_token(p);
            u8* endptr = num_tok->text.data+num_tok->text.length;
            if(search_string_first_occurrence(num_tok->text, '.').length > 0)
            {
                //FLOAT
                result->con.type=TYPE_F64;
                result->con.f_value = strtod(num_tok->text.data, &endptr);
                if(endptr == num_tok->text.data)
                {
                    parser_error(p, num_tok, "Failed to convert floating point constant!");
                }
            }
            else
            {
                //INTEGER
                result->con.type=TYPE_S64;
                result->con.s_value = strtoll(num_tok->text.data, &endptr, 10);
                if(endptr == num_tok->text.data)
                {
                    parser_error(p, num_tok, "Failed to convert integer constant!");
                }
            }
            break;
        }
        case TOKEN_ID:
        {
            Token* var_tok = next_token(p);
            result = ast_create_node(p->cur_scope, var_tok, &p->ast);
            result->type = N_VAR;
            result->var = ast_search_var_from_scope_and_name(var_tok->text, p->cur_scope);
            
            if(!result->var)
            {
                parser_error(p, var_tok, "Use of undeclared identifier '%.*s'!", var_tok->text);
            }
            
            break;
        }
        default: break;
    }
    
    if(unary_expr_type != EX_UNKNOWN)
    {
        result = ast_create_node(p->cur_scope, t, &p->ast);
        result->type = N_EXPR;
        result->exp.type = unary_expr_type;
        Node* term = parse_term(p);
        if(!term)
        {
            parser_error(p, t, "Expected term after Unary '%.*s'", t->text); 
        }
        ast_node_add_child(result, term, &p->ast);   
    }
    
    return result;
}



Node* parse_var_decl(Parser* p)
{
    Node* result = nullptr;
    
    if(peek_pattern(p, 2, TOKEN_BASIC_TYPE, TOKEN_ID))
    {
        Type var_type = token_data_type(expect(TOKEN_BASIC_TYPE, p));
        String var_name = token_text(expect(TOKEN_ID, p));
        
        result = ast_create_node(p->cur_scope, peek_token(p, -1), &p->ast);
        result->type = N_VAR_DECL;
        
        Node* var_node = result;
        
        if(accept('=', p))
        {
            Node* assign_expr = parse_expr(p);
            if(!assign_expr)
            {
                parser_error(p, peek_token(p, -1), "Expected expression after '=' in variable declaration!");   
            }
            else
            {
                Node* decl = result;
                result = ast_create_node(p->cur_scope, result->info.token, &p->ast);
                result->type = N_ASSIGN;
                ast_node_add_child(result, decl, &p->ast);
                ast_node_add_child(result, assign_expr, &p->ast);
            }
            expect(';', p);
        }
        else if(!accept(';', p))
        {
            parser_error(p, peek_token(p), "Can only use simple assign '=' or ';' when declaring a variable!");
        }
        
        Variable* var = ast_create_var(p->cur_scope, peek_token(p, 1), &p->ast);
        var->type = var_type;
        var->name = var_name;
        var_node->var = var;
        
    }
    
    return result;
}

Node* parse_assign(Parser* p)
{
    Node* result = nullptr;
    if(peek_token(p)->type == TOKEN_ID)
    {
        Token_Type token_type = peek_token(p, 1)->type;
        Expr_Op_Type op_type = EX_UNKNOWN;
        switch(token_type)
        {
            case '=':
            {
                break;       
            }
            case TOKEN_PLUS_EQ:// +=
            {
                op_type = EX_B_ADD;
                break;
            }
            case TOKEN_MINUS_EQ:// -=
            {
                op_type = EX_B_SUB;
                break;
            }
            case TOKEN_MUL_EQ:// *=
            {
                op_type = EX_B_MUL;
                break;
            }
            case TOKEN_DIV_EQ:// /=
            {
                op_type = EX_B_DIV;
                break;
            }
            case TOKEN_MOD_EQ:// %=
            {
                op_type = EX_B_MOD;
                break;
            }
            case TOKEN_SHIFT_L_EQ:// <<=
            {
                op_type = EX_B_SHIFTL;
                break;
            }
            case TOKEN_SHIFT_R_EQ:// >>=
            {
                op_type = EX_B_SHIFTR;
                break;
            }
            case TOKEN_AND_EQ:// &=
            {
                op_type = EX_B_AND;
                break;
            }
            case TOKEN_XOR_EQ:// ^=
            {
                op_type = EX_B_XOR;
                break;
            }
            case TOKEN_OR_EQ:// |=
            {
                op_type = EX_B_OR;
                break;
            }
            default:
            {
                return result;  
            }
        }
        
        Token* id_tok = expect(TOKEN_ID, p);
        Node* var_node = ast_create_node(p->cur_scope, id_tok, &p->ast);
        var_node->type = N_VAR;
        var_node->var = ast_search_var_from_scope_and_name(id_tok->text, p->cur_scope);
        
        if(!var_node->var)
        {
            parser_error(p, id_tok, "Trying to assign to undeclared identifier '%.*s'!", id_tok->text);
        }
        
        Token* assign_tok = next_token(p);
        
        Node* expr_node = parse_expr(p);
        
        if(!expr_node)
        {
            parser_error(p, assign_tok, "Expected expression after assign '%.*s'!", assign_tok->text);
        }
        
        if(op_type != EX_UNKNOWN)
        {
            Node* new_expr = ast_create_node(p->cur_scope, assign_tok, &p->ast);
            new_expr->type = N_EXPR;
            new_expr->exp.type = op_type;
            
            ast_node_add_child(new_expr, var_node, &p->ast);
            ast_node_add_child(new_expr, expr_node, &p->ast);
            
            expr_node = new_expr;
            
            Variable* var = var_node->var;
            var_node = ast_create_node(p->cur_scope, id_tok, &p->ast);
            var_node->type = N_VAR;
            var_node->var = var;
        }
        
        result = ast_create_node(p->cur_scope, assign_tok, &p->ast);
        result->type = N_ASSIGN;
        
        
        ast_node_add_child(result, var_node, &p->ast);
        ast_node_add_child(result, expr_node, &p->ast);
        
        
        expect(';', p);
        
    }
    
    
    return result;
}

Node* parse_return(Parser* p)
{
    Node* result = nullptr;
    if(accept(TOKEN_RETURN, p))
    {
        
        result = ast_create_node(p->cur_scope, peek_token(p, -1), &p->ast);
        result->type = N_RETURN;
        
        Node* e = parse_expr(p);
        ast_node_add_child(result, e, &p->ast);
        expect(';', p);
    }
    return result;
}

Node* parse_statement(Parser* p);
Node* parse_statement_seq(Parser* p);
Node* parse_if_else(Parser* p)
{
    Node* result = nullptr;
    Token* if_tok = accept(TOKEN_IF, p);
    if(if_tok)
    {
        result = ast_create_node(p->cur_scope, if_tok, &p->ast);
        result->type = N_IF;
        Node* expr_node = parse_expr(p);
        
        if(!expr_node)
        {
            parser_error(p, if_tok, "Expected expression for if statement!");   
        }
        
        ast_node_add_child(result, expr_node, &p->ast);
        Node* stmnt = parse_statement(p);
        ast_node_add_child(result, stmnt, &p->ast);        
        
        Token* else_tok = accept(TOKEN_ELSE, p);
        
        if(else_tok)
        {
            
            Node* else_node = ast_create_node(p->cur_scope, else_tok, &p->ast);
            else_node->type = N_ELSE;
            
            Node* stmnt_node = parse_statement(p); 
            ast_node_add_child(else_node, stmnt_node, &p->ast); 
 
            ast_node_add_child(result, else_node, &p->ast);
        }
        
    }
    
    return result;
}

Node* parse_statement(Parser* p)
{
    Node* result = nullptr;
    if((result = parse_return(p)))
    {}
    else if((result = parse_var_decl(p)))
    {}
    else if((result = parse_assign(p)))
    {}   
    else if((result = parse_if_else(p)))
    {} 
    else if(accept('{', p))
    {
        parser_create_scope_and_descend(p);
        result = parse_statement_seq(p);
        parser_ascend_scope(p);
        expect('}', p);
    }
    
    
    return result;
}

Node* parse_statement_seq(Parser* p)
{
    Node* result = nullptr;
    
    Node* stmnt = parse_statement(p);
    
    if(stmnt)
    {
        result = ast_create_node(p->cur_scope, stmnt->info.token, &p->ast);
        result->type = N_STATEMENT_SEQ;
        
        ast_node_add_child(result, stmnt, &p->ast);
        
        while((stmnt = parse_statement(p)))
        {
            ast_node_add_child(result, stmnt, &p->ast);    
        }
    }
    
    return result;
}

Node* parse_function(Parser* p)
{
    Type return_type = token_data_type(expect(TOKEN_BASIC_TYPE, p));
    String id = token_text(expect(TOKEN_ID, p));
    expect('(', p);
    
    Function* fun = ast_create_fun(peek_token(p, -2), &p->ast);
    fun->scope = ast_create_scope(p->cur_scope, &p->ast);
    fun->name = id;
    fun->return_type = return_type;
    
    Node* result = ast_create_node(p->cur_scope, fun->token, &p->ast);
    result->type = N_FUNCTION;
    result->fun = fun;
    
    p->cur_scope = fun->scope;
    
    
    if(!accept(')', p))
    {
        Type var_type = token_data_type(expect(TOKEN_BASIC_TYPE, p));
        id = token_text(expect(TOKEN_ID, p));
        
        Variable* var = ast_create_var(p->cur_scope, peek_token(p, -1), &p->ast);
        var->type = var_type;
        var->name = id;
        ARR_PUSH(fun->params, var);
        
        while(accept(',', p))
        {
            var_type = token_data_type(expect(TOKEN_BASIC_TYPE, p));
            id = token_text(expect(TOKEN_ID, p));
            var = ast_create_var(p->cur_scope, peek_token(p, -1), &p->ast);
            var->type = var_type;
            var->name = id;
            ARR_PUSH(fun->params, var);
        }
        expect(')', p);
    }
    expect('{', p);
    
    ast_node_add_child(result, parse_statement_seq(p), &p->ast);
    
    expect('}', p);
    
    parser_ascend_scope(p);
    
    return result;
}

void block(Parser* p)
{
    
    Node* root = ast_create_node(p->cur_scope, p->tokens, &p->ast);
    root->type= N_PROGRAM;
    p->ast.root = root;
    b8 found_something = true;
    while(found_something)
    {
        found_something = false;
        if(peek_pattern(p, 3, TOKEN_BASIC_TYPE, TOKEN_ID, (Token_Type)'('))
        {
            Node* fun = parse_function(p);
            if(fun)
            {
                ast_node_add_child(p->ast.root, fun, &p->ast);
                found_something = true;
            }
        }
        else 
        {
            Node* global_var = parse_var_decl(p);
            if(global_var)
            {
                ast_node_add_child(p->ast.root, global_var, &p->ast); 
                found_something = true;
            }
            
        }
        if(peek_pattern(p, 1, TOKEN_EOF))
        {
            break;
        }
    }
}

AST parse(Token* tokens, Heap_Allocator* heap)
{
    Parser p = init_parser(tokens, heap);
    
    block(&p);
    
    if(p.last_error_line != 0)
    {
        exit(EXIT_FAILURE);   
    }
    
    return p.ast;
}


#endif //PARSER_H
