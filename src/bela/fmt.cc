//////
#include <cstring>
#include <wchar.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <bela/fmt.hpp>
#include <bela/codecvt.hpp>
#include "fmtwriter.hpp"

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

using StringWriter = Writer<std::wstring>;
using BufferWriter = Writer<buffer>;

/// because format string is Null-terminated_string
template <typename T>
bool StrFormatInternal(Writer<T> &w, const wchar_t *fmt, const FormatArg *args,
                       size_t max_args) {
  if (args == nullptr || max_args == 0) {
    return false;
  }
  wchar_t digits[kFastToBufferSize];
  const auto dend = digits + kFastToBufferSize;
  auto it = fmt;
  auto end = it + wcslen(fmt);
  size_t ca = 0;
  wchar_t pc;
  uint32_t width;
  uint32_t frac_width;
  bool left = false;
  while (it < end) {
    ///  Fast search %,
    auto pos = memsearch(it, end, '%');
    if (pos == npos) {
      w.Append(it, end - it);
      return !w.overflow();
    }
    w.Append(it, pos);
    /// fmt endswith '\0'
    it += pos + 1;
    if (it >= end) {
      break;
    }
    left = (*it == '-');
    if (left) {
      it++;
      pc = ' ';
    } else {
      pc = (*it == '0') ? '0' : ' ';
    }
    // Parse ---
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
      switch (args[ca].at) {
      case ArgType::BOOLEAN:
      case ArgType::CHARACTER:
        w.AddBoolean(args[ca].character.c != 0);
        break;
      case ArgType::INTEGER:
      case ArgType::UINTEGER:
        w.AddBoolean(args[ca].integer.i != 0);
        break;
      default:
        break;
      }
      ca++;
      break;
    case 'c':
      if (ca >= max_args) {
        return false;
      }
      switch (args[ca].at) {
      case ArgType::CHARACTER:
        w.AddUnicode(args[ca].character.c, width, args[ca].character.width);
        break;
      case ArgType::UINTEGER:
      case ArgType::INTEGER:
        w.AddUnicode(static_cast<char32_t>(args[ca].integer.i), width,
                     args[ca].integer.width > 2 ? 4 : args[ca].integer.width);
        break;
      default:
        break;
      }
      ca++;
      break;
    case 's':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::STRING) {
        w.Append(args[ca].strings.data, args[ca].strings.len, width, pc, left);
      } else if (args[ca].at == ArgType::USTRING) {
        auto ws = bela::ToWide(args[ca].ustring.data, args[ca].ustring.len);
        w.Append(ws.data(), ws.size(), width, pc, left);
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
        auto p = AlphaNum(val, digits, width, 10, pc);
        if (sign) {
          w.Add('-');
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
        auto p = AlphaNum(val, digits, width, 8, pc);
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
        auto p = AlphaNum(val, digits, width, 16, pc);
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
        auto p = AlphaNum(val, digits, width, 16, pc, true);
        w.Append(p, dend - p);
      }
      ca++;
      break;
    case 'U':
      if (ca >= max_args) {
        return false;
      }
      switch (args[ca].at) {
      case ArgType::CHARACTER:
        w.AddUnicodePoint(args[ca].character.c);
        break;
      case ArgType::INTEGER:
      case ArgType::UINTEGER:
        w.AddUnicodePoint(static_cast<char32_t>(args[ca].integer.i));
        break;
      default:
        break;
      }
      ca++;
      break;
    case 'f':
      if (ca >= max_args) {
        return false;
      }
      if (args[ca].at == ArgType::FLOAT) {
        w.Floating(args[ca].floating.d, width, frac_width, pc);
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
        auto p = AlphaNum(x.i, digits, width, 16, pc, true);
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
        auto p = AlphaNum(ptr, digits, width, 16, pc, true);
        w.Append(L"0x", 2);    /// Force append 0x to pointer
        w.Append(p, dend - p); // 0xffff00000;
      }
      ca++;
      break;
    default:
      // % and other
      w.Add(*it);
      break;
    }
    it++;
  }
  return !w.overflow();
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
  return buffer_.overflow() ? -1 : static_cast<ssize_t>(buffer_.length());
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
