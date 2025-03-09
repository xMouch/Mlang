#ifndef AST_H
#define AST_H


enum Node_Type
{
    N_UNKNOWN=0,
    N_PROGRAM,
    N_VAR_DECL,
    N_ASSIGN,
    N_FUNCTION,
    N_STATEMENT_SEQ,
    N_RETURN,
    N_EXPR,
    N_VAR,
    N_IF,
    N_ELSE,
    N_CONSTANT,
    N_COUNT,
};

enum Expr_Op_Type
{
    EX_UNKNOWN = 0,    
    EX_B_ADD,
    EX_B_SUB,
    EX_B_MUL,
    EX_B_DIV,
    EX_B_MOD,
    EX_B_SHIFTL,
    EX_B_SHIFTR,
    EX_B_AND,
    EX_B_XOR,
    EX_B_OR,
    
    EX_C_OR,
    EX_C_AND,
    EX_C_EQ,
    EX_C_NEQ,
    EX_C_LT, //<
    EX_C_LTEQ, //<=
    EX_C_GT, //>
    EX_C_GTEQ, //>=
    
    EX_U_ADD,
    EX_U_SUB,
    EX_U_PREINC,
    EX_U_PREDEC,
    EX_U_LOGIC_INV,
    EX_U_BIN_INV,
    EX_U_CAST,
    EX_COUNT,
};

struct Constant
{
    Token* token;
    Type type;
    union
    {
        //     u64 u_value;
        s64 s_value;
        f64 f_value;
    };
};

struct Scope;
struct Variable
{
    Token* token;
    Type type;
    String name;
    Scope* scope;
};

struct Scope
{
    Scope* parent;
    Variable** variables;
};

struct Expr
{
    Expr_Op_Type type;
    Type result_type;
};

struct Function
{
    Scope* scope;
    Variable** params;
    Type return_type;
    String name;
    Token* token;
};


struct Node;
#if 1
//NOTE(Michael)This is here to help the debugger show the children properly...
struct Node_List
{
    Node* n0;
    Node* n1;
    Node* n2;
    Node* n3;
    Node* n4;
};

#endif

struct Node_Info
{
    Token* token;
    b8 has_error;
    Node* next_in_free_list;
    b8 implicit;
};

struct Node
{
    Node_Type type;
    Node* parent;
    union
    {
        Node** children;
        Node_List* ch;
    };
    Scope* scope;
    Node_Info info;
    union
    {
        Function* fun;
        Variable* var;
        String id_placeholder;
        Constant con;
        Expr exp;
    };
};



struct AST
{
    Heap_Allocator* heap;
    Node* nodes_ba;
    Function* functions_ba;
    Variable* variables_ba;
    Scope* scopes_ba;
    Node* root;
    Scope* global_scope;
    Node* node_free_list;
    b8 has_error;
};

inline
Node* ast_root(AST* ast)
{
    return BA_GET(ast->nodes_ba, 0);  
};

