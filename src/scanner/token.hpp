
#pragma once

#include "scanner.h"

#include "source-location.hpp"

namespace giraffe
{
struct Token final
{
 private:
   string_view text_   = ""; // the text of the token
   SourceLocation loc_ = {};
   uint16_t id_        = 0; // id of this token

 public:
   static constexpr Token make_start_token(uint16_t key) noexcept
   {
      return Token(TSTART, key);
   }
   static constexpr Token make_eof_token() noexcept { return Token(TEOF); }

   constexpr Token(uint16_t id = TNONE, uint16_t key = 0)
       : loc_({0, 0, 0, key})
       , id_(id)
   {}

   constexpr Token(uint16_t key,
                   string_view raw_data,
                   const scanner_params_t& params)
       : loc_({params.last_offset,
               params.last_line_no,
               params.last_column_no,
               key})
       , id_(params.token_id)
   {
      if(params.token_id != TEOF) {
         assert(offset() <= raw_data.size());
         assert(offset() + params.text_len <= raw_data.size());
         text_ = string_view(&raw_data[offset()], params.text_len);
         assert(memcmp(text_.data(), params.text, params.text_len) == 0);
      }
   }

   constexpr Token(const Token&) = default;
   constexpr Token(Token&&)      = default;
   constexpr ~Token()            = default;
   constexpr Token& operator=(const Token&) = default;
   constexpr Token& operator=(Token&&) = default;

   constexpr auto id() const noexcept { return id_; }
   constexpr auto text() const noexcept { return text_; }
   constexpr auto location() const noexcept { return loc_; }
   constexpr auto key() const noexcept { return loc_.key; }
   constexpr uint32_t offset() const noexcept { return loc_.offset; }
   constexpr auto line_no() const noexcept { return loc_.line_no; }
   constexpr auto column_no() const noexcept { return loc_.column_no; }
   constexpr auto end_offset() const noexcept
   {
      return offset() + text().length();
   }

   // -- Methods -- //

   constexpr bool is_none() const { return id() == TNONE; }
   constexpr bool is_start() const { return id() == TSTART; }
   constexpr bool is_eof() const { return id() == TEOF; }
   constexpr bool is_newline() const { return id() == TNEWLINE; }
   constexpr bool is_badchar() const { return id() == TBADCHAR; }

   string to_string() const
   {
      static constexpr auto k_newline_sv = string_view("{newline}");

      return format("{:19s} {:5d} {:5d} {:5d} '{}'",
                    token_id_to_str(id()),
                    line_no(),
                    column_no(),
                    offset(),
                    is_newline() ? k_newline_sv : text());
   }
};

} // namespace giraffe
