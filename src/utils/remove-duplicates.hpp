
#pragma once

namespace giraffe
{
template<typename Container> void remove_duplicates(Container& c) noexcept
{
   std::sort(begin(c), end(c));
   auto new_end = std::unique(begin(c), end(c));
   c.resize(size_t(std::distance(begin(c), new_end)));
}
} // namespace giraffe