void ast_print_node(Node* node)
{
    IR_NOT_NULL(node);
    
    if(node->info.implicit)
    {
        printf("\033[36m");   
    }
    
    if(node->info.has_error)
    {
        printf("\033[31m");   
    }
    
    switch(node->type)
    {
        case N_FUNCTION:
        {
            printf("%.*s", node->fun->name);
            break;   
        }
        case N_VAR:
        {
            printf("%.*s", node->var->name);
            if(node->var->type != TYPE_UNKNOWN)
            {
                printf(" %.*s", data_type_to_str(node->var->type));   
            }
            break;   
        }
        case N_EXPR:
        {
            switch(node->exp.type)
            {
                case EX_B_ADD:       printf("+"); break;
                case EX_B_SUB:       printf("-"); break;
                case EX_B_MUL:       printf("*"); break;
                case EX_B_DIV:       printf("/"); break;
                case EX_B_MOD:       printf("%%"); break;
                case EX_B_SHIFTL:    printf("<<"); break;
                case EX_B_SHIFTR:    printf(">>"); break;
                case EX_B_AND:       printf("&"); break;
                case EX_B_XOR:       printf("^"); break;
                case EX_B_OR:        printf("|"); break;
                case EX_C_OR:        printf("||"); break;
                case EX_C_AND:       printf("&&"); break;
                case EX_C_EQ:        printf("=="); break;
                case EX_C_NEQ:       printf("!="); break;
                case EX_C_LT:        printf("<"); break; //<
                case EX_C_LTEQ:      printf("<="); break; //<=
                case EX_C_GT:        printf(">"); break; //>
                case EX_C_GTEQ:      printf(">="); break; //>=
                case EX_U_ADD:       printf("+()"); break;
                case EX_U_SUB:       printf("-()"); break;
                case EX_U_PREINC:    printf("++()"); break;
                case EX_U_PREDEC:    printf("--()"); break;
                case EX_U_LOGIC_INV: printf("!()"); break;
                case EX_U_BIN_INV:   printf("~()"); break;
                case EX_U_CAST:      printf("cast"); break;
                default: break;
            }
            
            
            
            if(node->exp.result_type != TYPE_UNKNOWN)
            {
                printf(" %.*s", data_type_to_str(node->exp.result_type));   
            }
            break;
        }
        case N_STATEMENT_SEQ:
        {
            printf("┐{}");
            break;   
        }
        case N_RETURN:
        {
            printf("return");
            break;   
        }
        case N_PROGRAM:
        {
            printf("Program");
            break;   
        }
        case N_ASSIGN:
        {
            printf("=");
            break;   
        }
        case N_IF:
        {
            printf("if");
            break;   
        }
        case N_ELSE:
        {
            printf("else");
            break;   
        }
        case N_VAR_DECL:
        {
            printf("%.*s %.*s", data_type_to_str(node->var->type), node->var->name);
            break;   
        }
        case N_CONSTANT:
        {
            switch(node->con.type)
            {
                case TYPE_U8:
                case TYPE_U16:
                case TYPE_U32:
                case TYPE_U64:
                case TYPE_S8:
                case TYPE_S16:
                case TYPE_S32:
                case TYPE_S64: printf("%lli", node->con.s_value); break;
                case TYPE_MSI:
                case TYPE_F32:
                case TYPE_F64: printf("%f", node->con.f_value); break;
                case TYPE_B8: node->con.s_value ? printf("true") : printf("false"); break;
                default: break;
            }
            break;   
        }
        default: break;
    }
    
    
    printf("\033[0m"); 
    // printf(" %llu:%llu", node->token->line, node->token->column);
}

void ast_print_tree(Node* node, Heap_Allocator* heap, u64 depth = 0, b8* flags = nullptr, b8 is_last = false)
{
    IR_NOT_NULL(node);
    IR_NOT_NULL(heap);
    if(depth == 0 && flags == nullptr)
    {
        ARR_INIT(flags, 16, heap);
    }
    
    
    if(depth == ARR_LEN(flags))
    {
        ARR_PUSH(flags, true);   
    }
    
    for(msi i = 1; i < depth; ++i)
    {
        if(ARR_LEN(flags)>i && flags[i-1] == true)
        {
            printf("│  "); 
        }
        else
        {
            printf("   ");   
        }
    }
    
    if(depth == 0)
    {
        ast_print_node(node);
        printf("\n");
    }
    else if(is_last)
    {
        printf("└──");   
        ast_print_node(node);
        printf("\n");
        flags[depth-1] = false;   
    }
    else
    {
        printf("├──");   
        ast_print_node(node);
        printf("\n");
    }
    
    for(msi i = 0; node->children && i < ARR_LEN(node->children); ++i)
    {
        ast_print_tree(node->children[i], heap, depth + 1, flags,  i == (ARR_LEN(node->children)-1));
    }
    
    
    flags[depth-1] = true;
    
}

