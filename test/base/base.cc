#include <bela/base.hpp>
#include <bela/terminal.hpp>
#include <bela/str_cat_narrow.hpp>
#include <bela/win32.hpp>
#include <filesystem>
#include <numeric>

void string_no_const_print_v0() {
  char *s1 = nullptr;
  char8_t *s2 = nullptr;
  wchar_t *s3 = nullptr;
  char16_t *s4 = nullptr;
  // char32_t s5[32] = U"中国馆";
  bela::FPrintF(stderr, L"string_no_const_print: %s %s %s %s \n", s1, s2, s3, s4);
}

void string_no_const_print() {
  char s1[32] = "hello world";
  char8_t s2[32] = u8"hello world 2";
  wchar_t s3[32] = L"中国";
  char16_t s4[32] = u"中国";
  char32_t s5[32] = U"🎉💖✔️💜😁😂😊🤣❤️😍😒👌";
  bela::FPrintF(stderr, L"string_no_const_print: %s %s %s %s %s\n", s1, s2, s3, s4, s5);
}

void string_const_print() {
  constexpr const char *s1 = "hello world";
  constexpr const char8_t *s2 = u8"hello world 2";
  constexpr const wchar_t *s3 = L"中国";
  constexpr const char16_t *s4 = u"中国";
  constexpr const char32_t *s5 = U"🎉💖✔️💜😁😂😊🤣❤️😍😒👌";
  bela::FPrintF(stderr, L"string_const_print: %s %s %s %s %s\n", s1, s2, s3, s4, s5);
}

void string_view_print() {
  std::string_view s1 = "hello world";
  std::u8string_view s2 = u8"hello world 2";
  std::wstring_view s3 = L"中国";
  std::u16string_view s4 = u"中国";
  std::u32string_view s5 = U"🎉💖✔️💜😁😂😊🤣❤️😍😒👌";
  bela::FPrintF(stderr, L"string_view_print: %s %s %s %s %s\n", s1, s2, s3, s4, s5);
}

void string_print() {
  std::string s1 = "hello world";
  std::u8string s2 = u8"hello world 2";
  std::wstring s3 = L"中国";
  std::u16string s4 = u"中国";
  std::u32string s5 = U"🎉💖✔️💜😁😂😊🤣❤️😍😒👌";
  bela::FPrintF(stderr, L"string_print: %s %s %s %s %s\n", s1, s2, s3, s4, s5);
}

void print() {
  bela::FPrintF(stderr, L"short: %v - %v\n", (std::numeric_limits<short>::min)(),
                (std::numeric_limits<short>::max)()); //
  bela::FPrintF(stderr, L"unsigned short: %v - %v\n", (std::numeric_limits<unsigned short>::min)(),
                (std::numeric_limits<unsigned short>::max)()); //
  bela::FPrintF(stderr, L"int: %v - %v\n", (std::numeric_limits<int>::min)(),
                (std::numeric_limits<int>::max)()); //
  bela::FPrintF(stderr, L"unsigned int: %v - %v\n", (std::numeric_limits<unsigned int>::min)(),
                (std::numeric_limits<unsigned int>::max)()); //
  bela::FPrintF(stderr, L"long: %v - %v\n", (std::numeric_limits<long>::min)(),
                (std::numeric_limits<long>::max)()); //
  bela::FPrintF(stderr, L"unsigned long: %v - %v\n", (std::numeric_limits<unsigned long>::min)(),
                (std::numeric_limits<unsigned long>::max)()); //
  bela::FPrintF(stderr, L"long long: %v - %v\n", (std::numeric_limits<long long>::min)(),
                (std::numeric_limits<long long>::max)()); //
  bela::FPrintF(stderr, L"unsigned long long: %v - %v\n", (std::numeric_limits<unsigned long long>::min)(),
                (std::numeric_limits<unsigned long long>::max)()); //
  bela::FPrintF(stderr, L"float: %v - %v\n", (std::numeric_limits<float>::min)(),
                (std::numeric_limits<float>::max)()); //
  bela::FPrintF(stderr, L"double: %v - %v\n", (std::numeric_limits<double>::min)(),
                (std::numeric_limits<double>::max)()); //
}

int wmain(int argc, wchar_t **argv) {
  string_print();
  string_view_print();
  string_no_const_print();
  string_const_print();
  string_no_const_print_v0();
  print();
  std::filesystem::path p{argv[0]};
  constexpr char32_t sh = 0x1F496; //  💖
  auto xx = 199.9654321f;
  bela::FPrintF(stderr, L"usage: %v [%4c] %.2f\n", p, sh, xx);

  if (argc >= 2) {
    FILE *fd = nullptr;

    if (auto e = _wfopen_s(&fd, argv[1], L"rb"); e != 0) {
      auto ec = bela::make_error_code_from_errno(e);
      bela::FPrintF(stderr, L"unable open: %s\n", ec, fd);
      return 1;
    }
    fclose(fd);
  }
  auto ec = bela::make_error_code(bela::ErrCanceled, L"canceled");
  if (ec) {
    bela::FPrintF(stderr, L"Err: %v\n", ec);
  }
  auto ec2 = bela::make_error_code(bela::ErrCanceled, L"cancelede ccc");
  bela::FPrintF(stderr, L"Equal: %v\n", ec == bela::ErrCanceled);
  bela::FPrintF(stderr, L"Equal: %v\n", ec == ec2);
  bela::FPrintF(stderr, L"%s\n", bela::narrow::StringCat("H: ", bela::narrow::AlphaNum(bela::narrow::Hex(123456))));
  bela::FPrintF(stderr, L"EADDRINUSE: %s\nEWOULDBLOCK: %s\n", bela::make_error_code_from_errno(EADDRINUSE),
                bela::make_error_code_from_errno(EWOULDBLOCK));
  auto version = bela::windows::version();
  bela::FPrintF(stderr, L"%d.%d.%d %d.%d\n", version.major, version.minor, version.build, version.service_pack_major,
                version.service_pack_minor);
  return 0;
}