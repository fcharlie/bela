#include <bela/base.hpp>
#include <bela/terminal.hpp>
#include <bela/charconv.hpp>

void check_wchar_t_view() {
  bela::FPrintF(stderr, L"check %v\n", __FUNCTIONW__);
  constexpr std::wstring_view i64s[] = {L"5782112",
                                        L"-1",
                                        L"-987654321",
                                        L"9223372036854775807",
                                        L"-9223372036854775807",
                                        L"541",
                                        L"18446744073709551615"};
  for (auto s : i64s) {
    int64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    wchar_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }

  for (auto s : i64s) {
    uint64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    wchar_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }
}

void check_char_view() {
  bela::FPrintF(stderr, L"check %v\n", __FUNCTIONW__);
  constexpr std::string_view i64s[] = {
      "5782112", "-1", "-987654321", "9223372036854775807", "-9223372036854775807", "541", "18446744073709551615"};
  for (auto s : i64s) {
    int64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }

  for (auto s : i64s) {
    uint64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }
}

void char_char16_t_view() {
  bela::FPrintF(stderr, L"check %v\n", __FUNCTIONW__);
  constexpr std::u16string_view i64s[] = {u"5782112",
                                          u"-1",
                                          u"-987654321",
                                          u"9223372036854775807",
                                          u"-9223372036854775807",
                                          u"541",
                                          u"18446744073709551615"};
  for (auto s : i64s) {
    int64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char16_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }

  for (auto s : i64s) {
    uint64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char16_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }
}

void check_char8_t_view() {
  bela::FPrintF(stderr, L"check %v\n", __FUNCTIONW__);
  constexpr std::u8string_view i64s[] = {u8"5782112",
                                         u8"-1",
                                         u8"-987654321",
                                         u8"9223372036854775807",
                                         u8"-9223372036854775807",
                                         u8"541",
                                         u8"18446744073709551615"};
  for (auto s : i64s) {
    int64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char8_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }

  for (auto s : i64s) {
    uint64_t i = 0;
    if (auto r = bela::from_string_view(s, i); r.ec != std::errc{}) {
      bela::FPrintF(stderr, L"from_chars %s error: %v\n", s, bela::integral_cast(r.ec));
      continue;
    }
    char8_t buffer[64];
    bela::FPrintF(stderr, L"%d to number: %v\n", i, bela::to_chars_view(buffer, i, 10));
    bela::FPrintF(stderr, L"%d to hex: %v\n", i, bela::to_chars_view(buffer, i, 16));
    bela::FPrintF(stderr, L"%d to Octal: %v\n", i, bela::to_chars_view(buffer, i, 8));
    bela::FPrintF(stderr, L"%d to 36: %v\n", i, bela::to_chars_view(buffer, i, 36));
  }
}

void check_base_convert() {
  constexpr struct {
    std::wstring_view sv;
    int base;
  } svs[]{{
              .sv = L"18446744073709551615",
              .base = 10,
          },
          {
              .sv = L"ffffffffffffffff",
              .base = 16,
          },
          {
              .sv = L"1777777777777777777777",
              .base = 8,
          },
          {
              .sv = L"3w5e11264sgsf",
              .base = 36,
          }};
  for (const auto &s : svs) {
    uint64_t i = 0;
    if (auto res = bela::from_string_view(s.sv, i, s.base); res) {
      bela::FPrintF(stderr, L"%s --> %d\n", s.sv, i);
    }
  }
}

void check_base_convert_2() {
  constexpr struct {
    std::wstring_view sv;
    int base;
  } svs[]{{
              .sv = L"9223372036854775807",
              .base = 10,
          },
          {
              .sv = L"7fffffffffffffff",
              .base = 16,
          },
          {
              .sv = L"777777777777777777777",
              .base = 8,
          },
          {
              .sv = L"1y2p0ij32e8e7",
              .base = 36,
          },
          {
              .sv = L"-9223372036854775807",
              .base = 10,
          },
          {
              .sv = L"-7fffffffffffffff",
              .base = 16,
          },
          {
              .sv = L"-777777777777777777777",
              .base = 8,
          },
          {
              .sv = L"-1y2p0ij32e8e7",
              .base = 36,
          }};
  for (const auto &s : svs) {
    int64_t i = 0;
    if (auto res = bela::from_string_view(s.sv, i, s.base); res) {
      bela::FPrintF(stderr, L"%s --> %d\n", s.sv, i);
    }
  }
}

int wmain() {
  check_wchar_t_view();
  check_char_view();
  char_char16_t_view();
  check_char8_t_view();
  check_base_convert();
  check_base_convert_2();
  auto e = std::errc{};
  bela::FPrintF(stderr, L"std::errc{} %d\n", bela::integral_cast(e));
  return 0;
}
