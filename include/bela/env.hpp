/// Env helper
#ifndef BELA_ENV_HPP
#define BELA_ENV_HPP
#include <string>
#include <string_view>
#include <shared_mutex>
#include "match.hpp"
#include "str_split.hpp"
#include "str_join.hpp"
#include "span.hpp"
#include "phmap.hpp"
#include "base.hpp"

namespace bela {

template <size_t Len = 256> bool LookupEnv(std::wstring_view key, std::wstring &val) {
  val.resize(Len);
  auto len = GetEnvironmentVariableW(key.data(), val.data(), Len);
  if (len == 0) {
    return !(GetLastError() == ERROR_ENVVAR_NOT_FOUND);
  }
  if (len < Len) {
    val.resize(len);
    return true;
  }
  val.resize(len);
  auto nlen = GetEnvironmentVariableW(val.data(), s.data(), len);
  if (nlen < len) {
    val.resize(nlen);
  }
  return true;
}

// GetEnv You should set the appropriate size for the initial allocation
// according to your needs.
// https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getenvironmentvariable
template <size_t Len = 256> std::wstring GetEnv(std::wstring_view key) {
  std::wstring val;
  LookupEnv(key, val);
  return val;
}

inline void MakePathEnv(std::vector<std::wstring> &paths) {
  auto systemroot = bela::GetEnv(L"SystemRoot");
  auto system32_env = bela::StringCat(systemroot, L"\\System32");
  paths.emplace_back(system32_env);                             // C:\\Windows\\System32
  paths.emplace_back(systemroot);                               // C:\\Windows
  paths.emplace_back(bela::StringCat(system32_env, L"\\Wbem")); // C:\\Windows\\System32\\Wbem
  // C:\\Windows\\System32\\WindowsPowerShell\\v1.0
  paths.emplace_back(bela::StringCat(system32_env, L"\\WindowsPowerShell\\v1.0"));
}

std::wstring ExpandEnv(std::wstring_view sv);
std::wstring PathUnExpand(std::wstring_view sv);

namespace env {
constexpr const wchar_t Separator = L';';
constexpr const std::wstring_view Separators = L";";

struct StringCaseInsensitiveHash {
  using is_transparent = void;
  std::size_t operator()(std::wstring_view wsv) const noexcept {
    /// See Wikipedia
    /// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
#if defined(__x86_64) || defined(_WIN64)
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    constexpr size_t kFNVOffsetBasis = 14695981039346656037ULL;
    constexpr size_t kFNVPrime = 1099511628211ULL;
#else
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
    constexpr size_t kFNVOffsetBasis = 2166136261U;
    constexpr size_t kFNVPrime = 16777619U;
#endif
    size_t val = kFNVOffsetBasis;
    std::string_view sv = {reinterpret_cast<const char *>(wsv.data()), wsv.size() * 2};
    for (auto c : sv) {
      val ^= static_cast<size_t>(bela::ascii_tolower(c));
      val *= kFNVPrime;
    }
    return val;
  }
};

struct StringCaseInsensitiveEq {
  using is_transparent = void;
  bool operator()(std::wstring_view wlhs, std::wstring_view wrhs) const {
    return bela::EqualsIgnoreCase(wlhs, wrhs);
  }
};

using envmap_t = bela::flat_hash_map<std::wstring, std::wstring, StringCaseInsensitiveHash,
                                     StringCaseInsensitiveEq>;

template <typename Range> std::wstring JoinEnv(const Range &range) {
  return bela::strings_internal::JoinRange(range, Separators);
}

template <typename T> std::wstring JoinEnv(std::initializer_list<T> il) {
  return bela::strings_internal::JoinRange(il, Separators);
}

template <typename... T> std::wstring JoinEnv(const std::tuple<T...> &value) {
  return bela::strings_internal::JoinAlgorithm(value, Separators, AlphaNumFormatter());
}

template <typename... Args> std::wstring InsertEnv(std::wstring_view key, Args... arg) {
  std::wstring_view svv[] = {arg...};
  auto prepend = bela::env::JoinEnv(svv);
  auto val = bela::GetEnv(key);
  return bela::StringCat(prepend, Separators, val);
}

template <typename... Args> std::wstring AppendEnv(std::wstring_view key, Args... arg) {
  std::wstring_view svv[] = {arg...};
  auto ended = bela::env::JoinEnv(svv);
  auto val = bela::GetEnv(key);
  return bela::StringCat(val, Separators, ended);
}

// Derivator Environment variable derivation container
class Derivator {
public:
  using value_type = bela::flat_hash_map<std::wstring, std::wstring, StringCaseInsensitiveHash,
                                         StringCaseInsensitiveEq>;
  Derivator() = default;
  Derivator(const Derivator &) = delete;
  Derivator &operator=(const Derivator &) = delete;
  size_t Size() const { return envb.size(); }
  bool AddBashCompatible(int argc, wchar_t *const *argv) {
    for (int i = 0; i < argc; i++) {
      envb.emplace(bela::AlphaNum(i).Piece(), argv[i]);
    }
    envb.emplace(L"$",
                 bela::AlphaNum(GetCurrentProcessId()).Piece()); // current process PID
    envb.emplace(L"@", GetCommandLineW());                       // $@ -->cmdline
    if (auto userprofile = bela::GetEnv(L"USERPROFILE"); !userprofile.empty()) {
      envb.emplace(L"HOME", userprofile);
    }
    return true;
  }
  bool EraseEnv(std::wstring_view key) {
    //
    return envb.erase(key) != 0;
  }
  bool SetEnv(std::wstring_view key, std::wstring_view value, bool force = false) {
    if (force) {
      // envb[key] = value;
      envb.insert_or_assign(key, value);
      return true;
    }
    return envb.emplace(key, value).second;
  }
  bool PutEnv(std::wstring_view nv, bool force = false) {
    auto pos = nv.find(L'=');
    if (pos == std::wstring_view::npos) {
      return SetEnv(nv, L"", force);
    }
    return SetEnv(nv.substr(0, pos), nv.substr(pos + 1), force);
  }
  [[nodiscard]] std::wstring_view GetEnv(std::wstring_view key) const {
    auto it = envb.find(key);
    if (it == envb.end()) {
      return L"";
    }
    return it->second;
  }
  // ExpandEnv POSIX style ${KEY}. if not enable strict, use
  // GetEnvironmentVariableW if key not exists envb
  bool ExpandEnv(std::wstring_view raw, std::wstring &w, bool strict = false) const;
  std::wstring MakeEnv() const;
  // CleanupEnv create cleanup env. you can use bela::env::JoinEnv create it.
  std::wstring CleanupEnv(std::wstring_view prepend_to_path) const;

private:
  bool expandEnvValue(std::wstring_view key, std::wstring &s, bool strict) const {
    if (auto it = envb.find(key); it != envb.end()) {
      s.append(it->second);
      return true;
    }
    if (!strict) {
      return false;
    }
    s.append(bela::GetEnv(std::wstring(key)));
    return true;
  }
  value_type envb;
};

class Simulator {
public:
  Simulator() = default;
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;
  bool InitializeEnv();
  bool InitializeCleanupEnv();
  [[nodiscard]] bool LookupPath(std::wstring_view cmd, std::wstring &exe) const;
  bool ExpandEnv(std::wstring_view raw, std::wstring &w) const;
  // Inline support function

