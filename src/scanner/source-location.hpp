
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

} // namespace giraffe
