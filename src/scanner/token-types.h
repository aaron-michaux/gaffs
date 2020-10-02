
#pragma once

#include <stdint.h>

#define TNONE 0

/* occurs at the start of every (new) file */
#define TSTART 1

/* occurs ONCE after all inputs have been processed */
#define TEOF 2

/* ----------------------------- ;:()|*+? */
#define TCOLON 10
#define TSEMICOLON 11

#define TLPAREN 20
#define TRPAREN 21
#define TPIPE 22
#define TSTAR 23
#define TPLUS 24
#define TQUESTION 25

#define TSTRING 30

#define TIDENTIFIER 40

#define TNEWLINE 50
#define TCOMMENT 51
#define TWHITESPACE 52

#define TBADCHAR 99

#ifdef __cplusplus
extern "C" {
#endif
extern const char* token_id_to_str(uint16_t token_id);
#ifdef __cplusplus
}
#endif