  // AddBashCompatible bash compatible val
  bool AddBashCompatible(int argc, wchar_t *const *argv) {
    for (int i = 0; i < argc; i++) {
      envmap.emplace(bela::AlphaNum(i).Piece(), argv[i]);
    }
    envmap.emplace(L"$", bela::AlphaNum(GetCurrentProcessId()).Piece()); // $$
    envmap.emplace(L"@", GetCommandLineW());                             // $@ -->cmdline
    if (auto userprofile = bela::GetEnv(L"USERPROFILE"); !userprofile.empty()) {
      envmap.emplace(L"HOME", userprofile);
    }
    return true;
  }

  // EraseEnv
  bool EraseEnv(std::wstring_view key) {
    if (auto it = envmap.find(key); it != envmap.end()) {
      envmap.erase(it);
      cachedEnv.clear();
      return true;
    }
    return false;
  }

  // SetEnv
  bool SetEnv(std::wstring_view key, std::wstring_view value, bool force = false) {
    if (force) {
      cachedEnv.clear();
      envmap.insert_or_assign(key, value);
      return true;
    }
    if (envmap.emplace(key, value).second) {
      cachedEnv.clear();
      return true;
    }
    return false;
  }

  // PutEnv
  bool PutEnv(std::wstring_view nv, bool force = false) {
    if (auto pos = nv.find(L'='); pos != std::wstring_view::npos) {
      return SetEnv(nv.substr(0, pos), nv.substr(pos + 1), force);
    }
    return SetEnv(nv, L"", force);
  }

  // LookupEnv
  [[nodiscard]] bool LookupEnv(std::wstring_view key, std::wstring &val) const {
    if (auto it = envmap.find(key); it != envmap.end()) {
      val.assign(it->second);
      return true;
    }
    return false;
  }

  // GetEnv get
  [[nodiscard]] std::wstring_view GetEnv(std::wstring_view key) const {
    std::wstring val;
    if (!LookupEnv(key, val)) {
      return L"";
    }
    return val;
  }

  [[nodiscard]] std::wstring ExpandEnv(std::wstring_view raw) const {
    std::wstring s;
    ExpandEnv(raw, s);
    return s;
  }

  // MakeEnv make environment string
  [[nodiscard]] std::wstring MakeEnv() {
    if (!cachedEnv.empty()) {
      return cachedEnv;
    }
    return makeEnvInternal();
  }

private:
  std::vector<std::wstring> paths;
  std::vector<std::wstring> pathexts;
  envmap_t envmap;
  std::wstring cachedEnv;
  [[nodiscard]] std::wstring makeEnvInternal() {
    constexpr std::wstring_view pathc = L"Path";
    size_t len = pathc.size() + 1; // path=
    for (const auto &s : paths) {
      len += s.size() + 1; // /path/to;
    }
    for (const auto &p : pathexts) {
      len += p.size() + 1; // .exe;
    }
    for (const auto &kv : envmap) {
      len += kv.first.size() + kv.second.size() + 1 + 1;
    }
    len++;
    cachedEnv.reserve(len);
    cachedEnv.assign(L"Path=").append(bela::StrJoin(paths, Separators)).append(L"\0");
    cachedEnv.append(L"PATHEXT=").append(bela::StrJoin(pathexts, Separators)).append(L"\0");
    for (const auto &[name, value] : envmap) {
      cachedEnv.append(name).push_back(L'=');
      cachedEnv.append(value).push_back(L'\0');
    }
    cachedEnv.append(L"\0");
    return cachedEnv;
  }
};

} // namespace env

} // namespace bela

#endif
