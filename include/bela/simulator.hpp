//
#ifndef BELA_SIMULATOR_HPP
#define BELA_SIMULATOR_HPP
#include "env.hpp"
//

namespace bela::env {
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

std::wstring ExpandEnv(std::wstring_view sv);

using envmap_t = bela::flat_hash_map<std::wstring, std::wstring, StringCaseInsensitiveHash,
                                     StringCaseInsensitiveEq>;
class Simulator {
public:
  Simulator() = default;
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;
  bool InitializeEnv();
  bool InitializeCleanupEnv();
  bool LookupPath(std::wstring_view cmd, std::wstring &exe) const;
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

  bool AppendEnv(std::wstring_view key, std::wstring_view val) {
    if (key.empty() || val.empty()) {
      return false;
    }
    cachedEnv.clear();
    if (bela::EqualsIgnoreCase(key, L"Path")) {
      paths.emplace_back(val);
      return true;
    }
    if (bela::EqualsIgnoreCase(key, L"PATHEXT")) {
      pathexts.emplace_back(val);
      return true;
    }
    if (auto it = envmap.find(key); it != envmap.end()) {
      it->second.append(bela::Separators).append(val);
      return true;
    }
    envmap[key] = val;
    return true;
  }
  bool Insert(std::wstring_view key, std::wstring_view val) {
    if (key.empty() || val.empty()) {
      return false;
    }
    cachedEnv.clear();
    if (bela::EqualsIgnoreCase(key, L"Path")) {
      std::vector<std::wstring> paths_;
      paths_.reserve(paths.size() + 1);
      paths_.emplace_back(val);
      paths_.insert(paths.end(), paths.begin(), paths.end());
      paths = std::move(paths_);
      return true;
    }
    if (bela::EqualsIgnoreCase(key, L"PATHEXT")) {
      std::vector<std::wstring> exts_;
      exts_.reserve(pathexts.size() + 1);
      exts_.emplace_back(val);
      exts_.insert(pathexts.end(), pathexts.begin(), pathexts.end());
      pathexts = std::move(exts_);
      return true;
    }
    if (auto it = envmap.find(key); it != envmap.end()) {
      auto s = bela::StringCat(val, bela::Separators, it->second);
      it->second = s;
      return true;
    }
    envmap[key] = val;
    return true;
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
    return makeInternalEnv();
  }

private:
  std::vector<std::wstring> paths;
  std::vector<std::wstring> pathexts;
  envmap_t envmap;
  std::wstring cachedEnv;
  [[nodiscard]] std::wstring makeInternalEnv() {
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
    // DONT use append NULL string
    cachedEnv.assign(L"Path=").append(bela::StrJoin(paths, Separators)).push_back(L'\0');
    cachedEnv.append(L"PATHEXT=").append(bela::StrJoin(pathexts, Separators)).push_back(L'\0');
    for (const auto &[name, value] : envmap) {
      cachedEnv.append(name).push_back(L'=');
      cachedEnv.append(value).push_back(L'\0');
    }
    cachedEnv.append(L"\0");
    return cachedEnv;
  }
};

bool ExecutableExistsInPath(std::wstring_view cmd, std::wstring &exe);
bool ExecutableExistsInPath(std::wstring_view cmd, std::wstring &exe,
                            const std::vector<std::wstring> &paths);
} // namespace bela::env

#endif