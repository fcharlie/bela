///////////
// bela expand env
#include <bela/base.hpp>
#include <bela/env.hpp>
#include <bela/strcat.hpp>
#include <bela/path.hpp>

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

constexpr std::wstring_view env_wstrings[] = {
    L"ALLUSERSPROFILE",
    L"APPDATA",
    L"CommonProgramFiles",
    L"CommonProgramFiles(x86)",
    L"CommonProgramW6432",
    L"COMPUTERNAME",
    L"ComSpec",
    L"HOMEDRIVE",
    L"HOMEPATH",
    L"LOCALAPPDATA",
    L"LOGONSERVER",
    L"NUMBER_OF_PROCESSORS",
    L"OS",
    L"PATHEXT",
    L"PROCESSOR_ARCHITECTURE",
    L"PROCESSOR_ARCHITEW6432",
    L"PROCESSOR_IDENTIFIER",
    L"PROCESSOR_LEVEL",
    L"PROCESSOR_REVISION",
    L"ProgramData",
    L"ProgramFiles",
    L"ProgramFiles(x86)",
    L"ProgramW6432",
    L"PROMPT",
    L"PSModulePath",
    L"PUBLIC",
    L"SystemDrive",
    L"SystemRoot",
    L"TEMP",
    L"TMP",
    L"USERDNSDOMAIN",
    L"USERDOMAIN",
    L"USERDOMAIN_ROAMINGPROFILE",
    L"USERNAME",
    L"USERPROFILE",
    L"windir",
    // Windows Terminal
    L"SESSIONNAME",
    L"WT_SESSION",
    L"WSLENV",
    // Enables proxy information to be passed to Curl, the underlying download
    // library in cmake.exe
    L"http_proxy",
    L"https_proxy",
    // Environment variables to tell git to use custom SSH executable or command
    L"GIT_SSH",
    L"GIT_SSH_COMMAND",
    // Environment variables needed for ssh-agent based authentication
    L"SSH_AUTH_SOCK",
    L"SSH_AGENT_PID",
    // Enables find_package(CUDA) and enable_language(CUDA) in CMake
    L"CUDA_PATH",
    L"CUDA_PATH_V9_0",
    L"CUDA_PATH_V9_1",
    L"CUDA_PATH_V10_0",
    L"CUDA_PATH_V10_1",
    L"CUDA_TOOLKIT_ROOT_DIR",
    // Environmental variable generated automatically by CUDA after installation
    L"NVCUDASAMPLES_ROOT",
    // Enables find_package(Vulkan) in CMake. Environmental variable generated
    // by Vulkan SDK installer
    L"VULKAN_SDK",
    // Enable targeted Android NDK
    L"ANDROID_NDK_HOME",
};

inline bool ExistsEnv(std::wstring_view k) {
  for (const auto s : env_wstrings) {
    if (bela::EqualsIgnoreCase(s, k)) {
      return true;
    }
  }
  return false;
}

} // namespace env_internal

