
#include "scanner.hpp"

#include "scanner-bridge.h"
#include "source-location.hpp"

#include <cstdio>
#include <deque>

#define This Scanner

namespace giraffe
{
// ---------------------------------------------------------------------- Worker

struct Scanner::Worker
{
 private:
   size_t n_texts_                                    = 0;
   std::function<string_view(size_t index)> get_text_ = nullptr;

   void* scanner_              = nullptr;
   FILE* fp_                   = nullptr;
   size_t current_index_       = size_t(-1);
   string_view current_buffer_ = "";
   ScannerOptions opts_        = {};

   std::deque<Token> tokens_ = {};

   void cleanup_scanner_resources_() noexcept;
   void* get_scanner_() noexcept;

 public:
   Worker(size_t n_texts,
          std::function<string_view(size_t index)> get_text_fun,
          ScannerOptions opts)
       : n_texts_(n_texts)
       , get_text_(get_text_fun)
       , opts_(opts)
   {
      SourceLocation loc;
      assert(n_texts <= std::numeric_limits<decltype(loc.key)>::max());
   }
   ~Worker()
   {
      cleanup_scanner_resources_();
      TRACE("DELETE");
   }

   Token produce_token() noexcept;
   auto& tokens() noexcept { return tokens_; }
   string_view current_buffer() const noexcept { return current_buffer_; }
};

void This::Worker::cleanup_scanner_resources_() noexcept
{
   if(scanner_ != nullptr) destroy_scanner(scanner_);
   if(fp_ != nullptr) fclose(fp_);
   fp_      = nullptr;
   scanner_ = nullptr;
}

void* This::Worker::get_scanner_() noexcept
{
   if(scanner_ != nullptr) return scanner_; // We already have one!

   // Increment to the next text input
   current_index_ = (current_index_ == size_t(-1)) ? 0 : current_index_ + 1;

   // There's nothing to scan!
   if(current_index_ >= n_texts_) return nullptr;

   // Update the current buffer
   current_buffer_ = get_text_(current_index_);
   assert(fp_ == nullptr);
   fp_ = fmemopen(
       const_cast<char*>(current_buffer_.data()), current_buffer_.size(), "rb");
   scanner_ = init_scanner(fp_);
   return scanner_;
}

static bool is_token_of_interest(const Token& token,
                                 const ScannerOptions& opts) noexcept
{
   if(opts.skip_comments && token.id() == TCOMMENT) return false;
   if(opts.skip_whitespace
      && (token.id() == TWHITESPACE || token.id() == TNEWLINE))
      return false;
   return true;
}

Token This::Worker::produce_token() noexcept
{
   while(true) {
      if(scanner_ == nullptr) {
         if(get_scanner_() == nullptr)
            return Token::make_eof_token();
         else
            return Token::make_start_token(uint16_t(current_index_));
      }

      assert(scanner_ != nullptr);
      assert(current_index_ <= n_texts_);

      // lex the token
      const bool is_eof = (yylex(scanner_) == TEOF);
      if(is_eof) {
         cleanup_scanner_resources_(); // delete the scanner
         assert(scanner_ == nullptr);
         continue; // ends in a TSTART or TEOF token
      } else {
         // create the token
         const auto token = Token(uint16_t(current_index_),
                                  current_buffer_,
                                  *get_scanner_params(scanner_));

         // if token is of interest (e.g., not a TCOMMENT), then return it
         if(is_token_of_interest(token, opts_)) return token;

         // otherwise loop around and get the next token
      }
   }

   assert(false); // should never reach this line of code
   return Token::make_eof_token();
}

// ----------------------------------------------------------------- Constructor

This::This()
{
   initialize(0, [](size_t) { return string_view(""); });
}

This::This(string_view data, ScannerOptions opts) noexcept
{
   initialize(data, opts);
}

This::This(This&&) noexcept = default;

This::~This() = default;

This& This::operator=(This&&) noexcept = default;

// ------------------------------------------------------------------ initialize

/// Initialize from a single text... memory not stored here
void This::initialize(string_view data, ScannerOptions opts) noexcept
{
   initialize(
       1, [data](size_t ind) { return data; }, opts);
}

/// Initialize from a set of texts... possibly lazily loaded, memory not
/// stored here
void This::initialize(size_t n_texts,
                      std::function<string_view(size_t index)> get_text_fun,
                      ScannerOptions opts) noexcept
{
   position_ = 0;
   worker_.release();
   worker_ = make_unique<Worker>(n_texts, get_text_fun, opts);
}

// -------------------------------------------------------------- current buffer

string_view This::current_buffer() const noexcept
{
   return worker_->current_buffer();
}

// -----------------------------------------------------------------------------

bool This::found_eof() const noexcept
{
   return worker_->tokens().size() > 0 && worker_->tokens().back().is_eof();
}

bool This::has_next() noexcept { return !peek(0).is_eof(); }

void This::set_position(size_t new_position) noexcept
{
   position_ = new_position;
}

uint16_t This::next_token_id() noexcept { return peek(1).id(); }

const Token& This::consume() noexcept
{
   const auto& ret = peek(0); // get the current token
   position_ = ret.is_eof() ? (worker_->tokens().size() - 1) : (position_ + 1);
   return ret;
}

const Token& This::peek(size_t n) noexcept
{
   size_t ultimate_n = position_ + n;
   while(ultimate_n >= worker_->tokens().size()) {
      if(found_eof()) return worker_->tokens().back();
      worker_->tokens().push_back(worker_->produce_token());
   }
   return worker_->tokens().at(ultimate_n);
}

} // namespace giraffe

#undef This