b8 ast_node_has_error_in_tree(Node* node)
{
    IR_NOT_NULL(node);
    b8 result = node->info.has_error;
    
    if(!result && node->children)
    {
        for(msi i = 0; i < ARR_LEN(node->children); ++i)
        {
            if(ast_node_has_error_in_tree(node->children[i]))
            {
                result = true;
               // node->info.has_error= true;
                break;
            }
        }
    }
    
    return result;
}

inline
void ast_node_add_child(Node* parent, Node* child, AST* ast)
{
    IR_NOT_NULL(parent);
    if(child)
    {
        if(parent->children == nullptr)
        {
            ARR_INIT(parent->children, 2, ast->heap);
        }
        
        child->parent = parent;
        ARR_PUSH(parent->children, child);
    }
}

inline
void ast_move_node(Node* node, Node* new_parent, AST* ast)
{
    if(node->parent)
    {
        for(msi i = 0; i < ARR_LEN(node->parent->children); ++i)
        {
            if(node->parent->children[i] == node)
            {
                ARR_DEL(node->parent->children, i);
                break;
            }
        }     
    }
    
    if(new_parent)
    {
        ast_node_add_child(new_parent, node, ast);
    }
}

inline
void ast_insert_between(Node* parent, msi child_index, Node* new_node, AST* ast)
{
    IR_NOT_NULL(parent);
    IR_NOT_NULL(new_node);
    if(parent->children)
    {
        IR_ASSERT(child_index <= ARR_LEN(parent->children));
        if(ARR_LEN(parent->children) > child_index)
        {
            Node* child = parent->children[child_index];
            new_node->parent = parent;
            parent->children[child_index] = new_node;
            ast_node_add_child(new_node, child, ast);
        }
        else
        {
            ast_node_add_child(parent, new_node, ast);
        }
    }
    else
    {
        IR_ASSERT(child_index == 0);
        ast_node_add_child(parent, new_node, ast);
    }
    
    
}

inline
Node* ast_create_node(Scope* scope, Token* t, AST* ast)
{
    IR_NOT_NULL(scope);
    Node* result;
    if(ast->node_free_list)
    {
        result = ast->node_free_list;
        ast->node_free_list = ast->node_free_list->info.next_in_free_list;
    }
    else
    {
        result = BA_PUSH(ast->nodes_ba, (Node){}); 
    }
    IR_NOT_NULL(result);
    result->info.token = t;
    result->scope = scope;
    return result;
}

inline 
Function* ast_create_fun(Token* t, AST* ast)
{
    Function* result = BA_PUSH(ast->functions_ba, (Function){});
    IR_NOT_NULL(result);
    result->token = t;
    ARR_INIT(result->params, 0, ast->heap);
    return result;
}

inline 
Variable* ast_create_var(Scope* scope, Token* t, AST* ast)
{
    IR_NOT_NULL(scope);
    Variable* result = BA_PUSH(ast->variables_ba, (Variable){});
    ARR_PUSH(scope->variables, result);
    IR_NOT_NULL(result);
    result->scope = scope;
    result->token = t;
    return result;
}

inline 
Scope* ast_create_scope(Scope* parent, AST* ast)
{
    Scope* result = BA_PUSH(ast->scopes_ba, (Scope){});
    IR_NOT_NULL(result);
    result->parent = parent;
    ARR_INIT(result->variables, 0, ast->heap);
    return result;
}


inline
Variable* ast_search_var_from_scope_and_name(String name, Scope* scope)
{
    Variable* result = nullptr;
    while(scope)
    {
        for(msi i = 0; i < ARR_LEN(scope->variables); ++i)
        {
            if(cmp_string(name, scope->variables[i]->name))
            {
                result = scope->variables[i];
                return result;
            }
        }
        
        scope = scope->parent;
    }
    return result;
}


inline
void ast_remove_node(Node* node, AST* ast)
{
    if(node->parent)
    {
        ast_move_node(node, nullptr, ast);
    }
    node->info.next_in_free_list = ast->node_free_list;
    ast->node_free_list = node;
}


#endif //AST_H
