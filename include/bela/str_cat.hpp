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
#include "numbers.hpp"
#include "codecvt.hpp"

namespace bela {

// has_native support
//  bela::error_code
//  std::filesystem::path
template <typename T>
concept has_native = requires(const T &a) {
  { a.native() } -> std::convertible_to<const std::wstring &>;
};

// -----------------------------------------------------------------------------
// AlphaNum
// -----------------------------------------------------------------------------
//
// The `AlphaNum` class acts as the main parameter type for `StringCat()` and
// `StrAppend()`, providing efficient conversion of numeric, boolean, and
// hexadecimal values (through the `Hex` type) into strings.
class AlphaNum {
public:
  // No bool ctor -- bools convert to an integral type.
  // A bool ctor would also convert incoming pointers (bletch).

  AlphaNum(int x) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned int x) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(long x) // NOLINT(*)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned long x) // NOLINT(*)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(long long x) // NOLINT(*)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned long long x) // NOLINT(*)
      : piece_(digits_, numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}

  AlphaNum(float f) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::SixDigitsToBuffer(f, digits_)) {}
  AlphaNum(double f) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::SixDigitsToBuffer(f, digits_)) {}

  AlphaNum(Hex hex); // NOLINT(runtime/explicit)
  AlphaNum(Dec dec); // NOLINT(runtime/explicit)

  template <size_t size>
  AlphaNum( // NOLINT(runtime/explicit)
      const strings_internal::AlphaNumBuffer<size> &buf)
      : piece_(&buf.data[0], buf.size) {}

  AlphaNum(const wchar_t *c_str) : piece_(c_str == nullptr ? L"" : c_str) {} // NOLINT(runtime/explicit)
  AlphaNum(std::wstring_view pc) : piece_(pc) {}                             // NOLINT(runtime/explicit)

  template <typename Allocator>
  AlphaNum( // NOLINT(runtime/explicit)
      const std::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator> &str)
      : piece_(str) {}
  /// WARNING wchar_t or char16_t must not  surrogate . U+D800~U+DFFF
  AlphaNum(wchar_t c) : piece_(digits_, 1) { digits_[0] = c; }
  AlphaNum(char16_t ch) : piece_(digits_, 1) { digits_[0] = static_cast<wchar_t>(ch); }
  AlphaNum(char32_t ch) : piece_(digits_, bela::encode_into_unchecked(ch, digits_)) {
    static_assert(sizeof(digits_) / sizeof(wchar_t) > bela::kMaxEncodedUTF16Size, "difits_ buffer is too small");
  }

  // Extended type support
  // eg: std::filesystem::path
  template <typename T>
  requires has_native<T> AlphaNum(const T &t) : piece_(t.native()) {}

  AlphaNum(const AlphaNum &) = delete;
  AlphaNum &operator=(const AlphaNum &) = delete;

  std::wstring_view::size_type size() const { return piece_.size(); }
  const wchar_t *data() const { return piece_.data(); }
  std::wstring_view Piece() const { return piece_; }

  // Normal enums are already handled by the integer formatters.
  // This overload matches only scoped enums.
  template <typename T>
  requires bela::strict_enum<T> AlphaNum(T e) // NOLINT(runtime/explicit)
      : AlphaNum(bela::integral_cast(e)) {}

  // vector<bool>::reference and const_reference require special help to
  // convert to `AlphaNum` because it requires two user defined conversions.
  template <typename T>
  requires(std::is_class_v<T> && (std::is_same_v<T, std::vector<bool>::reference> ||
                                  std::is_same_v<T, std::vector<bool>::const_reference>)) AlphaNum(T e)
      : AlphaNum(static_cast<bool>(e)) {} // NOLINT(runtime/explicit)

private:
  std::wstring_view piece_;
  wchar_t digits_[numbers_internal::kFastToBufferSize];
};

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
