
#pragma once

#include "token-types.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------ Scanner */

struct scanner_params_t
{
   uint32_t offset;
   uint32_t last_offset;

   uint32_t line_no;
   uint32_t last_line_no;

   uint16_t column_no;
   uint16_t last_column_no;

   uint16_t token_id;
   const char* text;
   uint32_t text_len;
};

/** Defined in lexer.l */
extern void* init_scanner(FILE* fp);        /*!< Call before first yylex */
extern void destroy_scanner(void* scanner); /*!< Call after all yylex */
extern struct scanner_params_t*
get_scanner_params(void* scanner); /*!< Call to retrieve scanner parameters */
extern int yylex(void* scanner);

#ifdef __cplusplus
}
#endif
