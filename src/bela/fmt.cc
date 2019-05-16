//////
#include <cstring>
#include <wchar.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <bela/fmt.hpp>

namespace bela {
namespace format_internal {
constexpr const size_t npos = static_cast<size_t>(-1);
size_t memsearch(const wchar_t *begin, const wchar_t *end, int ch) {
  for (auto it = begin; it != end; it++) {
    if (*it == ch) {
      return it - begin;
    }
  }
  return npos;
}

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

struct u16container {
  std::vector<char16_t> data_;
  const wchar_t *wstr() const {
    return reinterpret_cast<const wchar_t *>(data_.data());
  }
  size_t size() const { return data_.size(); }
  bool mbrtoc16(const char *s, size_t n, bool skipillegal = false);
};

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

bool u16container::mbrtoc16(const char *s, size_t n, bool skipillegal) {
  constexpr const char32_t halfBase = 0x0010000UL;
  constexpr const char32_t halfMask = 0x3FFUL;
  constexpr const int halfShift = 10; /* used for shifting by 10 bits */

  // UTF-8 all 1byte. ---> numbers equal UTF-16
  // UTF-8 all 4byte --> UTF-16/ numbers/2
  data_.reserve(n); // This capacity is almost enough
  auto it = reinterpret_cast<const unsigned char *>(s);
  auto end = it + n;
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
        data_.push_back(static_cast<char16_t>(UNI_REPLACEMENT_CHAR));
        continue;
      }
      data_.push_back(static_cast<char16_t>(ch));
      continue;
    }
    if (ch > UNI_MAX_UTF16) {
      if (skipillegal) {
        return false;
      }
      data_.push_back(static_cast<char16_t>(UNI_REPLACEMENT_CHAR));
      continue;
    }
    ch -= halfBase;
    data_.push_back(
        static_cast<char16_t>((ch >> halfShift) + UNI_SUR_HIGH_START));
    data_.push_back(static_cast<char16_t>((ch & halfMask) + UNI_SUR_LOW_START));
  }
  //
  return true;
}

class buffer {
public:
  buffer(wchar_t *data_, size_t cap_) : data(data_), cap(cap_) {}
  buffer(const buffer &) = delete;
  ~buffer() {
    if (len < cap) {
      data[len] = 0;
    }
  }
  void push_back(wchar_t ch) {
    if (len < cap) {
      data[len++] = ch;
    } else {
      overflow = true;
    }
  }
  buffer &append(std::wstring_view s) {
    if (len + s.size() < cap) {
      memcpy(data + len, s.data(), s.size() * 2);
      len += s.size();
    } else {
      overflow = true;
    }
    return *this;
  }
  buffer &append(const wchar_t *str, size_t dl) {
    if (len + dl < cap) {
      memcpy(data + len, str, dl * 2);
      len += dl;
    } else {
      overflow = true;
    }
    return *this;
  }
  bool IsOverflow() const { return overflow; }
  size_t length() const { return len; }

private:
  wchar_t *data{nullptr};
  size_t len{0};
  size_t cap{0};
  bool overflow{false};
};

constexpr const size_t kFastToBufferSize = 32;
const wchar_t *AlphaNumber(uint64_t value, wchar_t *digits, size_t width,
                           int base, char fill, bool u = false) {
  wchar_t *const end = digits + kFastToBufferSize;
  wchar_t *writer = end;
  constexpr const wchar_t hex[] = L"0123456789abcdef";
  constexpr const wchar_t uhex[] = L"0123456789ABCDEF";
  auto w = (std::min)(width, kFastToBufferSize);
  switch (base) {
  case 8:
    do {
      *--writer = static_cast<wchar_t>('0' + (value & 0x7));
      value >>= 3;
    } while (value != 0);
    break;
  case 16:
    if (u) {
      do {
        *--writer = uhex[value & 0xF];
        value >>= 4;
      } while (value != 0);
    } else {
      do {
        *--writer = hex[value & 0xF];
        value >>= 4;
      } while (value != 0);
    }
    break;
  default:
    do {
      *--writer = hex[value % 10];
      value = value / 10;
    } while (value != 0);
    break;
  }
  wchar_t *beg;
  if ((size_t)(end - writer) < w) {
    beg = end - w;
    std::fill_n(beg, writer - beg, fill);
  } else {
    beg = writer;
  }
  return beg;
}

