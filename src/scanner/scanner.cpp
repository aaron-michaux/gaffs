
#include "scanner.hpp"

#include "scanner_bridge.h"

#include <stdio.h>

#define This Scanner

namespace giraffe
{
// ---------------------------------------------------------------------- Worker

struct This::Worker
{
 private:
   vector<string> data_; // The data to be lexed... memory is _STORED_ here
   void* scanner_             = nullptr;
   FILE* fp_                  = nullptr;
   size_t current_index_      = 0;
   string_view current_data_  = "";
   ScannerOptions opts_ = {};

   void cleanup_scanner_resources_() noexcept;
   void* get_scanner_() noexcept;

 public:
   Worker(vector<string>&& data, ScannerOptions opts)
       : data_(std::move(data))
       , opts_(opts)
   {
      assert(data_.size() < std::numeric_limits<uint16_t>::max());
   }
   ~Worker() { cleanup_scanner_resources_(); }

   Token produce_token() noexcept;
   const auto& data() const noexcept { return data_; }
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
   if(scanner_ != nullptr) return scanner_;           // We already have one!
   if(current_index_ >= data_.size()) return nullptr; // nothing to scan!
   current_data_ = data_.at(current_index_);
   assert(fp_ == nullptr);
   fp_ = fmemopen(
       const_cast<char*>(current_data_.data()), current_data_.size(), "rb");
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
   auto scanner = get_scanner_();
   while(scanner != nullptr) {
      assert(current_index_ < data_.size());
      const bool is_eof = (yylex(scanner) == TEOF);
      if(is_eof) {
         cleanup_scanner_resources_(); // delete the scanner
         return Token::make_start_token(uint16_t(current_index_++));
      } else {
         const auto token = Token(uint16_t(current_index_),
                                  current_data_,
                                  *get_scanner_params(scanner));
         if(is_token_of_interest(token, opts_)) return token;
         // otherwise loop around and get the next token
      }
   }
   return Token::make_eof_token();
}

// -----------------------------------------------------------------
// Constructor

static size_t estimate_n_tokens(const auto& data)
{
   // Make a guess at home many token's we'll need
   const size_t total_len = std::accumulate(
       cbegin(data), cend(data), size_t(0), [&](size_t x, const auto& s) {
          return x + s.size();
       });
   const size_t approx_tokens = total_len / 5;
   return approx_tokens;
}

This::This(string_view data, ScannerOptions opts) noexcept
    : worker_(
        make_unique<Worker>(vector{{string(cbegin(data), cend(data))}}, opts))
{
   tokens_.reserve(estimate_n_tokens(text_data()));
}

This::This(vector<string>&& data, ScannerOptions opts) noexcept
    : worker_(make_unique<Worker>(std::move(data), opts))
{
   tokens_.reserve(estimate_n_tokens(text_data()));
}

This::~This() = default;

const vector<string>& This::text_data() { return worker_->data(); }

bool This::has_next() noexcept { return !peek(1).is_eof(); }

auto This::next_token_id() noexcept { return peek(1).id(); }

const Token& This::consume() noexcept
{
   const auto& ret = peek(0); // get the current token
   position_       = ret.is_eof() ? (tokens_.size() - 1) : (position_ + 1);
   return ret;
}

const Token& This::peek(size_t n) noexcept
{
   size_t ultimate_n = position_ + n;
   while(ultimate_n >= tokens_.size()) {
      if(found_eof()) return tokens_.back();
      tokens_.push_back(worker_->produce_token());
   }
   return tokens_.at(ultimate_n);
}

} // namespace giraffe

#undef This
