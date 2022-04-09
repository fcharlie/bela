#ifndef BELA__STRINGS_STRINGS_CAT_HPP
#define BELA__STRINGS_STRINGS_CAT_HPP
#include <string>
#include <array>
#include <string_view>
#include <bela/types.hpp>

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



} // namespace bela

#endif