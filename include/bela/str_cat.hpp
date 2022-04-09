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
#include "__strings/string_cat_internal.hpp"

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

namespace bela {
template <typename CharT>
requires bela::character<CharT> size_t buffer_unchecked_concat(CharT *out, const basic_alphanum<CharT> &a) {
  if (a.size() != 0) {
    memcpy(out, a.data(), a.size() * sizeof(CharT));
  }
  return a.size();
}

namespace strings_internal {
  // Do not call directly - this is not part of the public API.
  std::wstring CatPieces(std::initializer_list<std::wstring_view> pieces);
  void AppendPieces(std::wstring * dest, std::initializer_list<std::wstring_view> pieces);
  std::string CatPieces(std::initializer_list<std::string_view> pieces);
  void AppendPieces(std::string * dest, std::initializer_list<std::string_view> pieces);

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

//
[[nodiscard]] inline std::string StringCatA() { return std::string(); }
[[nodiscard]] inline std::string StringCatA(const AlphaNumA &a) {
  //
  return std::string(a.data(), a.size());
}

[[nodiscard]] inline std::string StringCatA(const AlphaNumA &a, const AlphaNumA &b) {
  std::string result;
  result.resize(a.size() + b.size());
  auto out = result.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  return result;
}
[[nodiscard]] inline std::string StringCatA(const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c) {
  std::string result;
  result.resize(a.size() + b.size() + c.size());
  auto out = result.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
  return result;
}
[[nodiscard]] inline std::string StringCatA(const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c,
                                            const AlphaNumA &d) {
  std::string result;
  result.resize(a.size() + b.size() + c.size() + d.size());
  auto out = result.data();
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
  out += buffer_unchecked_concat(out, d);
  return result;
}

// Support 5 or more arguments
template <typename... AV>
[[nodiscard]] inline std::string StringCatA(const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c,
                                            const AlphaNumA &d, const AlphaNumA &e, const AV &...args) {
  return strings_internal::CatPieces(
      {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(), static_cast<const AlphaNumA &>(args).Piece()...});
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

// --------------
inline void StrAppend(std::string *) {}
inline void StrAppend(std::string *dest, const AlphaNumA &a) {
  dest->append(a.Piece()); //
}
inline void StrAppend(std::string *dest, const AlphaNumA &a, const AlphaNumA &b) {
  auto oldsize = dest->size();
  dest->resize(oldsize + a.size() + b.size());
  auto out = dest->data() + oldsize;
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
}

inline void StrAppend(std::string *dest, const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c) {
  auto oldsize = dest->size();
  dest->resize(oldsize + a.size() + b.size() + c.size());
  auto out = dest->data() + oldsize;
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
}

inline void StrAppend(std::string *dest, const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c,
                      const AlphaNumA &d) {
  auto oldsize = dest->size();
  dest->resize(oldsize + a.size() + b.size() + c.size() + d.size());
  auto out = dest->data() + oldsize;
  out += buffer_unchecked_concat(out, a);
  out += buffer_unchecked_concat(out, b);
  out += buffer_unchecked_concat(out, c);
  out += buffer_unchecked_concat(out, d);
}

// Support 5 or more arguments
template <typename... AV>
inline void StrAppend(std::string *dest, const AlphaNumA &a, const AlphaNumA &b, const AlphaNumA &c, const AlphaNumA &d,
                      const AlphaNumA &e, const AV &...args) {
  strings_internal::AppendPieces(
      dest, {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(), static_cast<const AlphaNumA &>(args).Piece()...});
}

} // namespace bela

#endif
