
#pragma once

#include "scanner-options.hpp"
#include "token.hpp"

namespace giraffe
{
class Scanner final
{
 private:
   size_t position_ = 0; //!< The position in the token stack

   struct Worker;
   unique_ptr<Worker> worker_;

 public:
   /// Initialize the producer. Text memory is STORED here.
   /// Tokens don't store text
   Scanner();
   Scanner(string_view text_data, ScannerOptions opts = {}) noexcept;
   Scanner(const Scanner&) = delete;
   Scanner(Scanner&&) noexcept;
   ~Scanner();
   Scanner& operator=(const Scanner&) = delete;
   Scanner& operator                  =(Scanner&&) noexcept;

   /// Initialize from a single text... memory not stored here
   void initialize(string_view text_data, ScannerOptions opts = {}) noexcept;

   /// Initialize from a set of texts... possibly lazily loaded, memory not
   /// stored here
   void initialize(size_t n_texts,
                   std::function<string_view(size_t index)> get_text_fun,
                   ScannerOptions opts = {}) noexcept;

   /// Returns the buffer for the current token
   string_view current_buffer() const noexcept;

   /// EOF is appened when the end of all `text-data` is processed
   bool found_eof() const noexcept;

   /// Index to the current token
   size_t position() const noexcept { return position_; }

   /// TRUE if the next token exists, and it isn't EOF
   bool has_next() noexcept;

   /// Skip to a specific token position
   void set_position(size_t new_position) noexcept;

   /// Consume the current token, and move to the next
   const Token& consume() noexcept;

   /// Get the token-id of the NEXT token
   auto next_token_id() noexcept;

   /// Look ahead `n` tokens, and return it, _without_ changing current position
   const Token& peek(size_t n = 1) noexcept;

   /// The current token
   const Token& current() noexcept { return peek(0); }
};

} // namespace giraffe
