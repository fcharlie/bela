// ---------------------------------------------------------------------------
// Copyright (C) 2022, Bela contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Includes work from abseil-cpp (https://github.com/abseil/abseil-cpp)
// with modifications.
//
// Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ---------------------------------------------------------------------------
#ifndef BELA_STRCAT_HPP
#define BELA_STRCAT_HPP
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "__strings/strings_cat_internal.hpp"

namespace bela {

// -----------------------------------------------------------------------------
// StringCat()
// -----------------------------------------------------------------------------
//
// Merges given strings or numbers, using no delimiter(s).
//
// `StringCat()` is designed to be the fastest possible way to construct a
// string out of a mix of raw C strings, string_views, strings, bool values, and
// numeric values.
//
// Don't use `StringCat()` for user-visible strings. The localization process
// works poorly on strings built up out of fragments.
//
// For clarity and performance, don't use `StringCat()` when appending to a
// string. Use `StrAppend()` instead. In particular, avoid using any of these
// (anti-)patterns:
//
//   str.append(StringCat(...))
//   str += StringCat(...)
//   str = StringCat(str, ...)
//
// The last case is the worst, with a potential to change a loop
// from a linear time operation with O(1) dynamic allocations into a
// quadratic time operation with O(n) dynamic allocations.
//
// See `StrAppend()` below for more information.

namespace strings_internal {

// Do not call directly - this is not part of the public API.
std::wstring CatPieces(std::initializer_list<std::wstring_view> pieces);
void AppendPieces(std::wstring *dest, std::initializer_list<std::wstring_view> pieces);

} // namespace strings_internal

[[nodiscard]] inline std::wstring StringCat() { return std::wstring(); }

[[nodiscard]] inline std::wstring StringCat(const AlphaNum &a) { return std::wstring(a.data(), a.size()); }

[[nodiscard]] std::wstring StringCat(const AlphaNum &a, const AlphaNum &b);
[[nodiscard]] std::wstring StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c);
[[nodiscard]] std::wstring StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d);

// Support 5 or more arguments
template <typename... AV>
[[nodiscard]] inline std::wstring StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d,
                                            const AlphaNum &e, const AV &...args) {
  return strings_internal::CatPieces(
      {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(), static_cast<const AlphaNum &>(args).Piece()...});
}

// -----------------------------------------------------------------------------
// StrAppend()
// -----------------------------------------------------------------------------
//
// Appends a string or set of strings to an existing string, in a similar
// fashion to `StringCat()`.
//
// WARNING: `StrAppend(&str, a, b, c, ...)` requires that none of the
// a, b, c, parameters be a reference into str. For speed, `StrAppend()` does
// not try to check each of its input arguments to be sure that they are not
// a subset of the string being appended to. That is, while this will work:
//
//   std::string s = "foo";
//   s += s;
//
// This output is undefined:
//
//   std::string s = "foo";
//   StrAppend(&s, s);
//
// This output is undefined as well, since `absl::string_view` does not own its
// data:
//
//   std::string s = "foobar";
//   absl::string_view p = s;
//   StrAppend(&s, p);

inline void StrAppend(std::wstring *) {}
void StrAppend(std::wstring *dest, const AlphaNum &a);
void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b);
void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b, const AlphaNum &c);
void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d);

// Support 5 or more arguments
template <typename... AV>
inline void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d,
                      const AlphaNum &e, const AV &...args) {
  strings_internal::AppendPieces(
      dest, {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(), static_cast<const AlphaNum &>(args).Piece()...});
}

// Helper function for the future StringCat default floating-point format, %.6g
// This is fast.
inline strings_internal::AlphaNumBuffer<numbers_internal::kSixDigitsToBufferSize> SixDigits(double d) {
  strings_internal::AlphaNumBuffer<numbers_internal::kSixDigitsToBufferSize> result{0};
  result.size = numbers_internal::SixDigitsToBuffer(d, &result.data[0]);
  return result;
}

} // namespace bela

#endif
