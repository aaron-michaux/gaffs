
#pragma once

namespace giraffe
{
// --------------------------------------------------------------------- grammar
constexpr auto first_set_grammar  = to_array<int>({TSTART});
constexpr auto final_set_grammar  = to_array<int>({TEOF});
constexpr auto follow_set_grammar = array<int, 0>{};

// ------------------------------------------------------------------------ rule
constexpr auto first_set_rule    = to_array<int>({TIDENTIFIER});
constexpr auto final_set_rule    = to_array<int>({TSEMICOLON});
constexpr auto follow_set_rule   = to_array<int>({TEOF, TIDENTIFIER});
constexpr auto recovery_seq_rule = to_array<int>({TIDENTIFIER, TCOLON});

// ---------------------------------------------------------------- element-list
constexpr auto first_set_element_list
    = to_array<int>({TIDENTIFIER, TSTRING, TLPAREN});
constexpr auto final_set_element_list
    = to_array<int>({TIDENTIFIER, TSTRING, TRPAREN, TSTAR, TPLUS, TQUESTION});
constexpr auto follow_set_element_list
    = to_array<int>({TPIPE, TSEMICOLON, TRPAREN});

// --------------------------------------------------------------------- element
constexpr auto first_set_element
    = to_array<int>({TIDENTIFIER, TSTRING, TLPAREN});
constexpr auto final_set_element
    = to_array<int>({TIDENTIFIER, TSTRING, TRPAREN, TSTAR, TPLUS, TQUESTION});
constexpr auto follow_set_element
    = to_array<int>({TIDENTIFIER, TSTRING, TLPAREN, TPIPE, TSEMICOLON});

constexpr auto token_suffix_list = to_array<int>({TSTAR, TPLUS, TQUESTION});

} // namespace giraffe
