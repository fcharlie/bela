/////
#ifndef BELA_FMT_HPP
#define BELA_FMT_HPP
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <string_view>

#if defined(_MSC_VER)
// Define ssize_t inside of our namespace.
#if defined(_WIN64)
typedef __int64 __ssize_t;
#else
typedef long __ssize_t;
#endif
#endif

namespace bela {
using ssize_t = __ssize_t;
namespace format_internal {
enum class ArgType {
  INTEGER, // short,int,
  UINTEGER,
  FLOAT,
  STRING,
  POINTER,
  BOOLEAN
};
struct FormatArg {
  FormatArg(char c) : at(ArgType::INTEGER) {
    integer.i = c; /// if caset to uint64_t
    integer.width = sizeof(char);
  }
  FormatArg(unsigned char c) : at(ArgType::UINTEGER) {
    integer.i = c;
    integer.width = sizeof(char);
  }
  FormatArg(signed short j) : at(ArgType::INTEGER) {
    integer.i = j;
    integer.width = sizeof(short);
  }
  FormatArg(unsigned short j) : at(ArgType::UINTEGER) {
    integer.i = j;
    integer.width = sizeof(short);
  }
  FormatArg(signed int j) : at(ArgType::INTEGER) {
    integer.i = j;
    integer.width = sizeof(int);
  }
  FormatArg(unsigned int j) : at(ArgType::UINTEGER) {
    integer.i = j;
    integer.width = sizeof(int);
  }
  FormatArg(signed long j) : at(ArgType::INTEGER) {
    integer.i = j;
    integer.width = sizeof(long);
  }
  FormatArg(unsigned long j) : at(ArgType::UINTEGER) {
    integer.i = j;
    integer.width = sizeof(long);
  }
  FormatArg(signed long long j) : at(ArgType::INTEGER) {
    integer.i = j;
    integer.width = sizeof(long long);
  }
  FormatArg(unsigned long long j) : at(ArgType::UINTEGER) {
    integer.i = j;
    integer.width = sizeof(long long);
  }
  FormatArg(bool b) : at(ArgType::BOOLEAN) {
    integer.i = b ? 1 : 0;
    integer.width = sizeof(char);
  }
  FormatArg(float f) : at(ArgType::FLOAT) {
    floating.d = f;
    floating.width = sizeof(float);
  }
  // A C-style text string. and string_view
  FormatArg(const wchar_t *str) : at(ArgType::STRING) {
    strings.data = (str == nullptr) ? L"(NULL)" : str;
    strings.len = (str == nullptr) ? sizeof("(NULL)") - 1 : wcslen(str);
  }
  template <typename Allocator>
  FormatArg( // NOLINT(runtime/explicit)
      const std::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>
          &str)
      : at(ArgType::STRING) {
    strings.data = str.data();
    strings.len = str.size();
  }
  FormatArg(std::wstring_view sv) : at(ArgType::STRING) {
    strings.data = sv.data();
    strings.len = sv.size();
  }

  // Any pointer value that can be cast to a "void*".
  template <class T> FormatArg(T *p) : ptr((void *)p), at(ArgType::POINTER) {}

  /// Convert To integer
  uint64_t ToInteger(bool *sign = nullptr) const noexcept {
    switch (at) {
    case ArgType::POINTER:
      return reinterpret_cast<uintptr_t>(ptr);
    case ArgType::FLOAT: {
      union {
        double d;
        uint64_t i;
      } x;
      x.d = floating.d;
      return x.i;
    }
    default:
      break;
    }
    int64_t i = integer.i;
    if (sign != nullptr) {
      if (at == ArgType::UINTEGER || !(*sign = i < 0)) {
        return static_cast<uint64_t>(i);
      }
      if (integer.width == 1) {
        return static_cast<uint32_t>(0 - static_cast<int8_t>(i));
      }
      if (integer.width == 2) {
        return static_cast<uint32_t>(0 - static_cast<int16_t>(i));
      }
      if (integer.width == 1) {
        return static_cast<uint32_t>(0 - static_cast<int32_t>(i));
      }
      return static_cast<uint64_t>(0 - i);
    }
    if (integer.width < sizeof(int64_t)) {
      i &= (1LL << (8 * integer.width)) - 1;
    }
    return static_cast<uint64_t>(i);
  }

  union {
    struct {
      int64_t i;
      unsigned char width;
    } integer;
    struct {
      double d;
      unsigned char width;
    } floating;
    struct {
      const wchar_t *data;
      size_t len;
    } strings;
    // struct {
    //   const char *str;
    //   size_t len;
    // } mbstring;
    const void *ptr;
  };
  const ArgType at;
};
ssize_t StrFormatInternal(wchar_t *buf, size_t sz, const wchar_t *fmt,
                          const FormatArg *args, size_t max_args);
std::wstring StrFormatInternal(const wchar_t *fmt, const FormatArg *args,
                               size_t max_args);
} // namespace format_internal

template <typename... Args>
ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt, Args... args) {
  const format_internal::FormatArg arg_array[] = {args...};
  return format_internal::StrFormatInternal(buf, N, fmt, arg_array,
                                            sizeof...(args));
}

template <size_t N, typename... Args>
ssize_t StrFormat(wchar_t (&buf)[N], const wchar_t *fmt, Args... args) {
  // Use Arg() object to record type information and then copy arguments to an
  // array to make it easier to iterate over them.
  const format_internal::FormatArg arg_array[] = {args...};
  return format_internal::StrFormatInternal(buf, N, fmt, arg_array,
                                            sizeof...(args));
}

template <typename... Args>
std::wstring StrFormat(const wchar_t *fmt, Args... args) {
  const format_internal::FormatArg arg_array[] = {args...};
  return format_internal::StrFormatInternal(fmt, arg_array, sizeof...(args));
}

// Fast-path when we don't actually need to substitute any arguments.
ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt);
std::wstring StrFormat(const wchar_t *fmt);
template <size_t N>
inline ssize_t StrFormat(wchar_t (&buf)[N], const wchar_t *fmt) {
  return StrFormat(buf, N, fmt);
}

} // namespace bela

#endif
