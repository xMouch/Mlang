#ifndef TOKENS_H
#define TOKENS_H

#include "stdio.h"
#include "stdlib.h"

#include "ir_types.h"
#include "ir_memory.h"
#include "ir_string.h"
#include "ir_ds.h"

enum Token_Type
{
    TOKEN_UNKOWN = 0,
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_STR_LIT,
    TOKEN_BASIC_TYPE,
    
    TOKEN_FN        = 1000,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONT,
    TOKEN_STRUCT,
    TOKEN_CAST,
    
    TOKEN_D_EQ      = 1100, // ==
    TOKEN_AND, // &&
    TOKEN_OR, // ||
    TOKEN_NOTEQ, // !=
    TOKEN_LEQ, // <=
    TOKEN_GEQ, // >=
    
    TOKEN_D_PLUS, // ++
    TOKEN_D_MINUS, // --
    TOKEN_PLUS_EQ, // +=
    TOKEN_MINUS_EQ, // -=
    TOKEN_MUL_EQ, // *=
    TOKEN_DIV_EQ, // /=
    TOKEN_MOD_EQ, // %=
    TOKEN_SHIFT_L_EQ, // <<=
    TOKEN_SHIFT_R_EQ, // >>=
    TOKEN_AND_EQ, // &=
    TOKEN_XOR_EQ, // ^=
    TOKEN_OR_EQ, // |=
    TOKEN_SHIFT_L, // <<
    TOKEN_SHIFT_R, // >>
    
    TOKEN_COMBINED,
    
    /*
       All single charactars are the same value as their ASCII value so A == 'A' == 65

    */
    
};

enum Type
{
    TYPE_UNKNOWN = 0,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_MSI,
    TYPE_S8,
    TYPE_S16,
    TYPE_S32,
    TYPE_S64,
    TYPE_F32,
    TYPE_F64,
    TYPE_B8,
    TYPE_VOID,
    TYPE_CUSTOM,
    TYPE_COUNT,
};

struct Token
{
    String text;
    String line_text;
    Token_Type type;
    Type data_type;
    msi line;
    msi column;
};

msi count_heading_whitespace_token_line(Token* t)
{
    msi result = 0;
    for(msi i = 1; i < t->column; ++i)
    {
        if(t->line_text.data[-i] == '\n')
        {
            break;
        }
        ++result;
        if(t->line_text.data[-i] != ' ')
        {
            result = 0;   
        }
    }
    
    return result;
}

static
String data_type_to_str(Type t)
{
    switch(t)
    {
        case TYPE_UNKNOWN: return IR_CONSTZ("unknown");
        case TYPE_U8: return IR_CONSTZ("u8");
        case TYPE_U16: return IR_CONSTZ("u16");
        case TYPE_U32: return IR_CONSTZ("u32");
        case TYPE_U64: return IR_CONSTZ("u64");
        case TYPE_S8: return IR_CONSTZ("s8");
        case TYPE_S16: return IR_CONSTZ("s16");
        case TYPE_S32: return IR_CONSTZ("s32");
        case TYPE_S64: return IR_CONSTZ("s64");
        case TYPE_MSI: return IR_CONSTZ("msi");
        case TYPE_F32: return IR_CONSTZ("f32");
        case TYPE_F64: return IR_CONSTZ("f64");
        case TYPE_B8: return IR_CONSTZ("b8");
        case TYPE_VOID: return IR_CONSTZ("void");
        case TYPE_CUSTOM: return IR_CONSTZ("custom");
        default: return IR_CONSTZ("data type print not implemented!");
    }
}

