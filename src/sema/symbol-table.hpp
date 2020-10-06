
#pragma once

// @see
// https://www.tutorialspoint.com/compiler_design/compiler_design_symbol_table.htm

namespace giraffe
{
// -------------------------------------------------------------- Predefinitions

class AstNode;

// ------------------------------------------------------------------------ Type
// Out simple langage has two types:
enum class Type : int8_t {
   NONE = 0, // null value
   RULE,     // a production rule
   TOKEN,    // a (lexer) token
   STRING    // a string literal
};

const char* str(Type type) noexcept;

// ---------------------------------------------------------------------- Symbol

struct Symbol final
{
   string_view label = "";
   AstNode* node     = nullptr; // if any...
   Type type         = Type::NONE;
};

Symbol make_rule_symbol(string_view label, AstNode* node = nullptr) noexcept;
Symbol make_token_symbol(string_view label, AstNode* node = nullptr) noexcept;
Symbol make_string_symbol(string_view label, AstNode* node = nullptr) noexcept;

// ----------------------------------------------------------------------- Scope

class Scope final
{
 private:
   Scope* parent_ = nullptr;

   // How many symbols before transfering from `vec_` to `map_`
   static constexpr size_t map_min_size_        = 16;
   bool using_vec_                              = true;
   vector<std::pair<string_view, Symbol>> vec_  = {};
   std::unordered_map<string_view, Symbol> map_ = {};

 public:
   /// nullptr for the global scope
   Scope* parent() noexcept { return parent_; }
   const Scope* parent() const noexcept { return parent_; }

   /// How many symbols are interned in this scope?
   size_t size() const noexcept;

   /// int a; ==> insert(a, int);
   void insert(string_view key, Symbol value) noexcept;

   /// Scope has the symbol
   bool has(string_view key) const noexcept;

   /// Same as `has`, but we search ancestor scopes
   bool has_recursive(string_view key) const noexcept;

   /// * Does the symbol exist in the table?
   /// * Is the symbol declared before use?
   /// * Is the symbol used within the scope?
   /// * Is the symbol initialized?
   /// * Is the symbol declared multiple times?
   const Symbol* lookup(string_view key) const noexcept;

   /// Same as `lookup`, but we search ancestor scopes
   const Symbol* lookup_recursive(string_view key) const noexcept;

   ///
   std::ostream& stream(std::ostream&) const noexcept;
};

} // namespace giraffe
