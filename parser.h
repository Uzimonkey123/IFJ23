/**
 *  @file parser.h
 *
 *
 *  @authors Milan Jakubec (xjakub41)
 *  @authors Jakub Ráček (xracek12)
 *  @brief Semantic and syntax analysis header file
 */

#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include <stdbool.h>
#include "gen.h"

typedef struct parser
{
    stack_t *stack;                 // Stack instance
    htab_value_t *function;         // Current function
    token_t token;                  // Current token
    token_t next_token;             // Next token
    gen_t *gen;                     // Generator instance
    variable_type_t return_type;    // Return type of current function
    
    // States of parser
    bool in_function;
    bool func_is_void;
    bool returned;
    bool returned_conditional;
    bool in_cycle;
    bool in_if;
    bool in_else;
} parser_t;

/**
 * @brief Initializes parser
 *
 * @param scanner Pointer to scanner
 * @return Initialized parser structure
 */

void parser_init(parser_t *parser, gen_t *gen);

/**
 * @brief Runs parser
 */
void run_parser(parser_t *parser);

/**
 * @brief Destroys parser
 */
void parser_destroy(parser_t *parser);

/**
 * @brief Loads token from scanner
 */
void load_token(parser_t *parser);

#endif // PARSER_H