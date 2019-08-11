//////////
// see:
// https://github.com/llvm-mirror/llvm/blob/master/lib/Support/ConvertUTF.cpp
//
#include <bela/codecvt.hpp>
#include "unicodewidth.hpp"
/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (char32_t)0x0000FFFD
#define UNI_MAX_BMP (char32_t)0x0000FFFF
#define UNI_MAX_UTF16 (char32_t)0x0010FFFF
#define UNI_MAX_UTF32 (char32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (char32_t)0x0010FFFF

#define UNI_MAX_UTF8_BYTES_PER_CODE_POINT 4

#define UNI_UTF16_BYTE_ORDER_MARK_NATIVE 0xFEFF
#define UNI_UTF16_BYTE_ORDER_MARK_SWAPPED 0xFFFE
#define UNI_SUR_HIGH_START (char32_t)0xD800
#define UNI_SUR_HIGH_END (char32_t)0xDBFF
#define UNI_SUR_LOW_START (char32_t)0xDC00
#define UNI_SUR_LOW_END (char32_t)0xDFFF

namespace bela {

namespace codecvt_internal {
template <typename T> class Literal;
template <> class Literal<char> {
public:
  static constexpr std::string_view Empty = "\"\"";
  static constexpr std::string_view UnicodePrefix = "\\U";
};
template <> class Literal<wchar_t> {
public:
#if defined(_MSC_VER) || defined(_LIBCPP_VERSION)
  static constexpr std::wstring_view Empty = L"\"\"";
  static constexpr std::wstring_view UnicodePrefix = L"\\U";
#else
  // libstdc++ call wcslen is bad
  static constexpr std::wstring_view Empty{L"\"\"", sizeof("\"\"") - 1};
  static constexpr std::wstring_view UnicodePrefix = {L"\\U",
                                                      sizeof("\\U") - 1};
#endif
};
template <> class Literal<char16_t> {
public:
  static constexpr std::u16string_view Empty = u"\"\"";
  static constexpr std::u16string_view UnicodePrefix = u"\\U";
};
} // namespace codecvt_internal

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
// clang-format off
static const char trailingbytesu8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
// clang-format on
constexpr const char32_t offsetfromu8[6] = {0x00000000UL, 0x00003080UL,
                                            0x000E2080UL, 0x03C82080UL,
                                            0xFA082080UL, 0x82082080UL};
constexpr const char32_t halfBase = 0x0010000UL;
constexpr const char32_t halfMask = 0x3FFUL;
constexpr const int halfShift = 10; /* used for shifting by 10 bits */
constexpr const size_t kMaxEncodedUTF8Size = 4;

size_t char32tochar16(char16_t *buf, size_t n, char32_t ch) {
  if (n < 2) {
    return 0;
  }
  if (ch <= UNI_MAX_BMP) {
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
      buf[0] = static_cast<char16_t>(UNI_REPLACEMENT_CHAR);
      return 1;
    }
    buf[0] = static_cast<char16_t>(ch);
    return 1;
  }
  if (ch > UNI_MAX_LEGAL_UTF32) {
    buf[0] = static_cast<char16_t>(UNI_REPLACEMENT_CHAR);
    return 1;
  }
  ch -= halfBase;
  buf[0] = static_cast<char16_t>((ch >> halfShift) + UNI_SUR_HIGH_START);
  buf[1] = static_cast<char16_t>((ch & halfMask) + UNI_SUR_LOW_START);
  return 2;
}

bool islegau8(const uint8_t *source, int length) {
  uint16_t a;
  const uint8_t *srcptr = source + length;
  switch (length) {
  default:
    return false;
    /* Everything else falls through when "true"... */
  case 4:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
      return false;
    }
    [[fallthrough]];
  case 3:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
      return false;
    }
    [[fallthrough]];
  case 2:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
      return false;
    }
    switch (*source) {
    /* no fall-through in this inner switch */
    case 0xE0:
      if (a < 0xA0) {
        return false;
      }
      break;
    case 0xED:
      if (a > 0x9F) {
        return false;
      }
      break;
    case 0xF0:
      if (a < 0x90) {
        return false;
      }
      break;
    case 0xF4:
      if (a > 0x8F) {
        return false;
      }
      break;
    default:
      if (a < 0x80) {
        return false;
      }
    }
    [[fallthrough]];
  case 1:
    if (*source >= 0x80 && *source < 0xC2) {
      return false;
    }
  }
  return *source <= 0xF4;
}