static
String token_type_to_str(Token_Type t)
{
    switch(t)
    {
        case TOKEN_UNKOWN: return IR_CONSTZ("TOKEN_UNKOWN");
        case TOKEN_EOF:    return IR_CONSTZ("EOF");
        case TOKEN_ID:     return IR_CONSTZ("Identifier");
        case TOKEN_NUM:    return IR_CONSTZ("Number Constant");
        case TOKEN_STR_LIT:return IR_CONSTZ("String Literal");
        case TOKEN_FN:     return IR_CONSTZ("fn");
        case TOKEN_RETURN: return IR_CONSTZ("return");
        case TOKEN_IF:     return IR_CONSTZ("if");
        case TOKEN_ELSE:   return IR_CONSTZ("else");
        case TOKEN_FOR:    return IR_CONSTZ("for");
        case TOKEN_WHILE:  return IR_CONSTZ("while");
        case TOKEN_BREAK:  return IR_CONSTZ("break");
        case TOKEN_CONT:   return IR_CONSTZ("continue");
        case TOKEN_STRUCT: return IR_CONSTZ("struct");
        case TOKEN_CAST:   return IR_CONSTZ("cast");
        case TOKEN_BASIC_TYPE:    return IR_CONSTZ("basic type");
        case TOKEN_D_EQ:   return IR_CONSTZ("==");
        case TOKEN_AND:    return IR_CONSTZ("&&");
        case TOKEN_OR:     return IR_CONSTZ("||");
        case TOKEN_LEQ:  return IR_CONSTZ("<=");
        case TOKEN_GEQ:  return IR_CONSTZ("!=");
        case TOKEN_D_PLUS:  return IR_CONSTZ("++");
        case TOKEN_D_MINUS:  return IR_CONSTZ("--");
        case TOKEN_PLUS_EQ:  return IR_CONSTZ("+=");
        case TOKEN_MINUS_EQ:  return IR_CONSTZ("-=");
        case TOKEN_MUL_EQ:  return IR_CONSTZ("*=");
        case TOKEN_DIV_EQ:  return IR_CONSTZ("/=");
        case TOKEN_MOD_EQ:  return IR_CONSTZ("%=");
        case TOKEN_SHIFT_L_EQ:  return IR_CONSTZ("<<=");
        case TOKEN_SHIFT_R_EQ:  return IR_CONSTZ(">>=");
        case TOKEN_AND_EQ:  return IR_CONSTZ("&=");
        case TOKEN_XOR_EQ:  return IR_CONSTZ("^=");
        case TOKEN_OR_EQ:  return IR_CONSTZ("|=");
        case TOKEN_SHIFT_L:  return IR_CONSTZ("<<");
        case TOKEN_SHIFT_R:  return IR_CONSTZ(">>");
        case '(': return IR_CONSTZ("'('");
        case ')': return IR_CONSTZ("')'");
        case '[': return IR_CONSTZ("'['");
        case ']': return IR_CONSTZ("']'");
        case '{': return IR_CONSTZ("'{'");
        case '}': return IR_CONSTZ("'}'");
        case '<': return IR_CONSTZ("'<'");
        case '>': return IR_CONSTZ("'>'");
        case '%': return IR_CONSTZ("'%'");
        case '+': return IR_CONSTZ("'+'");
        case '-': return IR_CONSTZ("'-'");
        case '*': return IR_CONSTZ("'*'");
        case '/': return IR_CONSTZ("'/'");
        case '!': return IR_CONSTZ("'!'");
        case '=': return IR_CONSTZ("'='");
        case ':': return IR_CONSTZ("':'");
        case ';': return IR_CONSTZ("';'");
        case ',': return IR_CONSTZ("','");
        case '.': return IR_CONSTZ("'.'");
        case '#': return IR_CONSTZ("'#'");
        case '&': return IR_CONSTZ("'&'");
        case '|': return IR_CONSTZ("'|'");
        case '^': return IR_CONSTZ("'^'");
        case '$': return IR_CONSTZ("'$'");
        case '?': return IR_CONSTZ("'?'");
        case '~': return IR_CONSTZ("'~'");
        default: return IR_CONSTZ("TOKEN TYPE PRINT NOT IMPLEMENTED");
    }
}

inline
b8 data_type_is_unsigned(Type data)
{
    if((data <= TYPE_MSI && data != TYPE_UNKNOWN) || data == TYPE_B8)
    {
        return true;   
    }
    return false;
}

inline
b8 data_type_is_signed(Type data)
{
    if((data <= TYPE_S64 && data >= TYPE_S8) || data == TYPE_B8)
    {
        return true;   
    }
    return false;
}

inline
b8 data_type_is_floating_point(Type data)
{
    if(data ==TYPE_F32 || data == TYPE_F64)
    {
        return true;   
    }
    return false;
}

inline
u8 data_type_size(Type data)
{
    switch(data)
    {
        case TYPE_UNKNOWN: return 0;
        case TYPE_U8:  return 8;
        case TYPE_U16: return 16;
        case TYPE_U32: return 32;
        case TYPE_U64: return 64;
        case TYPE_MSI: return 64;
        case TYPE_S8:  return 8;
        case TYPE_S16: return 16;
        case TYPE_S32: return 32;
        case TYPE_S64: return 64;
        case TYPE_F32: return 32;
        case TYPE_F64: return 64;
        case TYPE_B8:  return 8;
        case TYPE_VOID: return 0;
        case TYPE_CUSTOM: return 0;
        default: return 0;
    }
}


#endif //TOKENS_H
