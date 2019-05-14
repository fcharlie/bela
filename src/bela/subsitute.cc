///
#include <bela/subsitute.hpp>

namespace bela {
namespace substitute_internal {
void SubstituteAndAppendArray(std::wstring *output, std::wstring_view format,
                              const std::wstring_view *args_array,
                              size_t num_args) {
  size_t size = 0;
  for (size_t i = 0; i < format.size(); i++) {
    if (format[i] == L'$') {
      if (i + 1 >= format.size()) {
        return;
      }
      if (ascii_isdigit(format[i + 1])) {
        int index = format[i + 1] - L'0';
        if (static_cast<size_t>(index) >= num_args) {
          return;
        }
        size += args_array[index].size();
        ++i; // Skip next char.
      } else if (format[i + 1] == '$') {
        ++size;
        ++i; // Skip next char.
      } else {
        return;
      }
    } else {
      ++size;
    }
  }

  if (size == 0)
    return;

  // Build the std::string.
  size_t original_size = output->size();
  output->resize(original_size + size);
  wchar_t *target = &(*output)[original_size];
  for (size_t i = 0; i < format.size(); i++) {
    if (format[i] == '$') {
      if (ascii_isdigit(format[i + 1])) {
        const std::wstring_view src = args_array[format[i + 1] - '0'];
        target = std::copy(src.begin(), src.end(), target);
        ++i; // Skip next char.
      } else if (format[i + 1] == '$') {
        *target++ = '$';
        ++i; // Skip next char.
      }
    } else {
      *target++ = format[i];
    }
  }
}
} // namespace substitute_internal
} // namespace bela