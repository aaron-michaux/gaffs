

#pragma once

namespace giraffe
{
template<typename Iter> struct is_reverse_iterator : std::false_type
{};

template<typename Iter>
struct is_reverse_iterator<std::reverse_iterator<Iter>>
    : std::integral_constant<bool, !is_reverse_iterator<Iter>::value>
{};

} // namespace giraffe
