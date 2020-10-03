
#pragma once

namespace giraffe
{
struct SourceLocation final
{
   uint32_t offset    = 0; // offset within `key`
   uint32_t line_no   = 0; // line-number where starts on
   uint16_t column_no = 0; // column-no where source starts on
   uint16_t key       = 0; // key points to the raw text for source
};

// Finds the full line that 'offset' sits on, returning the
// start offset and the end offset. (End offset past the end of the line.)
std::pair<size_t, size_t> find_line(string_view text, size_t offset) noexcept;

} // namespace giraffe
