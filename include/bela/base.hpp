//////////
// Bela windows basic defined
// Windows error_code impl
#ifndef BELA_BASE_HPP
#define BELA_BASE_HPP
#pragma once
#include <SDKDDKVer.h>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "strcat.hpp"

namespace bela {
constexpr auto noerror_t = 0L; // NO_ERROR 0L
struct error_code {
  std::wstring message;
  long code{noerror_t};
  const wchar_t *data() const { return message.data(); }
  explicit operator bool() const noexcept { return code != noerror_t; }
};

inline bela::error_code make_error_code(long code, const AlphaNum &a) {
  return bela::error_code{std::wstring(a.Piece()), code};
}
inline bela::error_code make_error_code(long code, const AlphaNum &a,
                                        const AlphaNum &b) {
  bela::error_code ec;
  ec.code = code;
  ec.message.reserve(a.Piece().size() + b.Piece().size());
  ec.message.assign(a.Piece()).append(b.Piece());
  return ec;
}
inline bela::error_code make_error_code(long code, const AlphaNum &a,
                                        const AlphaNum &b, const AlphaNum &c) {
  bela::error_code ec;
  ec.code = code;
  ec.message.reserve(a.Piece().size() + b.Piece().size() + c.Piece().size());
  ec.message.assign(a.Piece()).append(b.Piece()).append(c.Piece());
  return ec;
}
inline bela::error_code make_error_code(long code, const AlphaNum &a,
                                        const AlphaNum &b, const AlphaNum &c,
                                        const AlphaNum &d) {
  bela::error_code ec;
  ec.code = code;
  ec.message.reserve(a.Piece().size() + b.Piece().size() + c.Piece().size() +
                     d.Piece().size());
  ec.message.assign(a.Piece()).append(b.Piece()).append(c.Piece()).append(
      d.Piece());
  return ec;
}
template <typename... AV>
bela::error_code make_error_code(long code, const AlphaNum &a,
                                 const AlphaNum &b, const AlphaNum &c,
                                 const AlphaNum &d, AV... av) {
  bela::error_code ec;
  ec.code = code;
  ec.message = strings_internal::CatPieces({a, b, c, d, av...});
  return ec;
}

inline std::wstring system_error_dump(DWORD ec) {
  LPWSTR buf = nullptr;
  auto rl = FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, ec,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buf, 0, nullptr);
  if (rl == 0) {
    return L"FormatMessageW error";
  }
  std::wstring msg(buf, rl);
  LocalFree(buf);
  return msg;
}

inline error_code make_system_error_code() {
  error_code ec;
  ec.code = GetLastError();
  ec.message = system_error_dump(ec.code);
  return ec;
}

// ToNarrow UTF-16 to UTF-8
inline std::string ToNarrow(std::wstring_view uw) {
  auto l = WideCharToMultiByte(CP_UTF8, 0, uw.data(), (int)uw.size(), nullptr,
                               0, nullptr, nullptr);
  std::string ustr;
  ustr.resize(l + 1);
  auto N = WideCharToMultiByte(CP_UTF8, 0, uw.data(), (int)uw.size(),
                               ustr.data(), l + 1, nullptr, nullptr);
  ustr.resize(N);
  return ustr;
}

// ToWide UTF-8 to UTF-16
inline std::wstring ToWide(std::string_view u8) {
  std::wstring wstr;
  auto N =
      MultiByteToWideChar(CP_UTF8, 0, u8.data(), (DWORD)u8.size(), nullptr, 0);
  if (N > 0) {
    wstr.resize(N);
    MultiByteToWideChar(CP_UTF8, 0, u8.data(), (DWORD)u8.size(), &wstr[0], N);
  }
  return wstr;
}

} // namespace bela

#endif
