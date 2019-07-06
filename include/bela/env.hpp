/// Env helper
#ifndef BELA_ENV_HPP
#define BELA_ENV_HPP
#include <string>
#include <string_view>
#include <shared_mutex>
#include "phmap.hpp"
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

namespace env {
class Derivative {
public:
  Derivative() = default;
  Derivative(const Derivative &) = delete;
  Derivative &operator=(const Derivative &) = delete;
  bool EraseEnv(std::wstring_view key);
  bool SetEnv(std::wstring_view key, std::wstring_view value,
              bool force = false);
  bool PutEnv(std::wstring_view nv, bool force = false);
  std::wstring_view GetEnv(std::wstring_view key) const;
  bool ExpandEnv(std::wstring_view raw, std::wstring &w) const;

private:
  bela::flat_hash_map<std::wstring, std::wstring> envblock;
};

class DerivativeMT {
public:
  DerivativeMT() = default;
  DerivativeMT(const DerivativeMT &) = delete;
  DerivativeMT &operator=(const DerivativeMT &) = delete;
  bool EraseEnv(std::wstring_view key);
  bool SetEnv(std::wstring_view key, std::wstring_view value,
              bool force = false);
  bool PutEnv(std::wstring_view nv, bool force = false);
  std::wstring GetEnv(std::wstring_view key);
  bool ExpandEnv(std::wstring_view raw, std::wstring &w);

private:
  //mutable std::shared_mutex mtx;
  bela::parallel_flat_hash_map<std::wstring, std::wstring> envblock;
};

} // namespace env

} // namespace bela

#endif
