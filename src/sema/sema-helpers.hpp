
#pragma once

namespace giraffe
{
// Pass reverse iterators if you want to search backwards
template<typename InputItr, std::invocable<ElementNode*> F>
void find_elem_set(InputItr bb, // beginning of range
                   InputItr ee, // end of range
                   F&& callback) noexcept
{
   for(auto ii = bb; ii != ee; ++ii) {
      ElementNode* elem = cast_ast_node<ElementNode>(*ii);
      if(elem->is_element_list()) {
         ElementListNode* elem_list = elem->element_list();
         if constexpr(is_reverse_iterator<InputItr>::value)
            find_elem_set(elem_list->rbegin(), elem_list->rend(), callback);
         else
            find_elem_set(elem_list->begin(), elem_list->end(), callback);
      } else {
         callback(elem);
      }
      if(!elem->is_optional()) break; // we're done
   }
}
} // namespace giraffe