template <typename T> class Writer {
public:
  Writer(T &t_) : t(t_) {}
  Writer(const Writer &) = delete;
  Writer &operator=(const Writer &) = delete;
  void Pad(size_t pad, bool pz = false) {
    constexpr size_t padlen = 32;
    constexpr const wchar_t padzero[padlen] = {'0'};
    constexpr const wchar_t padspace[padlen] = {' '};
    t.append(pz ? padzero : padspace, (std::min)(pad, padlen));
  }
  void Append(const wchar_t *str, size_t len) { t.append(str, len); }
  void Out(wchar_t ch) { t.push_back(ch); }
  bool Overflow();
  void Floating(double d, uint32_t width, uint32_t frac_width, bool zero) {
    if (std::signbit(d)) {
      Out('-');
      d = -d;
    }
    if (std::isnan(d)) {
      Append(L"nan", 3);
      return;
    }
    if (std::isinf(d)) {
      Append(L"inf", 3);
      return;
    }
    wchar_t digits[kFastToBufferSize + 1];
    const auto dend = digits + kFastToBufferSize;
    auto ui64 = static_cast<int64_t>(d);
    uint64_t frac = 0;
    uint32_t scale = 0;
    if (frac_width > 0) {
      scale = 1;
      for (int n = frac_width; n != 0; n--) {
        scale *= 10;
      }
      frac = (uint64_t)(std::round((d - (double)ui64) * scale));
      if (frac == scale) {
        ui64++;
        frac = 0;
      }
    }
    auto p = AlphaNumber(ui64, digits, width, 10, zero ? '0' : ' ');
    Append(p, dend - p);
    if (frac_width != 0) {
      Out('.');
      p = AlphaNumber(frac, digits, frac_width, 10, zero ? '0' : ' ');
      Append(p, dend - p);
    }
  }

private:
  T &t;
};

template <> inline bool Writer<std::wstring>::Overflow() { return false; }
template <> inline bool Writer<buffer>::Overflow() { return t.IsOverflow(); }

using StringWriter = Writer<std::wstring>;
using BufferWriter = Writer<buffer>;

