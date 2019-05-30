///
/// unicode escape L"CH\u2082O\u2083" => L"CH₂O"
#include <bela/escaping.hpp>
#include <bela/stdwriter.hpp>

int wmain() {
  const wchar_t *msg =
      L"H\\u2082O \\U0001F496 \\xA9 \\x1b[32mcolour escape\\x1b[0m";
  std::wstring dest;
  if (bela::CUnescape(msg, &dest)) {
    bela::FPrintF(stderr, L"%s\n%s\n", msg, dest);
  }
  return 0;
}
