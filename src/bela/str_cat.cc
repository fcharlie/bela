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
#include <cassert>
#include <bela/strings.hpp>
#include <bela/str_cat.hpp>
#include <bela/endian.hpp>
#include <bela/macros.hpp>

namespace bela {

namespace strings_internal {
// clang-format off
template <class _CharT> inline constexpr _CharT kHexTable[] = {_CharT{}};
template <>
inline constexpr char kHexTable<char>[513] = 
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
template <>
inline constexpr wchar_t kHexTable<wchar_t>[513] = 
   L"000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
// clang-format on

// FastHexToBufferZeroPad16()
//
// Outputs `val` into `out` as if by `snprintf(out, 17, "%016x", val)` but
// without the terminating null character. Thus `out` must be of length >= 16.
// Returns the number of non-pad digits of the output (it can never be zero
// since 0 has one digit).
inline size_t FastHexToBufferZeroPad16(uint64_t val, char *out) {
#ifdef BELA_INTERNAL_HAVE_SSSE3
  uint64_t be = bela::htons(val);
  const auto kNibbleMask = _mm_set1_epi8(0xf);
  const auto kHexDigits = _mm_setr_epi8('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f');
  auto v = _mm_loadl_epi64(reinterpret_cast<__m128i *>(&be)); // load lo dword
  auto v4 = _mm_srli_epi64(v, 4);                             // shift 4 right
  auto il = _mm_unpacklo_epi8(v4, v);                         // interleave bytes
  auto m = _mm_and_si128(il, kNibbleMask);                    // mask out nibbles
  auto hexchars = _mm_shuffle_epi8(kHexDigits, m);            // hex chars
  _mm_storeu_si128(reinterpret_cast<__m128i *>(out), hexchars);
#else
  for (int i = 0; i < 8; ++i) {
    auto byte = (val >> (56 - 8 * i)) & 0xFF;
    auto *hex = &strings_internal::kHexTable<char>[byte * 2];
    std::memcpy(out + 2 * i, hex, 2);
  }
#endif
  // | 0x1 so that even 0 has 1 digit.
  return 16 - std::countl_zero(val | 0x1) / 4;
}

inline size_t FastHexToBufferZeroPad16(uint64_t val, char8_t *out) {
  return FastHexToBufferZeroPad16(val, reinterpret_cast<char *>(out));
}

inline size_t FastHexToBufferZeroPad16(uint64_t val, wchar_t *out) {
  for (int i = 0; i < 8; ++i) {
    auto byte = (val >> (56 - 8 * i)) & 0xFF;
    auto *hex = &strings_internal::kHexTable<wchar_t>[byte * 2];
    std::memcpy(out + 2 * i, hex, 2 * sizeof(wchar_t));
  }
  // | 0x1 so that even 0 has 1 digit.
  return 16 - std::countl_zero(val | 0x1) / 4;
}
inline size_t FastHexToBufferZeroPad16(uint64_t val, char16_t *out) {
  return FastHexToBufferZeroPad16(val, reinterpret_cast<wchar_t *>(out));
}

// Do not call directly - these are not part of the public API.
template <typename CharT>
requires bela::character<CharT>
auto CatPieces(std::initializer_list<std::basic_string_view<CharT, std::char_traits<CharT>>> pieces) {
  std::basic_string<CharT, std::char_traits<CharT>> result;
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
std::wstring CatPieces(std::initializer_list<std::wstring_view> pieces) { return CatPieces<wchar_t>(pieces); }
std::string CatPieces(std::initializer_list<std::string_view> pieces) { return CatPieces<char>(pieces); }

template <typename CharT>
requires bela::character<CharT>
void AppendPieces(std::basic_string<CharT, std::char_traits<CharT>> *dest,
                  std::initializer_list<std::basic_string_view<CharT, std::char_traits<CharT>>> pieces) {
  size_t old_size = dest->size();
  size_t total_size = old_size;
  for (const auto piece : pieces) {
    total_size += piece.size();
  }
  dest->resize(total_size);

  auto *const begin = &(*dest)[0];
  auto *out = begin + old_size;
  for (const auto piece : pieces) {
    const size_t this_size = piece.size();
    memcpy(out, piece.data(), this_size * sizeof(CharT));
    out += this_size;
  }
  assert(out == begin + dest->size());
}

void AppendPieces(std::wstring *dest, std::initializer_list<std::wstring_view> pieces) {
  AppendPieces<wchar_t>(dest, pieces);
}
void AppendPieces(std::string *dest, std::initializer_list<std::string_view> pieces) {
  AppendPieces<char>(dest, pieces);
}

} // namespace strings_internal

template <typename CharT>
requires bela::character<CharT>
auto make_hex_string_view(Hex hex, CharT *end) {
  using string_view_t = std::basic_string_view<CharT, std::char_traits<CharT>>;
  auto real_width = strings_internal::FastHexToBufferZeroPad16(hex.value, end - 16);
  if (real_width >= hex.width) {
    return string_view_t{end - real_width, real_width};
  }
  // Pad first 16 chars because FastHexToBufferZeroPad16 pads only to 16 and
  // max pad width can be up to 20.
  std::fill_n(end - 32, 16, hex.fill);
  // Patch up everything else up to the real_width.
  std::fill_n(end - real_width - 16, 16, hex.fill);
  return string_view_t{end - hex.width, hex.width};
}

template <typename CharT>
requires bela::character<CharT>
auto make_dec_string_view(Dec dec, CharT *end) {
  using string_view_t = std::basic_string_view<CharT, std::char_traits<CharT>>;
  CharT *const minfill = end - dec.width;
  CharT *writer = end;
  uint64_t value = dec.value;
  bool neg = dec.neg;
  while (value > 9) {
    *--writer = '0' + (value % 10);
    value /= 10;
  }
  *--writer = static_cast<CharT>('0' + value);
  if (neg) {
    *--writer = '-';
  }

  ptrdiff_t fillers = writer - minfill;
  if (fillers > 0) {
    // Tricky: if the fill character is ' ', then it's <fill><+/-><digits>
    // But...: if the fill character is '0', then it's <+/-><fill><digits>
    bool add_sign_again = false;
    if (neg && dec.fill == '0') { // If filling with '0',
      ++writer;                   // ignore the sign we just added
      add_sign_again = true;      // and re-add the sign later.
    }
    writer -= fillers;
    std::fill_n(writer, fillers, dec.fill);
    if (add_sign_again) {
      *--writer = '-';
    }
  }
  return string_view_t{writer, static_cast<size_t>(end - writer)};
}

// char8_t
template <> basic_alphanum<char8_t>::basic_alphanum(Hex hex) {
  piece_ = make_hex_string_view<char8_t>(hex, digits_ + std::size(digits_));
}
template <> basic_alphanum<char8_t>::basic_alphanum(Dec dec) {
  piece_ = make_dec_string_view<char8_t>(dec, digits_ + std::size(digits_));
}

// char
template <> basic_alphanum<char>::basic_alphanum(Hex hex) {
  piece_ = make_hex_string_view<char>(hex, digits_ + std::size(digits_));
}
template <> basic_alphanum<char>::basic_alphanum(Dec dec) {
  piece_ = make_dec_string_view<char>(dec, digits_ + std::size(digits_));
}

// wchar_t
template <> basic_alphanum<wchar_t>::basic_alphanum(Hex hex) {
  piece_ = make_hex_string_view<wchar_t>(hex, digits_ + std::size(digits_));
}
template <> basic_alphanum<wchar_t>::basic_alphanum(Dec dec) {
  piece_ = make_dec_string_view<wchar_t>(dec, digits_ + std::size(digits_));
}

// char16_t
template <> basic_alphanum<char16_t>::basic_alphanum(Hex hex) {
  piece_ = make_hex_string_view<char16_t>(hex, digits_ + std::size(digits_));
}
template <> basic_alphanum<char16_t>::basic_alphanum(Dec dec) {
  piece_ = make_dec_string_view<char16_t>(dec, digits_ + std::size(digits_));
}

// ----------------------------------------------------------------------
// StringCat()
//    This merges the given strings or integers, with no delimiter. This
//    is designed to be the fastest possible way to construct a string out
//    of a mix of raw C strings, string_views, strings, and integer values.
// ----------------------------------------------------------------------

// Append is merely a version of memcpy that returns the address of the byte
// after the area just overwritten.
static wchar_t *Append(wchar_t *out, const AlphaNum &x) {
  // memcpy is allowed to overwrite arbitrary memory, so doing this after the
  // call would force an extra fetch of x.size().
  wchar_t *after = out + x.size();
  MemCopy(out, x.data(), x.size());
  return after;
}

std::wstring StringCat(const AlphaNum &a, const AlphaNum &b) {
  std::wstring result;
  result.resize(a.size() + b.size());
  wchar_t *const begin = &result[0];
  wchar_t *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  assert(out == begin + result.size());
  return result;
}

std::wstring StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c) {
  std::wstring result;
  result.resize(a.size() + b.size() + c.size());
  wchar_t *const begin = &result[0];
  wchar_t *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  assert(out == begin + result.size());
  return result;
}

std::wstring StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d) {
  std::wstring result;
  result.resize(a.size() + b.size() + c.size() + d.size());
  wchar_t *const begin = &result[0];
  wchar_t *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  out = Append(out, d);
  assert(out == begin + result.size());
  return result;
}

