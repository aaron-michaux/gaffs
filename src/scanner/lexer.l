%{
#include <stdio.h>
#include "scanner/scanner_bridge.h"

#define YY_INPUT(buf, result, max_size) {          \
        size_t sz = fread(buf, 1, max_size, yyin); \
        if(sz == 0) result = YY_NULL;              \
        else result = sz;                          \
    }

#define TOKEN(TOK_ID) (lexx_token((TOK_ID), yyextra, yytext, yyleng, yylineno))

int lexx_token(int token_id,
               struct scanner_params_t* params,
               const char* text,
               int text_len,
               int lineno);

%}

%option 8bit noyywrap reentrant
%option extra-type="struct scanner_params_t *"
%option fast
%option warn nodefault
%option never-interactive
%option yylineno

NL      ([\r\n])
WS      [\t ]
L	[@_a-zA-Z]
D	[@_\-\./\\a-zA-Z0-9]
A       ([\x01-\x09])|([\x20-\x7f])|{WS}|{NL}
NQ      ([\x21-\x21])|([\x23-\x7f])|{WS}|{NL}
NP      ([\x21-\x26])|([\x28-\x7f])|{WS}|{NL}


%%

<<EOF>>			       { return TOKEN(TEOF); }

"//".*			       { return TOKEN(TCOMMENT); }

"\""(\\.|{NQ})*"\""	       { return TOKEN(TSTRING); }
"'"(\\.|{NP})*"'"	       { return TOKEN(TSTRING); }

":"                            { return TOKEN(TCOLON); }
";"                            { return TOKEN(TSEMICOLON); }
"("                            { return TOKEN(TLPAREN); }
")"                            { return TOKEN(TRPAREN); }
"|"                            { return TOKEN(TPIPE); }
"*"                            { return TOKEN(TSTAR); }
"+"                            { return TOKEN(TPLUS); }
"?"                            { return TOKEN(TQUESTION); }

{L}{D}*                          { return TOKEN(TIDENTIFIER); }

{WS}+			       { return TOKEN(TWHITESPACE); }
{NL}                           { return TOKEN(TNEWLINE); }

[\x00-\xff]		       { return TOKEN(TBADCHAR); }

%%

void scanner_params_init(struct scanner_params_t* params)
{
    memset(params, 0, sizeof(struct scanner_params_t));
}

void scanner_params_destroy(struct scanner_params_t* params)
{
    free(params);
}

extern yyscan_t init_scanner(FILE* fp)
{
    struct scanner_params_t* params
        = malloc(sizeof(struct scanner_params_t));
    yyscan_t scanner;

    scanner_params_init(params);
    yylex_init(&scanner);
    yyset_extra(params, scanner);
    yyset_in(fp, scanner);
    
    return scanner;
}

extern struct scanner_params_t* get_scanner_params(yyscan_t scanner)
{
    return yyget_extra(scanner);
}

extern void destroy_scanner(yyscan_t scanner)
{
    scanner_params_destroy(yyget_extra(scanner));
    yylex_destroy(scanner);
}

int lexx_token(int token_id,
               struct scanner_params_t* params,
               const char* text,
               int text_len,
               int lineno)
{
    params->token_id = token_id;
    params->text = ""; 
    params->text_len = 0;
    params->last_line_no = params->line_no;
    params->last_column_no = params->column_no;
    params->last_offset = params->offset;
    params->line_no = lineno;
    
    if(token_id == TEOF) {
        params->column_no = 0;
    } else {
        params->text = text; 
        params->text_len = text_len;
        params->line_no = lineno;
        params->column_no = params->last_column_no + text_len;
        params->offset = params->last_offset + text_len;
    }

    if(token_id == TNEWLINE)
        params->column_no = 0;
    
    /* Always return the token_id */
    return token_id;
}
