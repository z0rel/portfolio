#ifndef LEXLIB_INTERNAL_HEADER_H
#define LEXLIB_INTERNAL_HEADER_H


#include "project_optimization.h"

COMPILER_LEXER_OPTIMIZATION_PUSH()

#include "lexlib.h"

void lex_get_rawId(yyscan_t yyscanner);

template <yyinput_t yyinput>
inline void lex_get_quoted2(yyscan_t yyscanner, int endchar);


template <int endc, yyinput_t yyinput>
inline void lex_get_quoted(yyscan_t yyscanner);

template <int endc, yyinput_t yyinput>
inline void lex_get_python_quoted(yyscan_t yyscanner);

template <int endc, yyinput_t yyinput>
inline void lex_get_3quoted(yyscan_t yyscanner);

template <yyinput_t yyinput, int prefix_len>
inline void lex_get_multiline_comment(yyscan_t yyscanner);

template <yyinput_t yyinput>
inline void skipWrappedEntity(yyscan_t yyscanner);

template <int begin_offset, int end_offset>
inline void llocBuf(yyscan_t yyscanner);

inline void llocBufSimple(yyscan_t yyscanner);


COMPILER_LEXER_OPTIMIZATION_POP()
#endif // LEXLIB_INTERNAL_HEADER_H
