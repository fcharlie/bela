// Bela IO utils
#ifndef BELA_IO_HPP
#define BELA_IO_HPP
#include "base.hpp"
#include "types.hpp"

namespace bela::io {
[[maybe_unused]] constexpr auto MaximumRead = 1024ull * 1024 * 8; // 8MB
[[maybe_unused]] constexpr auto MaximumLineLength = 1024ull * 64; // 64KB
bool ReadFile(std::wstring_view file, std::wstring &out, bela::error_code &ec, uint64_t maxsize = MaximumRead);
bool ReadLine(std::wstring_view file, std::wstring &out, bela::error_code &ec, uint64_t maxline = MaximumLineLength);
inline std::optional<std::wstring> ReadLine(std::wstring_view file, bela::error_code &ec,
                                            uint64_t maxline = MaximumLineLength) {
  std::wstring line;
  if (ReadLine(file, line, ec, maxline)) {
    return std::make_optional(std::move(line));
  }
  return std::nullopt;
}
bool WriteTextU16LE(std::wstring_view text, std::wstring_view file, bela::error_code &ec);
bool WriteText(std::string_view text, std::wstring_view file, bela::error_code &ec);
bool WriteTextAtomic(std::string_view text, std::wstring_view file, bela::error_code &ec);
inline bool WriteText(std::wstring_view text, std::wstring_view file, bela::error_code &ec) {
  return WriteText(bela::ToNarrow(text), file, ec);
}
inline bool WriteText(std::u16string_view text, std::wstring_view file, bela::error_code &ec) {
  return WriteText(bela::ToNarrow(text), file, ec);
}

struct Reader {
  virtual ssize_t Read(void *buffer, size_t len, bela::error_code &ec) = 0;
};

struct ReaderAt : public virtual Reader {
  virtual ssize_t ReadAt(void *buffer, size_t len, int64_t pos, bela::error_code &ec) = 0;
};

struct Writer {
  virtual ssize_t Write(const void *buffer, size_t len, bela::error_code &ec) = 0;
};

struct ReadeWriter : public virtual Reader, public virtual Writer {};

inline ssize_t ReadAtLeast(Reader &r, void *buffer, size_t len, size_t min, bela::error_code &ec) {
  if (len < min) {
    ec = bela::make_error_code(L"short buffer");
    return -1;
  }
  auto p = reinterpret_cast<uint8_t *>(buffer);
  size_t n = 0;
  for (; n < min;) {
    auto nn = r.Read(p + n, len - n, ec);
    if (nn < 0) {
      return -1;
    }
    if (nn == 0) {
      break;
    }
    n += nn;
  }
  if (n < min) {
    ec = bela::make_error_code(L"unexpected EOF");
    return static_cast<ssize_t>(n);
  }
  return static_cast<ssize_t>(n);
}

} // namespace bela::io

#endif