static inline size_t char32tochar8_internal(char *buffer, char32_t ch) {
  if (ch < (char32_t)0x80) {
    *buffer = static_cast<char>(ch);
    return 1;
  }
  if (ch < (char32_t)0x800) {
    buffer[1] = static_cast<char>(0x80 | (ch & 0x3F));
    ch >>= 6;
    buffer[0] = static_cast<char>(0xc0 | ch);
    return 2;
  }
  if (ch < (char32_t)0x10000) {
    buffer[2] = static_cast<char>(0x80 | (ch & 0x3F));
    ch >>= 6;
    buffer[1] = static_cast<char>(0x80 | (ch & 0x3F));
    ch >>= 6;
    buffer[0] = static_cast<char>(0xE0 | ch);
    return 3;
  }
  buffer[3] = static_cast<char>(0x80 | (ch & 0x3F));
  ch >>= 6;
  buffer[2] = static_cast<char>(0x80 | (ch & 0x3F));
  ch >>= 6;
  buffer[1] = static_cast<char>(0x80 | (ch & 0x3F));
  ch >>= 6;
  buffer[0] = static_cast<char>(0xF0 | ch);
  return 4;
}

size_t char32tochar8(char *buf, size_t n, char32_t ch) {
  if (n < kMaxEncodedUTF8Size) {
    return 0;
  }
  return char32tochar8_internal(buf, ch);
}

std::string c16tomb(const char16_t *data, size_t len, bool skipillegal) {
  std::string s;
  s.reserve(len);
  auto it = data;
  auto end = it + len;
  char buffer[8] = {0};
  while (it < end) {
    char32_t ch = *it++;
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      if (it >= end) {
        // parse skip
        return s;
      }
      char32_t ch2 = *it;
      if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
        ch = ((ch - UNI_SUR_HIGH_START) << halfShift) +
             (ch2 - UNI_SUR_LOW_START) + halfBase;
        ++it;
      } else if (skipillegal) {
        /* We don't have the 16 bits following the high surrogate. */
        return s;
      }
    }
    auto bw = char32tochar8_internal(buffer, ch);
    s.append(reinterpret_cast<const char *>(buffer), bw);
  }
  return s;
}

inline char32_t AnnexU8(const uint8_t *it, int nb) {
  char32_t ch = 0;
  switch (nb) {
  case 5:
    ch += *it++;
    ch <<= 6; /* remember, illegal UTF-8 */
    [[fallthrough]];
  case 4:
    ch += *it++;
    ch <<= 6; /* remember, illegal UTF-8 */
    [[fallthrough]];
  case 3:
    ch += *it++;
    ch <<= 6;
    [[fallthrough]];
  case 2:
    ch += *it++;
    ch <<= 6;
    [[fallthrough]];
  case 1:
    ch += *it++;
    ch <<= 6;
    [[fallthrough]];
  case 0:
    ch += *it++;
  }
  ch -= offsetfromu8[nb];
  return ch;
}

template <typename T, typename Allocator>
bool mbrtoc16(const unsigned char *s, size_t len,
              std::basic_string<T, std::char_traits<T>, Allocator> &container,
              bool skipillegal) {
  if (s == nullptr || len == 0) {
    return false;
  }
  container.reserve(len);
  auto it = reinterpret_cast<const unsigned char *>(s);
  auto end = it + len;
  while (it < end) {
    unsigned short nb = trailingbytesu8[*it];
    if (nb >= end - it) {
      return false;
    }
    if (!islegau8(it, nb + 1)) {
      return false;
    }
    // https://docs.microsoft.com/en-us/cpp/cpp/attributes?view=vs-2019
    auto ch = AnnexU8(it, nb);
    it += nb + 1;
    if (ch <= UNI_MAX_BMP) {
      if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
        if (skipillegal) {
          return false;
        }
        container += static_cast<T>(UNI_REPLACEMENT_CHAR);
        continue;
      }
      container += static_cast<T>(ch);
      continue;
    }
    if (ch > UNI_MAX_UTF16) {
      if (skipillegal) {
        return false;
      }
      container += static_cast<T>(UNI_REPLACEMENT_CHAR);
      continue;
    }
    ch -= halfBase;
    container += static_cast<T>((ch >> halfShift) + UNI_SUR_HIGH_START);
    container += static_cast<T>((ch & halfMask) + UNI_SUR_LOW_START);
  }
  //
  return true;
}

std::wstring mbrtowc(const unsigned char *str, size_t len, bool skipillegal) {
  std::wstring s;
  if (!mbrtoc16(str, len, s, skipillegal)) {
    s.clear();
  }
  return s;
}
std::u16string mbrtoc16(const unsigned char *str, size_t len,
                        bool skipillegal) {
  std::u16string s;
  if (!mbrtoc16(str, len, s, skipillegal)) {
    s.clear();
  }
  return s;
}

