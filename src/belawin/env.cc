///////////
// bela expand env
#include <bela/base.hpp>
#include <bela/env.hpp>

namespace bela {
namespace env_internal {
constexpr const size_t npos = static_cast<size_t>(-1);
size_t memsearch(const wchar_t *begin, const wchar_t *end, int ch) {
  for (auto it = begin; it != end; it++) {
    if (*it == ch) {
      return it - begin;
    }
  }
  return npos;
}

} // namespace env_internal

// https://docs.microsoft.com/en-us/windows/desktop/api/processenv/nf-processenv-expandenvironmentstringsw
std::wstring ExpandEnv(std::wstring_view sv) {
  auto pos = sv.find('%');
  if (pos == std::wstring_view::npos) {
    return std::wstring(sv);
  }
  auto pos2 = sv.find(L'%', pos + 1);
  if (pos2 == std::wstring_view::npos) {
    return std::wstring(sv);
  }
  std::wstring buf;
  buf.resize(sv.size() + 256);
  auto N = ExpandEnvironmentStringsW(sv.data(), buf.data(),
                                     static_cast<DWORD>(buf.size()));
  if (static_cast<size_t>(N) > buf.size()) {
    buf.resize(N);
    N = ExpandEnvironmentStringsW(sv.data(), buf.data(),
                                  static_cast<DWORD>(buf.size()));
  }
  if (N == 0 || static_cast<size_t>(N) > buf.size()) {
    return L"";
  }
  buf.resize(N - 1);
  return buf;
}

std::wstring PathUnExpand(std::wstring_view sv) {
  constexpr const std::wstring_view envvars[] = {
      //--
      L"ALLUSERSPROFILE", //
      L"APPDATA",         //
      L"COMPUTERNAME",    //
      L"ProgramFiles",    //
      L"SystemRoot",      //
      L"SystemDrive",     //
      L"USERPROFILE"
      //
  };
  std::wstring buf;
  buf.reserve(sv.size());
  auto it = sv.data();
  auto end = it + sv.size();
  while (it < end) {
    auto pos = env_internal::memsearch(it, end, '%');
    if (pos == env_internal::npos) {
      buf.append(it, end - it);
      break;
    }
    buf.append(it, pos);
    it += pos + 1;
    if (it >= end) {
      break;
    }
    size_t xlen = end - it;
    for (auto e : envvars) {
      if (xlen < e.size() + 1) {
        continue;
      }
      if (_wcsnicmp(it, e.data(), e.size()) == 0 && it[e.size()] == '%') {
        buf.append(GetEnv(e));
        it += e.size() + 1;
      }
    }
  }
  return buf;
}

namespace env {
bool Derivative::EraseEnv(std::wstring_view key) {
  return envblock.erase(key) != 0;
}

bool Derivative::SetEnv(std::wstring_view key, std::wstring_view value,
                        bool force) {
  auto it = envblock.find(key);
  if (force) {
    envblock[key] = value;
    return true;
  }
  return envblock.emplace(key, value).second;
}

bool Derivative::PutEnv(std::wstring_view nv, bool force) {
  auto pos = nv.find(L'=');
  if (pos == std::wstring_view::npos) {
    return SetEnv(nv, L"", force);
  }
  return SetEnv(nv.substr(0, pos), nv.substr(pos + 1), force);
}

std::wstring_view Derivative::GetEnv(std::wstring_view key) const {
  auto it = envblock.find(key);
  if (it == envblock.end()) {
    return L"";
  }
  return it->second;
}

// Expand Env string to normal string only support '${KEY}' ?
bool Derivative::ExpandEnv(std::wstring_view raw, std::wstring &w) const {
  //
  return false;
}

// DerivativeMT support MultiThreading
bool DerivativeMT::EraseEnv(std::wstring_view key) {
  return envblock.erase(key) != 0; /// Internal is thread safe
}

bool DerivativeMT::SetEnv(std::wstring_view key, std::wstring_view value,
                          bool force) {
  if (force) {
    envblock[key] = value;
    return true;
  }
  return envblock.emplace(key, value).second;
}

bool DerivativeMT::PutEnv(std::wstring_view nv, bool force) {
  auto pos = nv.find(L'=');
  if (pos == std::wstring_view::npos) {
    return SetEnv(nv, L"", force);
  }
  return SetEnv(nv.substr(0, pos), nv.substr(pos + 1), force);
}

std::wstring DerivativeMT::GetEnv(std::wstring_view key) {
  auto it = envblock.find(key);
  if (it == envblock.end()) {
    return L"";
  }
  return it->second;
}

bool DerivativeMT::ExpandEnv(std::wstring_view raw, std::wstring &w) {
  //
  return true;
}

} // namespace env

} // namespace bela