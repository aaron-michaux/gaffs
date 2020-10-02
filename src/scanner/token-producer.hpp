
#pragma once

#include "token.hpp"

namespace giraffe
{
struct TokenProducerOptions final
{
   bool skip_whitespace = true;
   bool skip_comments   = true;
};

struct TokenProducer final
{
 private:
   vector<Token> tokens_ = {};
   size_t position_      = 0; //!< The position in the token stack

   struct Worker;
   unique_ptr<Worker> worker_;

 public:
   /// Initialize the producer. Text memory is STORED here.
   /// Tokens don't store text
   TokenProducer(string_view text_data,
                 TokenProducerOptions opts = {}) noexcept;
   TokenProducer(vector<string>&& text_data,
                 TokenProducerOptions opts = {}) noexcept;
   TokenProducer(const TokenProducer&) = delete;
   TokenProducer(TokenProducer&&)      = default;
   ~TokenProducer();
   TokenProducer& operator=(const TokenProducer&) = delete;
   TokenProducer& operator=(TokenProducer&&) = default;

   /// The underlying text data
   const vector<string>& text_data();

   /// EOF is appened when the end of all `text-data` is processed
   bool found_eof() const noexcept
   {
      return tokens_.size() > 0 && tokens_.back().is_eof();
   }

   /// Index to the current token
   size_t position() const noexcept { return position_; }

   /// TRUE if the next token exists, and it isn't EOF
   bool has_next() noexcept;

   /// Skip to a specific token position
   void set_position(size_t new_position) noexcept { position_ = new_position; }

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