// https://docs.microsoft.com/en-us/windows/desktop/api/processenv/nf-processenv-expandenvironmentstringsw
std::wstring ExpandEnv(std::wstring_view sv) {
  auto pos = sv.find('%');
  if (pos == std::wstring_view::npos) {
    return std::wstring(sv);
  }
  // NO check
  if (sv.find(L'%', pos + 1) == std::wstring_view::npos) {
    return std::wstring(sv);
  }
  std::wstring buf;
  buf.resize(sv.size() + 256);
  auto N = ExpandEnvironmentStringsW(sv.data(), buf.data(), static_cast<DWORD>(buf.size()));
  if (static_cast<size_t>(N) > buf.size()) {
    buf.resize(N);
    N = ExpandEnvironmentStringsW(sv.data(), buf.data(), static_cast<DWORD>(buf.size()));
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

inline bool is_shell_specia_var(wchar_t ch) {
  return (ch == '*' || ch == '#' || ch == '$' || ch == '@' || ch == '!' || ch == '?' || ch == '-' ||
          (ch >= '0' && ch <= '9'));
}

inline bool is_alphanum(wchar_t ch) {
  return (ch == '_' || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
          (ch >= 'A' && ch <= 'Z'));
}

std::wstring_view resovle_shell_name(std::wstring_view s, size_t &off) {
  off = 0;
  if (s.front() == '{') {
    if (s.size() > 2 && is_shell_specia_var(s[1]) && s[2] == '}') {
      off = 3;
      return s.substr(1, 2);
    }
    for (size_t i = 1; i < s.size(); i++) {
      if (s[i] == '}') {
        if (i == 1) {
          off = 2;
          return L"";
        }
        off = i + 1;
        return s.substr(1, i - 1);
      }
    }
    off = 1;
    return L"";
  }
  if (is_shell_specia_var(s[0])) {
    off = 1;
    return s.substr(0, 1);
  }
  size_t i = 0;
  for (; i < s.size() && is_alphanum(s[i]); i++) {
    ;
  }
  off = i;
  return s.substr(0, i);
}

// Expand Env string to normal string only support  Unix style'${KEY}'
bool Derivator::ExpandEnv(std::wstring_view raw, std::wstring &w, bool strict) const {
  w.reserve(raw.size() * 2);
  std::wstring osenv_;
  size_t i = 0;
  for (size_t j = 0; j < raw.size(); j++) {
    if (raw[j] == '$' && j + 1 < raw.size()) {
      w.append(raw.substr(i, j - i));
      size_t off = 0;
      auto name = resovle_shell_name(raw.substr(j + 1), off);
      if (name.empty()) {
        if (off == 0) {
          w.push_back(raw[j]);
        }
      } else {
        expandEnvValue(name, w, strict);
      }
      j += off;
      i = j + 1;
    }
  }
  w.append(raw.substr(i));
  return true;
}

std::wstring Derivator::MakeEnv() const {
  LPWCH envs{nullptr};
  auto deleter = bela::finally([&] {
    if (envs) {
      FreeEnvironmentStringsW(envs);
      envs = nullptr;
    }
  });
  envs = ::GetEnvironmentStringsW();
  if (envs == nullptr) {
    return L"";
  }
  std::wstring ne;
  for (wchar_t const *lastch{envs}; *lastch != '\0'; ++lastch) {
    const auto len = ::wcslen(lastch);
    const std::wstring_view entry{lastch, len};
    const auto pos = entry.find(L'=');
    if (pos == std::wstring_view::npos) {
      lastch += len;
      continue;
    }
    auto key = entry.substr(0, pos);
    if (auto it = envb.find(key); it == envb.end()) {
      ne.append(lastch).push_back(L'\0');
    }
    lastch += len;
  }
  for (const auto &[name, value] : envb) {
    ne.append(name).push_back(L'=');
    ne.append(value).push_back(L'\0');
  }
  ne.push_back('\0');
  return ne;
}

std::wstring Derivator::CleanupEnv(std::wstring_view prepend_to_path) const {
  LPWCH envs{nullptr};
  auto deleter = bela::finally([&] {
    if (envs) {
      FreeEnvironmentStringsW(envs);
      envs = nullptr;
    }
  });
  envs = ::GetEnvironmentStringsW();
  if (envs == nullptr) {
    return L"";
  }
  auto systemroot = bela::GetEnv(L"SystemRoot");
  auto system32_env = bela::StringCat(systemroot, L"\\System32");
  auto newpath = bela::StringCat(
      L"Path=", prepend_to_path, L";",             // prepandpath
      system32_env, L";",                          // C:\\Windows\\System32
      systemroot, L";",                            // C:\\Windows
      system32_env, L"\\Wbem;",                    // C:\\Windows\\System32\\Wbem
      system32_env, L"\\WindowsPowerShell\\v1.0"); // C:\\Windows\\System32\\WindowsPowerShell\\v1.0
  std::wstring ne;
  for (wchar_t const *lastch{envs}; *lastch != '\0'; ++lastch) {
    const auto len = ::wcslen(lastch);
    const std::wstring_view entry{lastch, len};
    const auto pos = entry.find(L'=');
    if (pos == std::wstring_view::npos) {
      lastch += len;
      continue;
    }
    auto key = entry.substr(0, pos);
    if (!env_internal::ExistsEnv(key)) {
      continue;
    }
    if (auto it = envb.find(key); it == envb.end()) {
      ne.append(lastch).push_back(L'\0');
    }
    lastch += len;
  }
  for (const auto &[name, value] : envb) {
    if (bela::EqualsIgnoreCase(name, L"Path")) {
      bela::StrAppend(&newpath, L";", value);
      continue;
    }
    ne.append(name).push_back(L'=');
    ne.append(value).push_back(L'\0');
  }
  ne.append(newpath).push_back(L'\0');
  ne.push_back('\0');
  return ne;
}

bool cleanupPathExt(std::wstring_view pathext, std::vector<std::wstring> &exts) {
  constexpr std::wstring_view defaultexts[] = {L".com", L".exe", L".bat", L".cmd"};
  std::vector<std::wstring_view> exts_ =
      bela::StrSplit(pathext, bela::ByChar(bela::env::Separator), bela::SkipEmpty()); //
  if (exts_.empty()) {
    exts.assign(std::begin(defaultexts), std::end(defaultexts));
    return true;
  }
  for (const auto &e : exts_) {
    if (e.front() != '.') {
      exts.emplace_back(bela::StringCat(L".", bela::AsciiStrToLower(e)));
      continue;
    }
    exts.emplace_back(bela::AsciiStrToLower(e));
  }
  return true;
}

inline bool HasExt(std::wstring_view file) {
  if (auto pos = file.rfind(L'.'); pos != std::wstring_view::npos) {
    return file.find_last_of(L":\\/") < pos;
  }
  return false;
}

bool FindExecutable(std::wstring_view file, const std::vector<std::wstring> &exts,
                    std::wstring &p) {
  if (HasExt(file) && PathFileIsExists(file)) {
    p = file;
    return true;
  }
  std::wstring newfile;
  newfile.reserve(file.size() + 8);
  newfile.assign(file);
  auto rawsize = newfile.size();
  for (const auto &e : exts) {
    // rawsize always < newfile.size();
    // std::char_traits::assign
    newfile.resize(rawsize);
    newfile.append(e);
    if (PathFileIsExists(newfile)) {
      p.assign(std::move(newfile));
      return true;
    }
  }
  return false;
}

bool Simulator::InitializeEnv() {
  LPWCH envs{nullptr};
  auto deleter = bela::finally([&] {
    if (envs) {
      FreeEnvironmentStringsW(envs);
      envs = nullptr;
    }
  });
  envs = ::GetEnvironmentStringsW();
  if (envs == nullptr) {
    return false;
  }
  for (wchar_t const *lastch{envs}; *lastch != '\0'; ++lastch) {
    const auto len = ::wcslen(lastch);
    const std::wstring_view entry{lastch, len};
    const auto pos = entry.find(L'=');
    if (pos == std::wstring_view::npos) {
      lastch += len;
      continue;
    }
    auto key = entry.substr(0, pos);
    auto val = entry.substr(pos + 1);
    lastch += len;
    if (bela::EqualsIgnoreCase(key, L"Path")) {
      std::vector<std::wstring_view> paths_ =
          bela::StrSplit(val, bela::ByChar(bela::env::Separator), bela::SkipEmpty());
      for (const auto p : paths_) {
        paths.emplace_back(bela::PathCat(p));
      }
      continue;
    }
    if (bela::EqualsIgnoreCase(key, L"PATHEXT")) {
      cleanupPathExt(val, pathexts);
      continue;
    }
    envmap.emplace(key, val);
  }
  return true;
}

bool Simulator::InitializeCleanupEnv() {
  LPWCH envs{nullptr};
  auto deleter = bela::finally([&] {
    if (envs) {
      FreeEnvironmentStringsW(envs);
      envs = nullptr;
    }
  });
  envs = ::GetEnvironmentStringsW();
  if (envs == nullptr) {
    return false;
  }
  for (wchar_t const *lastch{envs}; *lastch != '\0'; ++lastch) {
    const auto len = ::wcslen(lastch);
    const std::wstring_view entry{lastch, len};
    const auto pos = entry.find(L'=');
    if (pos == std::wstring_view::npos) {
      lastch += len;
      continue;
    }
    auto key = entry.substr(0, pos);
    auto val = entry.substr(pos + 1);
    lastch += len;
    if (bela::EqualsIgnoreCase(key, L"PATHEXT")) {
      cleanupPathExt(val, pathexts);
      continue;
    }
    if (bela::env_internal::ExistsEnv(key)) {
      envmap.emplace(key, val);
    }
  }
  bela::MakePathEnv(paths);
  return true;
}

[[nodiscard]] bool Simulator::LookupPath(std::wstring_view cmd, std::wstring &exe) const {
  if (cmd.find_first_of(L":\\/") != std::wstring_view::npos) {
    auto ncmd = bela::PathAbsolute(cmd);
    return FindExecutable(ncmd, pathexts, exe);
  }
  auto cwdfile = bela::PathAbsolute(cmd);
  if (FindExecutable(cwdfile, pathexts, exe)) {
    return true;
  }
  for (auto p : paths) {
    auto exefile = bela::StringCat(p, L"\\", cmd);
    if (FindExecutable(exefile, pathexts, exe)) {
      return true;
    }
  }
  return false;
}

bool Simulator::ExpandEnv(std::wstring_view raw, std::wstring &w) const {
  w.reserve(raw.size() * 2);
  size_t i = 0;
  for (size_t j = 0; j < raw.size(); j++) {
    if (raw[j] == '$' && j + 1 < raw.size()) {
      w.append(raw.substr(i, j - i));
      size_t off = 0;
      auto name = resovle_shell_name(raw.substr(j + 1), off);
      if (name.empty()) {
        if (off == 0) {
          w.push_back(raw[j]);
        }
      } else {
        if (auto it = envmap.find(name); it != envmap.end()) {
          w.append(it->second);
        }
      }
      j += off;
      i = j + 1;
    }
  }
  w.append(raw.substr(i));
  return true;
}

bool ExecutableExistsInPath(std::wstring_view cmd, std::wstring &exe,
                            const std::vector<std::wstring> &paths) {
  std::vector<std::wstring> exts;
  cleanupPathExt(bela::GetEnv(L"PATHEXT"), exts);
  if (cmd.find_first_of(L":\\/") != std::wstring_view::npos) {
    auto ncmd = bela::PathAbsolute(cmd);
    return FindExecutable(ncmd, exts, exe);
  }
  auto cwdfile = bela::PathAbsolute(cmd);
  if (FindExecutable(cwdfile, exts, exe)) {
    return true;
  }
  for (auto p : paths) {
    auto exefile = bela::StringCat(p, L"\\", cmd);
    if (FindExecutable(exefile, exts, exe)) {
      return true;
    }
  }
  return false;
}

bool ExecutableExistsInPath(std::wstring_view cmd, std::wstring &exe) {
  std::vector<std::wstring> exts;
  cleanupPathExt(bela::GetEnv(L"PATHEXT"), exts);
  if (cmd.find_first_of(L":\\/") != std::wstring_view::npos) {
    auto ncmd = bela::PathAbsolute(cmd);
    return FindExecutable(ncmd, exts, exe);
  }
  auto cwdfile = bela::PathAbsolute(cmd);
  if (FindExecutable(cwdfile, exts, exe)) {
    return true;
  }
  auto path = GetEnv<4096>(L"PATH"); // 4K suggest.
  std::vector<std::wstring_view> pathv =
      bela::StrSplit(path, bela::ByChar(L';'), bela::SkipEmpty());
  for (auto p : pathv) {
    auto exefile = bela::StringCat(p, L"\\", cmd);
    if (FindExecutable(exefile, exts, exe)) {
      return true;
    }
  }
  return false;
}

} // namespace env
} // namespace bela
