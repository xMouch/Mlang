#ifndef TOKENIZER_H
#define TOKENIZER_H

struct Tokenizer
{
    String file;
    String line_text;
    Token* tokens;
    String* str_literals;
    c8  n[3];
    msi cur;
    msi cur_line;
    msi cur_column;
};

static
Tokenizer create_tokenizer(String file, Heap_Allocator* heap)
{
    Tokenizer t = {};
    t.file = file;
    t.line_text = substring(file, 0, search_string_first_occurrence(file, '\n').data - file.data +1);
    t.str_literals = nullptr;
    ARR_INIT(t.str_literals, 16, heap);
    ARR_INIT(t.tokens, 64, heap);
    
    t.cur_line = 1;
    t.cur_column = 1;
    
    if(file.length >= 1)
    {
        t.n[0]=t.file.data[0];
    }
    if(file.length >= 2)
    {
        t.n[1]=t.file.data[1];
    }
    
    return t;
    
}

static 
void adv_chars(Tokenizer* t, msi count)
{
    for(msi i = 0; i < count; ++i)
    {
        if(t->cur >= t->file.length)
        {
            t->n[0] = 0;
            t->n[1] = 0;
            t->n[2] = 0;
            break;
        }
        
        ++t->cur_column;
        if(is_end_of_line(t->file.data[t->cur]))
        {
            ++t->cur_line;
            t->cur_column = 1;
            
            msi begin = 1;
            for(;is_whitespace(t->file.data[t->cur+begin]);++begin)
            {}
            
            t->line_text.data = &t->file.data[t->cur+begin];
            
            msi i = 0;
            for(; !is_end_of_line(t->line_text.data[i]); ++i)
            {}
            
            t->line_text.length = i+1;
            
        }
        ++t->cur;
        
        t->n[0] = t->file.data[t->cur];
        
        if(t->cur+1 < t->file.length)
            t->n[1] = t->file.data[t->cur+1];
        else
            t->n[1] = 0;
        
        if(t->cur+2 < t->file.length)
            t->n[2] = t->file.data[t->cur+2];
        else
            t->n[2] = 0;
    }
}

static 
void skip_whitespace_and_comments(Tokenizer* t)
{
    for(;;)
    {
        if(is_whitespace(t->n[0]) || is_end_of_line(t->n[0]))
        {
            adv_chars(t, 1);
            continue;
        }
        //Skip CPP style comment
        if(t->n[0] == '/' && t->n[1] == '/')
        {
            while(!is_end_of_line(t->n[0]))
            {
                adv_chars(t, 1);
            }
            continue;
        }
  
        
        //Skip C style comment
        if(t->n[0] == '/' && t->n[1] == '*')
        {
            while(!(t->n[0] == '*' && t->n[1] == '/'))
            {
                adv_chars(t, 1);
            }  
            adv_chars(t, 2);
            continue;
        }
        
        break;
    }
}

