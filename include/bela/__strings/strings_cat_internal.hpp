#ifndef BELA__STRINGS_STRINGS_CAT_HPP
#define BELA__STRINGS_STRINGS_CAT_HPP
#include <string>
#include <array>
#include <string_view>
#include <bela/types.hpp>
#include <bela/numbers.hpp>
#include <bela/codecvt.hpp>

namespace bela {
namespace strings_internal {
template <size_t max_size, typename CharT = wchar_t>
requires bela::character<CharT>
struct AlphaNumBuffer {
  std::array<CharT, max_size> data;
  size_t size;
};
template <typename CharT>
requires bela::character<CharT> std::basic_string_view<CharT, std::char_traits<CharT>>
NullSafeStringView(const CharT *str) {
  using string_view_t = std::basic_string_view<CharT, std::char_traits<CharT>>;
  return (str == nullptr) ? string_view_t() : string_view_t(str);
}
} // namespace strings_internal
// Enum that specifies the number of significant digits to return in a `Hex` or
// `Dec` conversion and fill character to use. A `kZeroPad2` value, for example,
// would produce hexadecimal strings such as "0a","0f" and a 'kSpacePad5' value
// would produce hexadecimal strings such as "    a","    f".
enum PadSpec : uint8_t {
  kNoPad = 1,
  kZeroPad2,
  kZeroPad3,
  kZeroPad4,
  kZeroPad5,
  kZeroPad6,
  kZeroPad7,
  kZeroPad8,
  kZeroPad9,
  kZeroPad10,
  kZeroPad11,
  kZeroPad12,
  kZeroPad13,
  kZeroPad14,
  kZeroPad15,
  kZeroPad16,
  kZeroPad17,
  kZeroPad18,
  kZeroPad19,
  kZeroPad20,

  kSpacePad2 = kZeroPad2 + 64,
  kSpacePad3,
  kSpacePad4,
  kSpacePad5,
  kSpacePad6,
  kSpacePad7,
  kSpacePad8,
  kSpacePad9,
  kSpacePad10,
  kSpacePad11,
  kSpacePad12,
  kSpacePad13,
  kSpacePad14,
  kSpacePad15,
  kSpacePad16,
  kSpacePad17,
  kSpacePad18,
  kSpacePad19,
  kSpacePad20,
};

// -----------------------------------------------------------------------------
// Hex
// -----------------------------------------------------------------------------
//
// `Hex` stores a set of hexadecimal string conversion parameters for use
// within `AlphaNum` string conversions.
struct Hex {
  uint64_t value;
  uint8_t width;
  char fill;

  template <typename Int>
  requires(sizeof(Int) == 1 && !std::is_pointer_v<Int>) constexpr explicit Hex(Int v, PadSpec spec = kNoPad)
      : Hex(spec, static_cast<uint8_t>(v)) {}
  template <typename Int>
  requires(sizeof(Int) == 2 && !std::is_pointer_v<Int>) constexpr explicit Hex(Int v, PadSpec spec = kNoPad)
      : Hex(spec, static_cast<uint16_t>(v)) {}
  template <typename Int>
  requires(sizeof(Int) == 4 && !std::is_pointer_v<Int>) constexpr explicit Hex(Int v, PadSpec spec = kNoPad)
      : Hex(spec, static_cast<uint32_t>(v)) {}
  template <typename Int>
  requires(sizeof(Int) == 8 && !std::is_pointer_v<Int>) constexpr explicit Hex(Int v, PadSpec spec = kNoPad)
      : Hex(spec, static_cast<uint64_t>(v)) {}
  template <typename Pointer>
  requires std::is_pointer_v<Pointer>
  explicit Hex(Pointer *v, PadSpec spec = kNoPad) : Hex(spec, reinterpret_cast<uintptr_t>(v)) {}

private:
  constexpr Hex(PadSpec spec, uint64_t v)
      : value(v), width(spec == kNoPad       ? 1
                        : spec >= kSpacePad2 ? spec - kSpacePad2 + 2
                                             : spec - kZeroPad2 + 2),
        fill(spec >= kSpacePad2 ? ' ' : '0') {}
};

// -----------------------------------------------------------------------------
// Dec
// -----------------------------------------------------------------------------
//
// `Dec` stores a set of decimal string conversion parameters for use
// within `AlphaNum` string conversions.  Dec is slower than the default
// integer conversion, so use it only if you need padding.
struct Dec {
  uint64_t value;
  uint8_t width;
  char fill;
  bool neg;

  template <typename Int>
  requires std::integral<Int>
  constexpr explicit Dec(Int v, PadSpec spec = kNoPad)
      : value(v >= 0 ? static_cast<uint64_t>(v) : uint64_t{0} - static_cast<uint64_t>(v)),
        width(spec == kNoPad       ? 1
              : spec >= kSpacePad2 ? spec - kSpacePad2 + 2
                                   : spec - kZeroPad2 + 2),
        fill(spec >= kSpacePad2 ? ' ' : '0'), neg(v < 0) {}
};

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

} // namespace bela

#endif