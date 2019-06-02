/// Env helper
#ifndef BELA_ENV_HPP
#define BELA_ENV_HPP
#include <string>
#include <string_view>
#include "base.hpp"

namespace bela {
// GetEnv You should set the appropriate size for the initial allocation
// according to your needs.
template <size_t Len = 256> std::wstring GetEnv(std::wstring_view val) {
  std::wstring s;
  s.resize(Len);
  auto len = GetEnvironmentVariableW(val.data(), s.data(), Len);
  if (len == 0) {
    return L"";
  }
  if (len > Len) {
    s.resize(len);
    len = GetEnvironmentVariableW(val.data(), s.data(), len);
    if (len == 0) {
      return L"";
    }
  }
  s.resize(len);
  return s;
}

std::wstring ExpandEnv(std::wstring_view sv);
std::wstring PathUnExpand(std::wstring_view sv);
} // namespace bela

#endif