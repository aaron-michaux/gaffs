
#pragma once

#include "token-types.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parameter block that bridges between `flex` and the scanner
 */
struct scanner_params_t
{
   const char* text;        //!< Token text (memory owned by lexer)
   uint32_t text_len;       //!< Length of the current token
   uint32_t offset;         //!< Offset into the underlying source file
   uint32_t last_offset;    //!< Offset of previous token
   uint32_t line_no;        //!< Line number of current token, 0-indexed
   uint32_t last_line_no;   //!< Line number of previous token, 0-indexed
   uint16_t column_no;      //!< Column number of curren token, 0-indexed
   uint16_t last_column_no; //!< Column number of previous token, 0-indexed
   uint16_t token_id;       //!< Token identifier
};

/** Create (and initialize) a new scanner */
extern void* init_scanner(FILE* fp);

/** Free resources allocated to the scanner */
extern void destroy_scanner(void* scanner);

/** Retrieve scanner parameters */
extern struct scanner_params_t* get_scanner_params(void* scanner);

/** Get flex to lex a single token */
extern int yylex(void* scanner);

#ifdef __cplusplus
}
#endif
