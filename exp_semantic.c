/**
 *  @file exp_semantic.c
 * 
 * 
 *  @authors Norman Babiak (xbabia01)
 */

//TODO: ADDING GENERATORS

#include "exp_semantic.h"
#include "error.h"

extern error_t error;

sem_data_type_t tok_type(parser_t* parserData) { 
    data_t *tmpData;

    switch(parserData->token.type) {
        case TOK_NOT:
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_MUL:
        case TOK_DIV:
        case TOK_LBRACKET:
        case TOK_RBRACKET:
        case TOK_GREATER:
        case TOK_LESS:
        case TOK_EQUAL:
        case TOK_NOTEQUAL:
        case TOK_GREATEREQ:
        case TOK_LESSEQ:
        case TOK_DQUESTMK:
            return SEM_OPERATOR;
        case TOK_INT:
            return SEM_INT;
        case TOK_DOUBLE:
            return SEM_FLOAT;
        case TOK_STRING:
        case TOK_MLSTRING:
            return SEM_STRING;
        case K_NIL:
            return SEM_NIL;
        case TOK_IDENTIFIER:

            tmpData = stack_lookup_var(parserData->stack, parserData->token.attribute.string);
            if(tmpData->type == VAR || tmpData->type == LET) {
                switch(tmpData->value.var_id.type) {
                    case VAL_INT:
                    case VAL_INTQ:
                        return SEM_INT;
                    case VAL_STRING:
                    case VAL_STRINGQ:
                        return SEM_STRING;
                    case VAL_DOUBLE:
                    case VAL_DOUBLEQ:
                        return SEM_FLOAT;
                    case VAL_BOOL:
                        return SEM_BOOL;
                    case VAL_NIL:
                        return SEM_NIL;
                    default:
                        return SEM_UNDEF;
                }
            }
            return SEM_UNDEF;

        default:
            return SEM_UNDEF;
    }
}

bool is_string(sem_data_type_t data) {
    return (data == SEM_STRING);
}

bool is_int(sem_data_type_t data) {
    return (data == SEM_INT);
}

bool is_float(sem_data_type_t data) {
    return (data == SEM_FLOAT);
}

bool is_nil(sem_data_type_t data) {
    return (data == SEM_NIL);
}

bool is_operator(sem_data_type_t data) {
    return (data = SEM_OPERATOR);
}

bool is_number(sem_data_type_t data) {
    return (is_float(data) || is_int(data));
}

bool check_operator_compatibility(stack_terminal_t* operator, stack_terminal_t* left, stack_terminal_t* right) {
    switch(operator->type) {
        case TOK_PLUS:
            return (is_string(left->data) && is_string(right->data)) ||
                   (is_number(left->data) && is_number(right->data));

        case TOK_MINUS:
        case TOK_MUL:
        case TOK_DIV:
            return is_number(left->data) && is_number(right->data);
        
        case TOK_EQUAL:
        case TOK_NOTEQUAL:
            return (is_string(left->data) && is_string(right->data)) ||
                   (is_float(left->data) && is_float(right->data))   ||
                   (is_int(left->data) && is_int(right->data));
                   
        case TOK_LESS:
        case TOK_GREATER:
        case TOK_GREATEREQ:
        case TOK_LESSEQ:
            return is_number(left->data) && is_number(right->data);

        case TOK_DQUESTMK:
            return ((is_string(left->data) || is_nil(left->data)) && (is_string(right->data) || is_nil(right->data))) ||
                   ((is_int(left->data) || is_nil(left->data)) && (is_int(right->data) || is_nil(right->data))) ||
                   ((is_float(left->data) || is_nil(left->data)) && (is_float(right->data) || is_nil(right->data)));

        default:
            error = ERR_SEM_TYPE;
            break;
    }

    error = ERR_SEM_TYPE;
    return false;
}

int get_result_type(stack_terminal_t* operator, stack_terminal_t* left, stack_terminal_t* right) {
    if(operator->type == TOK_PLUS) {
        if(is_number(left->data) && is_number(right->data)) {
            if(is_int(left->data) && is_int(right->data)) {
                return SEM_INT;
            } else {
                if(is_int(left->data)) {
                     gen_call_convert(gen);
                } else if(is_int(right->data)) {
                    gen_call_convert2(gen);
                }
                return SEM_FLOAT;
            }
        } else {
            return SEM_STRING;
        }
    }

    if(operator->type == TOK_MINUS || operator->type == TOK_MUL || operator->type == TOK_DIV) {
        if(is_number(left->data) && is_number(right->data)) {
            if(is_int(left->data) && is_int(right->data)) {
                return SEM_INT;
            } else {
                if(is_int(left->data)) {
                     gen_call_convert(gen);
                } else if(is_int(right->data)) {
                    gen_call_convert2(gen);
                }
                return SEM_FLOAT;
            }
        }
    }

    if(operator->type == TOK_EQUAL || operator->type == TOK_NOTEQUAL) {
        //Since these two operators accept just the same data type from both variables,
        //there is no need for additional check.
        return SEM_BOOL;
    }

    if(operator->type == TOK_LESS || operator->type == TOK_GREATER || operator->type == TOK_GREATEREQ || 
       operator->type == TOK_LESSEQ) {
        if(right->data != left->data) {
            if(is_int(left->data)) {
                gen_call_convert(gen);
            } else if(is_int(right->data)) {
                gen_call_convert2(gen);
            }
        }

        return SEM_BOOL;
    }

    if(operator->type == TOK_DQUESTMK) {
        if(is_nil(right->data)) {
            return left->data;

        } else if(is_nil(right->data) && is_nil(left->data)) {
            return SEM_NIL;

        } else if(is_nil(left->data)) {
            return right->data;

        } else {
            return left->data;
        }
    }

    return SEM_UNDEF;
}

bool sem_analysis(analysis_t* analysis) {
    if (!check_operator_compatibility(analysis->tok2, analysis->tok1, analysis->tok3)) {
        error = ERR_SEM_TYPE;
        return false;
    }

    analysis->end_type = get_result_type(analysis->tok2, analysis->tok1, analysis->tok3);
    if(analysis->end_type == SEM_UNDEF) {
        error = ERR_SEM_TYPE;
        return false;
    }

    return true;
}