static
Token* tokenize(String file, Heap_Allocator* heap)
{
    IR_NOT_NULL(heap);
    Tokenizer t = create_tokenizer(file, heap);
    
    while(t.n[0] != 0)
    {
        skip_whitespace_and_comments(&t);
        
        Token token = {};
        token.text.data = &t.file.data[t.cur];
        token.text.length = 1;
        token.line = t.cur_line;
        token.column = t.cur_column;
        token.line_text = t.line_text;
        
        
        if(t.n[0] == '=' && t.n[1] == '=')
        {
            token.type = TOKEN_D_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '&' && t.n[1] == '&')
        {
            token.type = TOKEN_AND;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '|' && t.n[1] == '|')
        {
            token.type = TOKEN_OR;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '!' && t.n[1] == '=')
        {
            token.type = TOKEN_OR;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }

        if(t.n[0] == '<' && t.n[1] == '=')
        {
            token.type = TOKEN_LEQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }

        if(t.n[0] == '>' && t.n[1] == '=')
        {
            token.type = TOKEN_GEQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '+' && t.n[1] == '+')
        {
            token.type = TOKEN_D_PLUS;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '-' && t.n[1] == '-')
        {
            token.type = TOKEN_D_MINUS;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '+' && t.n[1] == '=')
        {
            token.type = TOKEN_PLUS_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '-' && t.n[1] == '=')
        {
            token.type = TOKEN_MINUS_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '*' && t.n[1] == '=')
        {
            token.type = TOKEN_MUL_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '/' && t.n[1] == '=')
        {
            token.type = TOKEN_DIV_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '%' && t.n[1] == '=')
        {
            token.type = TOKEN_MOD_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '<' && t.n[1] == '<' && t.n[2] == '=')
        {
            token.type = TOKEN_SHIFT_L_EQ;
            adv_chars(&t, 3);
            token.text.length = 3;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '>' && t.n[1] == '>' && t.n[2] == '=')
        {
            token.type = TOKEN_SHIFT_R_EQ;
            adv_chars(&t, 3);
            token.text.length = 3;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '&' && t.n[1] == '=')
        {
            token.type = TOKEN_AND_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '^' && t.n[1] == '=')
        {
            token.type = TOKEN_XOR_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '|' && t.n[1] == '=')
        {
            token.type = TOKEN_OR_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '<' && t.n[1] == '<')
        {
            token.type = TOKEN_SHIFT_L;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(t.n[0] == '>' && t.n[1] == '>')
        {
            token.type = TOKEN_SHIFT_R;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        switch(t.n[0])
        {
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '<':
            case '>':
            case '%':
            case '+':
            case '-':
            case '*':
            case '/':
            case '!':
            case '=':
            case ':':
            case ';':
            case ',':
            case '.':
            case '#':
            case '&':
            case '|':
            case '^':
            case '$':
            case '?':
            case '~':
            {
                token.type = (Token_Type)t.n[0];
                adv_chars(&t, 1);
                
                ARR_PUSH(t.tokens, token);
                continue;
            }
        }
        
        if(t.n[0] == '"')
        {
            String str_lit;
            ARR_INIT(str_lit.data, 4, heap);
            token.type = TOKEN_STR_LIT;
            adv_chars(&t, 1);
            token.line = t.cur_line;
            token.column = t.cur_column;
            
            while(t.n[0] != '"' && t.n[0] != 0)
            {
                
                if(t.n[0] == '\\' && t.n[1] == '"')
                {
                    adv_chars(&t, 2);
                    ARR_PUSH(str_lit.data, t.n[0]);
                    ARR_PUSH(str_lit.data, t.n[1]);
                }
                else if(t.n[0] == '\\' && t.n[1] == 'n')
                {
                    adv_chars(&t, 2);
                    ARR_PUSH(str_lit.data, '\n');
                }
                else
                {
                    ARR_PUSH(str_lit.data, t.n[0]);
                    adv_chars(&t, 1);
                }
            }
            
            if(t.n[0] == '"')
                adv_chars(&t, 1);
            
            str_lit.length = ARR_LEN(str_lit.data);
            token.text.length = str_lit.length;
            token.text.data = str_lit.data;
            ARR_PUSH(t.str_literals, str_lit); 
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(is_alpha(t.n[0]) || t.n[0] == '_')
        {
            token.type = TOKEN_ID;
            
            u32 length = 1;
            adv_chars(&t, 1);
            while( is_alpha(t.n[0]) || is_number(t.n[0]) || t.n[0] == '_')
            {
                adv_chars(&t,1);
                length++;
            }
            
            token.text.length = length;
            
            if(cmp_string(IR_CONSTZ("fn"), token.text))
            {
                token.type = TOKEN_FN;   
            }else if(cmp_string(IR_CONSTZ("return"), token.text))
            {
                token.type = TOKEN_RETURN;   
            }else if(cmp_string(IR_CONSTZ("if"), token.text))
            {
                token.type = TOKEN_IF;   
            }else if(cmp_string(IR_CONSTZ("else"), token.text))
            {
                token.type = TOKEN_ELSE;   
            }else if(cmp_string(IR_CONSTZ("for"), token.text))
            {
                token.type = TOKEN_FOR;   
            }else if(cmp_string(IR_CONSTZ("while"), token.text))
            {
                token.type = TOKEN_WHILE;   
            }else if(cmp_string(IR_CONSTZ("break"), token.text))
            {
                token.type = TOKEN_BREAK;   
            }else if(cmp_string(IR_CONSTZ("continue"), token.text))
            {
                token.type = TOKEN_CONT;   
            }else if(cmp_string(IR_CONSTZ("struct"), token.text))
            {
                token.type = TOKEN_STRUCT;   
            }else if(cmp_string(IR_CONSTZ("cast"), token.text))
            {
                token.type = TOKEN_CAST;   
            }else if(cmp_string(IR_CONSTZ("s8"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;
                token.data_type = TYPE_S8;
            }else if(cmp_string(IR_CONSTZ("s16"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;
                token.data_type = TYPE_S16;   
            }else if(cmp_string(IR_CONSTZ("s32"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;   
                token.data_type = TYPE_S32;
            }else if(cmp_string(IR_CONSTZ("s64"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;
                token.data_type = TYPE_S64;  
            }else if(cmp_string(IR_CONSTZ("u8"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;  
                token.data_type = TYPE_U8; 
            }else if(cmp_string(IR_CONSTZ("u16"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;   
                token.data_type = TYPE_U16; 
            }else if(cmp_string(IR_CONSTZ("u32"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;  
                token.data_type = TYPE_U32;  
            }else if(cmp_string(IR_CONSTZ("u64"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;  
                token.data_type = TYPE_U64;  
            }else if(cmp_string(IR_CONSTZ("msi"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;  
                token.data_type = TYPE_MSI;  
            }else if(cmp_string(IR_CONSTZ("f32"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE; 
                token.data_type = TYPE_F32;   
            }else if(cmp_string(IR_CONSTZ("f64"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE; 
                token.data_type = TYPE_F64;   
            }else if(cmp_string(IR_CONSTZ("b8"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE;  
                token.data_type = TYPE_B8;  
            }else if(cmp_string(IR_CONSTZ("void"), token.text))
            {
                token.type = TOKEN_BASIC_TYPE; 
                token.data_type = TYPE_VOID;   
            }
            
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        
        if(is_number(t.n[0]))
        {
            token.type = TOKEN_NUM;
            u32 num_length = 0;
            bool isFloat = false;
            while(is_number(t.n[0]) || (!isFloat && t.n[0]=='.'))
            {
                if (t.n[0]=='.'){
                    isFloat = true;
                }
                num_length++;
                adv_chars(&t, 1);
            }
            token.text.length = num_length;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        
        
        if(token.type == TOKEN_UNKOWN)
        {
            adv_chars(&t, 1);
            
            //ARR_PUSH(t.tokens, token);
            continue;
        }
    }
    
    Token token = {};
    token.text.data = &t.file.data[t.cur];
    token.text.length = 0;
    token.type = TOKEN_EOF;
    token.line = t.cur_line;
    token.column = t.cur_column;
    ARR_PUSH(t.tokens, token);
    
    return t.tokens;
}


#endif //TOKENIZER_H
