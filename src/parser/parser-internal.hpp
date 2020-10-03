
#include "ast/ast.hpp"
#include "compiler/compiler-context.hpp"
#include "scanner/scanner.hpp"

#include "skip-to-sequence.hpp"
#include "token-sets.hpp"

namespace giraffe
{
template<typename T> inline bool expect(Scanner& tokens, const T& ids) noexcept;
template<typename T> inline bool accept(Scanner& tokens, const T& ids) noexcept;

// Recovery functions
bool recover_to_next_rule(Scanner& tokens) noexcept;
bool skip_past_element(Scanner& tokens) noexcept;

// Accept functions
GrammarNode* accept_grammar(CompilerContext&) noexcept;
RuleNode* accept_rule(CompilerContext&) noexcept;
ElementListNode* accept_element_list(CompilerContext&) noexcept;
ElementNode* accept_element(CompilerContext&) noexcept;

//
//
//
//
//
//
//
//
//
// ------------- implementations

template<typename T> inline bool expect(Scanner& tokens, const T& ids) noexcept
{
   if constexpr(std::is_integral<std::remove_reference_t<T>>::value) {
      return tokens.current().id() == ids;
   } else {
      const int current_id = tokens.current().id();
      const auto ii        = std::find(cbegin(ids), cend(ids), current_id);
      return (ii != cend(ids));
   }
}

template<typename T> inline bool accept(Scanner& tokens, const T& ids) noexcept
{
   if(expect(tokens, ids)) {
      tokens.consume();
      return true;
   }
   return false;
}

} // namespace giraffe
