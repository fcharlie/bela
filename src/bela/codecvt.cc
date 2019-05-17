//////////
#include <bela/codecvt.hpp>
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
  case 3:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
      return false;
    }
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

  case 1:
    if (*source >= 0x80 && *source < 0xC2) {
      return false;
    }
  }
  return *source <= 0xF4;
}

static const char32_t offsetfromu8[6] = {0x00000000UL, 0x00003080UL,
                                         0x000E2080UL, 0x03C82080UL,
                                         0xFA082080UL, 0x82082080UL};
constexpr const char32_t halfBase = 0x0010000UL;
constexpr const char32_t halfMask = 0x3FFUL;
constexpr const int halfShift = 10; /* used for shifting by 10 bits */
static const uint8_t firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0,
                                         0xF0, 0xF8, 0xFC};
std::string c16tomb(const char16_t *data, size_t len, bool skipillegal) {
  std::string s;
  s.reserve(len);
  auto it = data;
  auto end = it + len;
  constexpr const char32_t byteMask = 0xBF;
  constexpr const char32_t byteMark = 0x80;
  while (it < end) {
    unsigned short bw = 0;
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
    /* Figure out how many bytes the result will require */
    if (ch < (char32_t)0x80) {
      bw = 1;
    } else if (ch < (char32_t)0x800) {
      bw = 2;
    } else if (ch < (char32_t)0x10000) {
      bw = 3;
    } else if (ch < (char32_t)0x110000) {
      bw = 4;
    } else {
      bw = 3;
      ch = UNI_REPLACEMENT_CHAR;
    }
    switch (bw) { /* note: everything falls through. */
    case 4:
      s.push_back((uint8_t)((ch | byteMark) & byteMask));
      ch >>= 6;
    case 3:
      s.push_back((uint8_t)((ch | byteMark) & byteMask));
      ch >>= 6;
    case 2:
      s.push_back((uint8_t)((ch | byteMark) & byteMask));
      ch >>= 6;
    case 1:
      s.push_back((uint8_t)(ch | firstByteMark[bw]));
    }
  }
  return s;
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
    char32_t ch = 0;
    unsigned short nb = trailingbytesu8[*it];
    if (nb >= end - it) {
      return false;
    }
    if (!islegau8(it, nb + 1)) {
      return false;
    }
    switch (nb) {
    case 5:
      ch += *it++;
      ch <<= 6; /* remember, illegal UTF-8 */
    case 4:
      ch += *it++;
      ch <<= 6; /* remember, illegal UTF-8 */
    case 3:
      ch += *it++;
      ch <<= 6;
    case 2:
      ch += *it++;
      ch <<= 6;
    case 1:
      ch += *it++;
      ch <<= 6;
    case 0:
      ch += *it++;
    }
    ch -= offsetfromu8[nb];
    if (ch <= UNI_MAX_BMP) {
      if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
        if (skipillegal) {
          return false;
        }
        container.push_back(static_cast<T>(UNI_REPLACEMENT_CHAR));
        continue;
      }
      container.push_back(static_cast<T>(ch));
      continue;
    }
    if (ch > UNI_MAX_UTF16) {
      if (skipillegal) {
        return false;
      }
      container.push_back(static_cast<T>(UNI_REPLACEMENT_CHAR));
      continue;
    }
    ch -= halfBase;
    container.push_back(static_cast<T>((ch >> halfShift) + UNI_SUR_HIGH_START));
    container.push_back(static_cast<T>((ch & halfMask) + UNI_SUR_LOW_START));
  }
  //
  return true;
}

std::wstring mbrtowc(const unsigned char *str, size_t len, bool skipillegal) {
  std::wstring s;
  if (!mbrtoc16(str, len, s, skipillegal)) {
    return L"";
  }
  return s;
}
std::u16string mbrtoc16(const unsigned char *str, size_t len,
                        bool skipillegal) {
  std::u16string s;
  if (!mbrtoc16(str, len, s, skipillegal)) {
    return U"";
  }
  return s;
}
} // namespace bela
