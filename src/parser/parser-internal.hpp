
#include "ast/ast.hpp"
#include "compiler/compiler-context.hpp"
#include "scanner/token-producer.hpp"

#include "skip-to-sequence.hpp"
#include "token-sets.hpp"

namespace giraffe
{
template<typename T>
inline bool expect(TokenProducer& tokens, const T& ids) noexcept;
template<typename T>
inline bool accept(TokenProducer& tokens, const T& ids) noexcept;

// Recovery functions
bool recover_to_next_rule(TokenProducer& tokens) noexcept;
bool recover_to_next_element_list(TokenProducer& tokens) noexcept;

// Accept functions
GrammarNode* accept_grammar(CompilerContext&, TokenProducer&) noexcept;
RuleNode* accept_rule(CompilerContext&, TokenProducer&) noexcept;
ElementListNode* accept_element_list(CompilerContext&, TokenProducer&) noexcept;
ElementNode* accept_element(CompilerContext&, TokenProducer&) noexcept;

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

template<typename T>
inline bool expect(TokenProducer& tokens, const T& ids) noexcept
{
   if constexpr(std::is_integral<std::remove_reference_t<T>>::value) {
      return tokens.current().id() != ids;
   } else {
      const int current_id = tokens.current().id();
      const auto ii        = std::find(cbegin(ids), cend(ids), current_id);
      return (ii != cend(ids));
   }
}

template<typename T>
inline bool accept(TokenProducer& tokens, const T& ids) noexcept
{
   if(expect(tokens, ids)) {
      tokens.consume();
      return true;
   }
}

} // namespace giraffe
