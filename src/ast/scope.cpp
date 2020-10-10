
#include "scope.hpp"

#define This Scope

namespace giraffe
{
// ------------------------------------------------------------------- str(Type)

const char* str(Type type) noexcept
{
   switch(type) {
   case Type::NONE: return "none";
   case Type::RULE: return "rule";
   case Type::TOKEN: return "token";
   case Type::STRING: return "string";
   }
   assert(false);
   return "<unknown>";
}

// ----------------------------------------------------------------- make-symbol

Symbol make_rule_symbol(string_view label, AstNode* node) noexcept
{
   return {label, node, Type::RULE};
}

Symbol make_token_symbol(string_view label, AstNode* node) noexcept
{
   return {label, node, Type::TOKEN};
}

Symbol make_string_symbol(string_view label, AstNode* node) noexcept
{
   return {label, node, Type::STRING};
}

// ------------------------------------------------------------------------ size

size_t This::size() const noexcept
{
   return using_vec_ ? vec_.size() : map_.size();
}

// ---------------------------------------------------------------------- insert

void This::insert(string_view key, Symbol value) noexcept
{
   assert(lookup(key) == nullptr);
   if(using_vec_) {
      vec_.push_back({key, value});
      if(vec_.size() == map_min_size_) {
         assert(map_.size() == 0);
         map_.insert(cbegin(vec_), cend(vec_));
         using_vec_ = false;
         vec_.clear();
      }
   } else {
      map_.insert({key, value});
   }
}

// ------------------------------------------------------------------------- has

bool This::has(string_view key) const noexcept
{
   return lookup(key) != nullptr;
}

bool This::has_recursive(string_view key) const noexcept
{
   return lookup_recursive(key) != nullptr;
}

// ---------------------------------------------------------------------- lookup

const Symbol* This::lookup(string_view key) const noexcept
{
   if(using_vec_) {
      auto ii = std::find_if(cbegin(vec_), cend(vec_), [key](const auto& o) {
         return o.first == key;
      });
      return (ii == cend(vec_)) ? nullptr : &ii->second;
   } else {
      auto ii = map_.find(key);
      return (ii == cend(map_)) ? nullptr : &ii->second;
   }
}

const Symbol* This::lookup_recursive(string_view key) const noexcept
{
   auto symbol = lookup(key);
   if(symbol == nullptr && parent() != nullptr) return parent()->lookup(key);
   return symbol;
}

// ---------------------------------------------------------------------- stream

std::ostream& This::stream(std::ostream& ss) const noexcept
{
   auto f = [&ss](auto bb, auto ee) {
      vector<decltype(vec_)::value_type> sorted(bb, ee);
      std::sort(begin(sorted), end(sorted), [&](auto& A, auto& B) {
         if(A.second.type != B.second.type)
            return A.second.type < B.second.type;
         return A.second.label < B.second.label;
      });

      constexpr size_t align = 7;
      for(const auto& ii : sorted) {
         const auto& symbol = ii.second;

         const string_view type_s = str(symbol.type);
         assert(type_s.size() <= align);
         const auto n_spaces = align - type_s.size();
         ss << "   " << str(symbol.type) << ": ";
         for(size_t i = 0; i < n_spaces; ++i) ss << ' ';
         ss << symbol.label << '\n';
      }
   };

   ss << "Symbols:\n";
   if(using_vec_)
      f(cbegin(vec_), cend(vec_));
   else
      f(cbegin(map_), cend(map_));

   return ss;
}

} // namespace giraffe

#undef This
