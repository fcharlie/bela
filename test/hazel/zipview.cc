//
#include <hazel/hazel.hpp>
#include <hazel/zip.hpp>
#include <bela/terminal.hpp>

int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s zipfile\n", argv[0]);
    return 1;
  }
  hazel::File file;
  bela::error_code ec;
  if (!file.NewFile(argv[1], ec)) {
    bela::FPrintF(stderr, L"unable openfile: %s %s\n", argv[1], ec.message);
    return 1;
  }
  hazel::FileAttributeTable fat;
  if (!file.Lookup(fat, ec)) {
    bela::FPrintF(stderr, L"unable detect file type: %s %s\n", argv[1], ec.message);
    return 1;
  }
  if (!fat.LooksLikeZIP()) {
    bela::FPrintF(stderr, L"file: %s not zip file\n", argv[1]);
    return 1;
  }
  auto zr = hazel::zip::NewReader(file.FD(), ec);
  if (!zr) {
    bela::FPrintF(stderr, L"open zip file: %s error %s\n", file.FullPath(), ec.message);
    return 1;
  }
  if (!zr->Comment().empty()) {
    bela::FPrintF(stderr, L"comment: %s\n", zr->Comment());
  }
  bela::FPrintF(stderr, L"Files: %d\n", zr->Files().size());
  for (const auto &file : zr->Files()) {
    if (file.IsEncrypted()) {
      bela::FPrintF(stderr, L"File: %s (%s %s) %d\n", file.name, hazel::zip::Method(file.method), file.AesText(),
                    file.uncompressedSize);
      continue;
    }
    bela::FPrintF(stderr, L"File: %s (%s) %d\n", file.name, hazel::zip::Method(file.method), file.uncompressedSize);
  }
  return 0;
}