namespace strings_internal {

// It's possible to call StrAppend with an std::wstring_view that is itself a
// fragment of the string we're appending to.  However the results of this are
// random. Therefore, check for this in debug mode.  Use unsigned math so we
// only have to do one comparison. Note, there's an exception case: appending an
// empty string is always allowed.
#define ASSERT_NO_OVERLAP(dest, src)                                                                                   \
  assert(((src).size() == 0) || (uintptr_t((src).data() - (dest).data()) > uintptr_t((dest).size())))

} // namespace strings_internal

void StrAppend(std::wstring *dest, const AlphaNum &a) {
  ASSERT_NO_OVERLAP(*dest, a);
  dest->append(a.data(), a.size());
}

void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  std::string::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size());
  wchar_t *const begin = &(*dest)[0];
  wchar_t *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  assert(out == begin + dest->size());
}

void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b, const AlphaNum &c) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  ASSERT_NO_OVERLAP(*dest, c);
  std::wstring::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size());
  wchar_t *const begin = &(*dest)[0];
  wchar_t *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  assert(out == begin + dest->size());
}

void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b, const AlphaNum &c, const AlphaNum &d) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  ASSERT_NO_OVERLAP(*dest, c);
  ASSERT_NO_OVERLAP(*dest, d);
  std::wstring::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size() + d.size());
  wchar_t *const begin = &(*dest)[0];
  wchar_t *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  out = Append(out, d);
  assert(out == begin + dest->size());
}

} // namespace bela
