//////////
#ifndef BELA_CODECVT_HPP
#define BELA_CODECVT_HPP
#include <string>
#include <vector>

namespace bela {
std::string c16tomb(const char16_t *data, size_t len, bool skipillegal = false);
std::wstring mbrtowc(const unsigned char *str, size_t len,
                     bool skipillegal = false);
std::u16string mbrtoc16(const unsigned char *str, size_t len,
                        bool skipillegal = false);
//// Narrow wchar_t std::wstring_view
inline std::string ToNarrow(std::wstring_view uw) {
  return c16tomb(reinterpret_cast<const char16_t *>(uw.data()), uw.size());
}
inline std::string ToNarrow(const wchar_t *data, size_t len) {
  return c16tomb(reinterpret_cast<const char16_t *>(data), len);
}
inline std::string ToNarrow(std::u16string_view uw) {
  return c16tomb(uw.data(), uw.size());
}
inline std::string ToNarrow(const char16_t *data, size_t len) {
  return c16tomb(data, len);
}

inline std::wstring ToWide(const char *str, size_t len) {
  return mbrtowc(reinterpret_cast<const unsigned char *>(str), len);
}

inline std::wstring ToWide(std::string_view sv) {
  return mbrtowc(reinterpret_cast<const unsigned char *>(sv.data()), sv.size());
}
} // namespace bela

#endif