template <typename T>
bool StrFormatInternal(Writer<T> &w, const wchar_t *fmt, const FormatArg *args,
                       size_t max_args) {
  if (args == nullptr || max_args == 0) {
    return false;
  }
  wchar_t digits[kFastToBufferSize + 1];
  const auto dend = digits + kFastToBufferSize;
  auto it = fmt;
  auto end = it + wcslen(fmt);
  size_t ca = 0;
  bool zero;
  uint32_t width;
  uint32_t frac_width;
  while (it < end) {
    ///  Fast search %,
    auto pos = memsearch(it, end, '%');
    if (pos == npos) {
      w.Append(it, end - it);
      return !w.Overflow();
    }
    w.Append(it, pos);
    it += pos + 1;
    // Parse ---
    zero = (*it == '0');
    width = 0;
    frac_width = 0;
    while (*it >= '0' && *it <= '9') {
      width = width * 10 + (*it++ - '0');
    }
    if (*it == '.') {
      it++;
      while (*it >= '0' && *it <= '9') {
        frac_width = frac_width * 10 + (*it++ - '0');
      }
    }
    switch (*it) {
    case 'b':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::INTEGER || args[ca].at == ArgType::UINTEGER ||
          args[ca].at == ArgType::BOOLEAN) {
        if (args[ca].integer.i == 0) {
          w.Append(L"false", sizeof("false") - 1);
        } else {
          w.Append(L"true", sizeof("true") - 1);
        }
      }
      ca++;
      break;
    case 'c':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::INTEGER || args[ca].at == ArgType::UINTEGER) {
        if (width > 1) {
          w.Pad(width - 1, zero);
        }
        w.Out(static_cast<char>(args[ca].integer.i));
      }
      ca++;
      break;
    case 's':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::STRING) {
        if (width > args[ca].strings.len) {
          w.Pad(width - args[ca].strings.len, zero);
        }
        w.Append(args[ca].strings.data, args[ca].strings.len);
      } else if (args[ca].at == ArgType::USTRING) {
        u16container u16;
        u16.mbrtoc16(args[ca].ustring.data, args[ca].ustring.len);
        if (width > u16.size()) {
          w.Pad(width - static_cast<uint32_t>(u16.size()), zero);
        }
        w.Append(u16.wstr(), u16.size());
      }
      ca++;
      break;
    case 'd':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at != ArgType::STRING) {
        bool sign = false;
        auto val = args[ca].ToInteger(&sign);
        auto p = AlphaNumber(val, digits, width, 10, zero ? '0' : ' ');
        if (sign) {
          w.Out('-');
        }
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'o':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at != ArgType::STRING) {
        auto val = args[ca].ToInteger();
        auto p = AlphaNumber(val, digits, width, 8, zero ? '0' : ' ');
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'x':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at != ArgType::STRING) {
        auto val = args[ca].ToInteger();
        auto p = AlphaNumber(val, digits, width, 16, zero ? '0' : ' ');
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'X':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at != ArgType::STRING) {
        auto val = args[ca].ToInteger();
        auto p = AlphaNumber(val, digits, width, 16, zero ? '0' : ' ', true);
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'f':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::FLOAT) {
        w.Floating(args[ca].floating.d, width, frac_width, zero);
      }
      ca++;
      break;
    case 'a':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::FLOAT) {
        union {
          double d;
          uint64_t i;
        } x;
        x.d = args[ca].floating.d;
        auto p = AlphaNumber(x.i, digits, width, 16, zero ? '0' : ' ', true);
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'p':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::POINTER) {
        auto ptr = reinterpret_cast<ptrdiff_t>(args[ca].ptr);
        auto p = AlphaNumber(ptr, digits, width, 16, zero ? '0' : ' ');
        w.Append(L"0x", 2);    /// Force append 0x to pointer
        w.Append(p, dend - p); // 0xffff00000;
      }
      ca++;
      break;
    default:
      // % and other
      w.Out(*it);
      break;
    }
    it++;
  }
  return !w.Overflow();
}

std::wstring StrFormatInternal(const wchar_t *fmt, const FormatArg *args,
                               size_t max_args) {
  std::wstring s;
  StringWriter sw(s);
  if (!StrFormatInternal(sw, fmt, args, max_args)) {
    return L"";
  }
  return s;
}

ssize_t StrFormatInternal(wchar_t *buf, size_t N, const wchar_t *fmt,
                          const FormatArg *args, size_t max_args) {
  buffer buffer_(buf, N);
  BufferWriter bw(buffer_);
  if (!StrFormatInternal(bw, fmt, args, max_args)) {
    return -1;
  }
  return static_cast<ssize_t>(buffer_.length());
}
} // namespace format_internal

ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt) {
  format_internal::buffer buffer_(buf, N);
  std::wstring s;
  const wchar_t *src = fmt;
  for (; *src != 0; ++src) {
    buffer_.push_back(*src);
    if (src[0] == '%' && src[1] == '%') {
      ++src;
    }
  }
  return buffer_.IsOverflow() ? -1 : static_cast<ssize_t>(buffer_.length());
}

std::wstring StrFormat(const wchar_t *fmt) {
  std::wstring s;
  const wchar_t *src = fmt;
  for (; *src != 0; ++src) {
    s.push_back(*src);
    if (src[0] == '%' && src[1] == '%') {
      ++src;
    }
  }
  return s;
}
} // namespace bela
