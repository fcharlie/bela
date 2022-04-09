/// TEST code don't call
#include <bela/str_cat.hpp>

namespace bela {


template <typename CharT = wchar_t, typename Allocater = std::allocator<CharT>>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater> strings_cat() {
  return std::basic_string<CharT, std::char_traits<CharT>, Allocater>();
}

template <typename CharT = wchar_t, typename Allocater = std::allocator<CharT>>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater>
strings_cat(const basic_alphanum<CharT> &a) {
  return std::basic_string<CharT, std::char_traits<CharT>, Allocater>(a.Piece());
}

template <typename CharT = wchar_t, typename Allocater = std::allocator<CharT>>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater>
strings_cat(const basic_alphanum<CharT> &a, const basic_alphanum<CharT> &b) {
  std::basic_string<CharT, std::char_traits<CharT>, Allocater> s;
  s.resize(a.size() + b.size());
  auto out = s.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  return s;
}

template <typename CharT = wchar_t, typename Allocater = std::allocator<CharT>>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater>
strings_cat(const basic_alphanum<CharT> &a, const basic_alphanum<CharT> &b, const basic_alphanum<CharT> &c) {
  std::basic_string<CharT, std::char_traits<CharT>, Allocater> s;
  s.resize(a.size() + b.size() + c.size());
  auto out = s.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
  return s;
}

template <typename CharT = wchar_t, typename Allocater = std::allocator<CharT>>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater>
strings_cat(const basic_alphanum<CharT> &a, const basic_alphanum<CharT> &b, const basic_alphanum<CharT> &c,
            const basic_alphanum<CharT> &d) {
  std::basic_string<CharT, std::char_traits<CharT>, Allocater> s;
  s.resize(a.size() + b.size() + c.size() + d.size());
  auto out = s.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
  out += buffer_unchecked_concat(out, d);
  return s;
}

template <typename CharT, typename Allocater, typename... AV>
requires bela::character<CharT>
[[nodiscard]] inline std::basic_string<CharT, std::char_traits<CharT>, Allocater>
strings_cat(const basic_alphanum<CharT> &a, const basic_alphanum<CharT> &b, const basic_alphanum<CharT> &c,
            const basic_alphanum<CharT> &d, const basic_alphanum<CharT> &e, const AV &...args) {
  const std::basic_string_view<CharT, std::char_traits<CharT>> pieces[] = {
      a.Piece(), b.Piece(), c.Piece(),
      d.Piece(), e.Piece(), static_cast<const basic_alphanum<CharT> &>(args).Piece()...};
  std::basic_string<CharT, std::char_traits<CharT>, Allocater> result;
  size_t total_size = 0;
  for (const auto piece : pieces) {
    total_size += piece.size();
  }
  result.resize(total_size);
  auto *const begin = &result[0];
  auto *out = begin;
  for (const auto piece : pieces) {
    const size_t this_size = piece.size();
    memcpy(out, piece.data(), this_size * sizeof(CharT));
    out += this_size;
  }
  assert(out == begin + result.size());
  return result;
}

} // namespace bela