template <size_t N, typename T>
inline std::basic_string_view<T> EncodeUnicode(T (&buf)[N], char32_t ch) {
  T *end = buf + N;
  T *writer = end;
  uint64_t value = ch;
  static const char hexdigits[] = "0123456789ABCDEF";
  do {
    *--writer = static_cast<T>(hexdigits[value & 0xF]);
    value >>= 4;
  } while (value != 0);

  T *beg;
  if (end - writer < 8) {
    beg = end - 8;
    std::fill_n(beg, writer - beg, '0');
  } else {
    beg = writer;
  }
  return std::basic_string_view<T>(beg, end - beg);
}

// EscapeNonBMP UTF-8
std::string EscapeNonBMP(std::string_view sv) {
  if (sv.empty()) {
    return "";
  }
  std::string s;
  s.reserve(sv.size());
  char ub[10] = {0};
  auto len = sv.size();
  auto it = reinterpret_cast<const unsigned char *>(sv.data());
  auto end = it + len;
  while (it < end) {
    auto c = *it;
    if (c == '\\') {
      s += "\\\\";
      it++;
      continue;
    }
    if (c < 0x80) {
      s += c;
      it++;
      continue;
    }
    unsigned short nb = trailingbytesu8[*it];
    if (nb >= end - it) {
      return s;
    }
    auto ch = AnnexU8(it, nb);
    if (ch <= UNI_MAX_BMP) {
      s.append(reinterpret_cast<const char *>(it), nb + 1);
      it += nb + 1;
      continue;
    }
    s.append(codecvt_internal::Literal<char>::UnicodePrefix)
        .append(EncodeUnicode(ub, ch));
    it += nb + 1;
  }
  return s;
}

template <typename T>
std::basic_string<T> EscapeNonBMPInternal(std::u16string_view sv) {
  std::basic_string<T> s;
  s.reserve(sv.size());
  auto it = sv.data();
  auto end = it + sv.size();
  T buffer[10] = {0};
  while (it < end) {
    char32_t ch = *it++;
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      if (it >= end) {
        return s;
      }
      char32_t ch2 = *it;
      if (ch2 < UNI_SUR_LOW_START || ch2 > UNI_SUR_LOW_END) {
        break;
      }
      ch = ((ch - UNI_SUR_HIGH_START) << halfShift) +
           (ch2 - UNI_SUR_LOW_START) + halfBase;
      ++it;
    }
    if (ch < UNI_MAX_BMP) {
      s += static_cast<T>(ch);
      continue;
    }
    s.append(codecvt_internal::Literal<T>::UnicodePrefix)
        .append(EncodeUnicode(buffer, ch));
  }
  return s;
}

// EscapeNonBMP UTF-16
std::wstring EscapeNonBMP(std::wstring_view sv) {
  return EscapeNonBMPInternal<wchar_t>(
      {reinterpret_cast<const char16_t *>(sv.data()), sv.size()});
}
std::u16string EscapeNonBMP(std::u16string_view sv) {
  return EscapeNonBMPInternal<char16_t>(sv);
}

// CalculateLength calculate unicode codepoint display width
// http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
// http://www.unicode.org/Public/UCD/latest/ucd/EastAsianWidth.txt
size_t CalculateWidth(char32_t ch) {
  return codecvt_internal::CalculateWidthInternal(ch);
}

// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
// https://vt100.net/

// Calculate UTF-8 string display width
size_t StringWidth(std::string_view str) {
  size_t width = 0;
  auto it = reinterpret_cast<const unsigned char *>(str.data());
  auto end = it + str.size();
  while (it < end) {
    if (*it == 0x1b) {
      while (it < end) {
        // We only support strip ANSI color
        if (*it == 'm') {
          it++;
          break;
        }
        it++;
      }
      if (it >= end) {
        break;
      }
    }
    unsigned short nb = trailingbytesu8[*it];
    if (nb >= end - it) {
      break;
    }
    auto ch = AnnexU8(it, nb);
    it += nb + 1;
    width += CalculateWidth(ch);
  }
  return 0;
}

// Calculate UTF-16 string display width
size_t StringWidth(std::u16string_view str) {
  size_t width = 0;
  auto it = str.data();
  auto end = it + str.size();
  while (it < end) {
    if (*it == 0x1b) {
      while (it < end) {
        // We only support strip ANSI color
        if (*it == L'm') {
          it++;
          break;
        }
        it++;
      }
      if (it >= end) {
        break;
      }
    }
    char32_t ch = *it++;
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      if (it >= end) {
        break;
      }
      char32_t ch2 = *it;
      if (ch2 < UNI_SUR_LOW_START || ch2 > UNI_SUR_LOW_END) {
        break;
      }
      ch = ((ch - UNI_SUR_HIGH_START) << halfShift) +
           (ch2 - UNI_SUR_LOW_START) + halfBase;
      ++it;
    }
    width += CalculateWidth(ch);
  }
  return width;
}

} // namespace bela
