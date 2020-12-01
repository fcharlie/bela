//
#ifndef BELA_BUFIO_HPP
#define BELA_BUFIO_HPP
#include "base.hpp"

namespace bela::bufio {
constexpr int default_buffer_size = 4096;
// Fixed capacity size bufio.Reader implementation
template <int Size = default_buffer_size> class Reader {
public:
  Reader(HANDLE r) : fd(r) {}
  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;
  int Read(void *buffer, int len, bela::error_code &ec) {
    //
    return -1;
  }
  constexpr int size() const { return Size; }
  bela::error_code &error_code() const { return e; }

private:
  HANDLE fd;
  uint8_t data[Size];
  bela::error_code e;
  int w{0};
  int r{0};
  int cap{0};
  bool fsread(void *b, int len, int &rlen, bela::error_code &ec);
};
} // namespace bela::bufio

#endif