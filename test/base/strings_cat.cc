///
#include <bela/__strings/string_cat_internal.hpp>
#include <bela/terminal.hpp>
#include <filesystem>
#include "strings_cat.hpp"

int wmain(int argc, wchar_t **argv) {
  std::error_code ec;
  constexpr char32_t blueheart = U'💙';
  auto arg0 = std::filesystem::absolute(argv[0], ec);
  bela::basic_alphanum<wchar_t> was[] = {(std::numeric_limits<uint64_t>::max)(),
                                         (std::numeric_limits<uint64_t>::min)(),
                                         (std::numeric_limits<int64_t>::max)(),
                                         (std::numeric_limits<int64_t>::min)(),
                                         (std::numeric_limits<uint32_t>::max)(),
                                         (std::numeric_limits<uint32_t>::min)(),
                                         (std::numeric_limits<int32_t>::max)(),
                                         (std::numeric_limits<int32_t>::min)(),
                                         (std::numeric_limits<uint16_t>::max)(),
                                         (std::numeric_limits<uint16_t>::min)(),
                                         (std::numeric_limits<int16_t>::max)(),
                                         (std::numeric_limits<int16_t>::min)(),
                                         1,
                                         1.2f,
                                         L"hello world",
                                         u"hello world",
                                         u'\u00a9',
                                         L'我',
                                         L"这是一段简单的文字",
                                         blueheart,
                                         arg0,
                                         bela::Hex((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                         bela::Hex((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20),
                                         bela::Dec((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                         bela::Dec((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20)};
  for (const auto &a : was) {
    bela::FPrintF(stderr, L"wchar_t: %v\n", a.Piece());
  }
  bela::basic_alphanum<char16_t> uas[] = {(std::numeric_limits<uint64_t>::max)(),
                                          (std::numeric_limits<uint64_t>::min)(),
                                          (std::numeric_limits<int64_t>::max)(),
                                          (std::numeric_limits<int64_t>::min)(),
                                          (std::numeric_limits<uint32_t>::max)(),
                                          (std::numeric_limits<uint32_t>::min)(),
                                          (std::numeric_limits<int32_t>::max)(),
                                          (std::numeric_limits<int32_t>::min)(),
                                          (std::numeric_limits<uint16_t>::max)(),
                                          (std::numeric_limits<uint16_t>::min)(),
                                          (std::numeric_limits<int16_t>::max)(),
                                          (std::numeric_limits<int16_t>::min)(),
                                          1,
                                          1.2f,
                                          L"hello world",
                                          u"hello world",
                                          u'\u00a9',
                                          L'我',
                                          L"这是一段简单的文字",
                                          blueheart,
                                          arg0,
                                          bela::Hex((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                          bela::Hex((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20),
                                          bela::Dec((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                          bela::Dec((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20)};
  for (const auto &a : uas) {
    bela::FPrintF(stderr, L"char16_t: %v\n", a.Piece());
  }
  bela::basic_alphanum<char8_t> u8as[] = {(std::numeric_limits<uint64_t>::max)(),
                                          (std::numeric_limits<uint64_t>::min)(),
                                          (std::numeric_limits<int64_t>::max)(),
                                          (std::numeric_limits<int64_t>::min)(),
                                          (std::numeric_limits<uint32_t>::max)(),
                                          (std::numeric_limits<uint32_t>::min)(),
                                          (std::numeric_limits<int32_t>::max)(),
                                          (std::numeric_limits<int32_t>::min)(),
                                          (std::numeric_limits<uint16_t>::max)(),
                                          (std::numeric_limits<uint16_t>::min)(),
                                          (std::numeric_limits<int16_t>::max)(),
                                          (std::numeric_limits<int16_t>::min)(),
                                          1,
                                          1.2f,
                                          u8"hello world",
                                          "hello world",
                                          u8'U',
                                          'x',
                                          U'我',
                                          u8"这是一段简单的文字",
                                          blueheart,
                                          bela::Hex((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                          bela::Hex((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20),
                                          bela::Dec((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                          bela::Dec((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20)};
  for (const auto &a : u8as) {
    bela::FPrintF(stderr, L"char8_t: %v\n", a.Piece());
  }
  bela::basic_alphanum<char> cas[] = {(std::numeric_limits<uint64_t>::max)(),
                                      (std::numeric_limits<uint64_t>::min)(),
                                      (std::numeric_limits<int64_t>::max)(),
                                      (std::numeric_limits<int64_t>::min)(),
                                      (std::numeric_limits<uint32_t>::max)(),
                                      (std::numeric_limits<uint32_t>::min)(),
                                      (std::numeric_limits<int32_t>::max)(),
                                      (std::numeric_limits<int32_t>::min)(),
                                      (std::numeric_limits<uint16_t>::max)(),
                                      (std::numeric_limits<uint16_t>::min)(),
                                      (std::numeric_limits<int16_t>::max)(),
                                      (std::numeric_limits<int16_t>::min)(),
                                      1,
                                      1.97821741f,
                                      u8"hello world",
                                      "hello world",
                                      u8'U',
                                      'x',
                                      U'我',
                                      "这是一段简单的文字",
                                      blueheart,
                                      bela::Hex((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                      bela::Hex((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20),
                                      bela::Dec((std::numeric_limits<int32_t>::max)(), bela::kZeroPad20),
                                      bela::Dec((std::numeric_limits<int32_t>::min)(), bela::kZeroPad20)};
  for (const auto &a : cas) {
    bela::FPrintF(stderr, L"char: %v\n", a.Piece());
  }
  constexpr char32_t em = 0x1F603; // 😃 U+1F603
  auto s = bela::strings_cat<wchar_t>(L"Look emoji --> ", em, L" see: U+",
                                      bela::basic_alphanum<wchar_t>(bela::Hex(static_cast<uint32_t>(em))));
  bela::FPrintF(stderr, L"%s\n", s);
  auto s2 = bela::strings_cat<wchar_t, std::allocator<wchar_t>>(
      L"Look emoji --> ", em, L" see: U+", bela::basic_alphanum<wchar_t>(bela::Hex(static_cast<uint32_t>(em))),
      L" jacksome");
  bela::FPrintF(stderr, L"%s\n", s2);
  return 0;
}