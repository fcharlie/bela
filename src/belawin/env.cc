///////////
// bela expand env
#include <bela/base.hpp>

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

constexpr auto DWORD_MAX = static_cast<DWORD>(-1);
template <DWORD Len = DWORD_MAX> std::wstring GetEnv(std::wstring_view val) {
  std::wstring s;
  DWORD buflen = Len;
  if constexpr (Len == DWORD_MAX) {
    buflen = GetEnvironmentVariableW(val.data(), nullptr, 0);
    if (buflen == 0) {
      return L"";
    }
    s.resize(buflen);
  } else {
    s.resize(Len);
  }
  auto len = GetEnvironmentVariableW(val.data(), s.data(), buflen);
  if (len == 0 || len > buflen) {
    return L"";
  }
  s.resize(len);
  return s;
}

} // namespace env_internal
// https://docs.microsoft.com/en-us/windows/desktop/api/processenv/nf-processenv-expandenvironmentstringsw
// New expand env not require system call
// %SystemRoot%\\System32\\cmd.exe --> C:\\Windows\\System32\\cmd.exe
// %% --> %
// %SystemRoot% -->C:\\Windows (or others)
std::wstring ExpandEnv(std::wstring_view sv) {
  std::wstring buf;
  buf.reserve(sv.size());
  auto it = sv.data();
  auto end = it + sv.size();
  while (it < end) {
    ///  Fast search %,
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
    if (*it == '%') {
      it++;
      buf.push_back('%');
      continue;
    }
    pos = env_internal::memsearch(it, end, '%');
    if (pos == env_internal::npos) {
      buf.append(it, end - it);
      break;
    }
    std::wstring key(it, pos);
    buf.append(env_internal::GetEnv(key));
    it += pos + 1;
  }
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
        buf.append(env_internal::GetEnv<260>(e));
        it += e.size() + 1;
      }
    }
  }
  return buf;
}

} // namespace bela