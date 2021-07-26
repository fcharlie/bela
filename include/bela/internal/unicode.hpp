///
#ifndef BELA_INTERNAL_HPP
#define BELA_INTERNAL_HPP
#include <string_view>
#include <span>
#include <bela/types.hpp>

namespace bela::unicode {
constexpr bool rune_is_surrogate(char32_t rune) { return (rune >= 0xD800 && rune <= 0xDFFF); }
template <typename CharT = char8_t>
requires bela::narrow_character<CharT>
[[nodiscard]] constexpr std::basic_string_view<CharT, std::char_traits<CharT>> encode_into(char32_t rune, CharT *dest,
                                                                                           size_t len) {
  using string_view_t = std::basic_string_view<CharT, std::char_traits<CharT>>;
  if (rune <= 0x7F) {
    if (len == 0) {
      return string_view_t();
    }
    dest[0] = static_cast<CharT>(rune);
    return string_view_t(dest, 1);
  }
  if (rune <= 0x7FF) {
    if (len < 2) {
      return string_view_t();
    }
    dest[0] = static_cast<CharT>(0xC0 | ((rune >> 6) & 0x1F));
    dest[1] = static_cast<CharT>(0x80 | (rune & 0x3F));
    return string_view_t(dest, 2);
  }
  if (rune <= 0xFFFF) {
    if (len < 3) {
      return string_view_t();
    }
    dest[0] = static_cast<CharT>(0xE0 | ((rune >> 12) & 0x0F));
    dest[1] = static_cast<CharT>(0x80 | ((rune >> 6) & 0x3F));
    dest[2] = static_cast<CharT>(0x80 | (rune & 0x3F));
    return string_view_t(dest, 3);
  }
  if (rune <= 0x10FFFF && len >= 4) {
    dest[0] = static_cast<CharT>(0xF0 | ((rune >> 18) & 0x07));
    dest[1] = static_cast<CharT>(0x80 | ((rune >> 12) & 0x3F));
    dest[2] = static_cast<CharT>(0x80 | ((rune >> 6) & 0x3F));
    dest[3] = static_cast<CharT>(0x80 | (rune & 0x3F));
    return string_view_t(dest, 4);
  }
  return string_view_t();
}

template <typename CharT = wchar_t>
requires bela::wide_character<CharT>
[[nodiscard]] constexpr std::basic_string_view<CharT, std::char_traits<CharT>> encode_into(char32_t rune, CharT *dest,
                                                                                           size_t len) {
  using string_view_t = std::basic_string_view<CharT, std::char_traits<CharT>>;
  if (len == 0) {
    return string_view_t();
  }
  if (rune <= 0xFFFF) {
    dest[0] = rune_is_surrogate(rune) ? 0xFFFD : static_cast<CharT>(rune);
    return string_view_t(dest, 1);
  }
  if (rune > 0x0010FFFF) {
    dest[0] = 0xFFFD;
    return string_view_t(dest, 1);
  }
  if (len < 2) {
    return string_view_t();
  }
  dest[0] = static_cast<CharT>(0xD7C0 + (rune >> 10));
  dest[1] = static_cast<CharT>(0xDC00 + (rune & 0x3FF));
  return string_view_t(dest, 2);
}

template <typename CharT = char8_t, size_t N>
requires bela::character<CharT>
[[nodiscard]] constexpr std::basic_string_view<CharT, std::char_traits<CharT>> encode_into(char32_t rune,
                                                                                           CharT (&dest)[N]) {
  return encode_into<CharT>(rune, dest, N);
}

} // namespace bela::unicode

#endif