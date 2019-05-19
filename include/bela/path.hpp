///
#ifndef BELA_WIN_PATH_HPP
#define BELA_WIN_PATH_HPP
#pragma once
#include "strcat.hpp"
#include "span.hpp"
#include <string_view>

namespace bela {
// Windows Path base
constexpr const wchar_t PathSeparator = L'\\';
constexpr const wchar_t PathUnixSeparator = L'/';
constexpr const size_t PathMax = 0x8000;
inline bool IsPathSeparator(wchar_t c) {
  return c == PathSeparator || c == PathUnixSeparator;
}

namespace path_internal {
std::wstring PathCatPieces(bela::Span<std::wstring_view> pieces);
} // namespace path_internal

[[nodiscard]] inline std::wstring PathCat(const AlphaNum &a) {
  std::wstring_view pv[] = {a.Piece()};
  return path_internal::PathCatPieces(pv);
}

[[nodiscard]] inline std::wstring PathCat(const AlphaNum &a,
                                          const AlphaNum &b) {
  std::wstring_view pv[] = {a.Piece(), b.Piece()};
  return path_internal::PathCatPieces(pv);
}

[[nodiscard]] inline std::wstring PathCat(const AlphaNum &a, const AlphaNum &b,
                                          const AlphaNum &c) {
  std::wstring_view pv[] = {a.Piece(), b.Piece(), c.Piece()};
  return path_internal::PathCatPieces(pv);
}

[[nodiscard]] inline std::wstring PathCat(const AlphaNum &a, const AlphaNum &b,
                                          const AlphaNum &c,
                                          const AlphaNum &d) {
  std::wstring_view pv[] = {a.Piece(), b.Piece(), c.Piece(), d.Piece()};
  return path_internal::PathCatPieces(pv);
}

// Support 5 or more arguments
template <typename... AV>
[[nodiscard]] inline std::wstring
PathCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c,
        const AlphaNum &d, const AlphaNum &e, const AV &... args) {
  return path_internal::PathCatPieces(
      {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(),
       static_cast<const AlphaNum &>(args).Piece()...});
}

// std::wstring_view ::data() must Null-terminated string
bool PathExists(std::wstring_view src);

} // namespace bela

#endif