/// Env helper
#ifndef BELA_ENV_HPP
#define BELA_ENV_HPP
#include <string>
#include <string_view>
#include "base.hpp"

namespace bela {
// GetEnv You should set the appropriate size for the initial allocation
// according to your needs.
// https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getenvironmentvariable
template <size_t Len = 256> std::wstring GetEnv(std::wstring_view val) {
  std::wstring s;
  s.resize(Len);
  auto len = GetEnvironmentVariableW(val.data(), s.data(), Len);
  if (len == 0) {
    return L"";
  }
  if (len < Len) {
    s.resize(len);
    return s;
  }
  s.resize(len);
  auto nlen = GetEnvironmentVariableW(val.data(), s.data(), len);
  if (nlen == 0 || nlen > len) {
    return L"";
  }
  s.resize(nlen);
  return s;
}

std::wstring ExpandEnv(std::wstring_view sv);
std::wstring PathUnExpand(std::wstring_view sv);
} // namespace bela

#endif
