//
#include <bela/env.hpp>
#include <bela/stdwriter.hpp>

int wmain() {
  const wchar_t *svv[] = {L"%SystemRoot%\\System32\\cmd.exe", L"%%%%-----",
                          L"----------%-------------"};
  for (auto s : svv) {
    auto es = bela::ExpandEnv(s);
    bela::FPrintF(stderr, L"[%s] Expand to [%s]\n", s, es);
  }
  auto ss = bela::PathUnExpand(L"%APPDATA%\\Microsoft\\WindowsApp\\wt.exe");
 bela::FPrintF(stderr, L"PathUnExpand [%s]\n", ss);
  return 0;
}