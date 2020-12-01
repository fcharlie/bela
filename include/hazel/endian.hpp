///
#ifndef HAZEL_ENDIAN_HPP
#define HAZEL_ENDIAN_HPP
#include <bela/endian.hpp>

namespace hazel::endian {

#if IS_BIG_ENDIAN
enum class Endian { little = 0, big = 1, native = big };
#else
enum class Endian { little = 0, big = 1, native = little };
#endif

template <Endian E = Endian::native> class Reader {
public:
  Reader(const void *p, size_t len) : data(reinterpret_cast<const uint8_t *>(p)), size(len) {}
  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;
  template <typename T> T Read() {
    auto p = reinterpret_cast<const T *>(data);
    data += sizeof(T);
    size -= sizeof(T);
    if constexpr (E == Endian::native) {
      return *p;
    }
    return bela::bswap(*p);
  }
  size_t Discard(size_t n) {
    if (n >= size) {
      data = nullptr;
      size = 0;
    }
    size -= n;
    data += n;
    return size;
  }
  size_t Size() const { return size; }

private:
  Endian e{E};
  const uint8_t *data;
  size_t size;
};

} // namespace hazel::endian

#endif