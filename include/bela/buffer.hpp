///
#ifndef BELA_BUFFER_HPP
#define BELA_BUFFER_HPP
#include <memory>
#include <optional>
#include <span>
#include <string>
#include "endian.hpp"
#include "types.hpp"

namespace bela {
// Buffer
namespace buffer {
template <typename Allocator = std::allocator<uint8_t>> class Buffer {
private:
  typedef Allocator allocator_type;
  allocator_type get_allocator() const noexcept { return alloc_; }
  void MoveFrom(Buffer &&other) {
    Free(); // Free self
    data_ = other.data_;
    alloc_ = std::move(other.alloc_);
    other.data_ = nullptr;
    capacity_ = other.capacity_;
    other.capacity_ = 0;
    size_ = other.size_;
    other.size_ = 0;
  }
  void Free() {
    if (data_ != nullptr) {
      get_allocator().deallocate(data_, capacity_);
      data_ = nullptr;
      capacity_ = 0;
      size_ = 0;
    }
  }
  allocator_type alloc_;

public:
  Buffer() = default;
  Buffer(size_t maxsize) { grow(maxsize); }
  Buffer(Buffer &&other) { MoveFrom(std::move(other)); }
  Buffer &operator=(Buffer &&other) {
    MoveFrom(std::move(other));
    return *this;
  }
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;
  ~Buffer() { Free(); }
  [[nodiscard]] size_t size() const { return size_; }
  [[nodiscard]] size_t &size() { return size_; }
  [[nodiscard]] size_t capacity() const { return capacity_; }
  void grow(size_t n) {
    if (n <= capacity_) {
      return;
    }
    auto b = get_allocator().allocate(n);
    if (size_ != 0) {
      memcpy(b, data_, size_);
    }
    if (data_ != nullptr) {
      get_allocator().deallocate(data_, capacity_);
    }
    data_ = b;
    capacity_ = n;
  }
  std::string_view substr(size_t pos = 0) const {
    if (pos > size_) {
      return std::string_view();
    }
    return std::string_view{reinterpret_cast<const char *>(data_) + pos, size_ - pos};
  }
  std::string_view cstring_view(size_t offset, size_t cslength = std::string_view::npos) const {
    if (offset > size_) {
      return std::string_view();
    }
    cslength = (std::min)(cslength, size_);
    auto p = data_ + cslength;
    if (auto end = memchr(data_, 0, cslength); end != nullptr) {
      return std::string_view(reinterpret_cast<const char *>(p), end - p);
    }
    return std::string_view();
  }
  template <typename T> const T *direct_cast(size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(data_ + offset);
  }
  template <typename T> const T *bit_cast(T *t, size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return nullptr;
    }
    return reinterpret_cast<T *>(memcpy(t, data_ + offset, sizeof(T)));
  }
  template <typename T>
  requires std::integral<T> T cast_fromle(size_t offset)
  const {
    if (offset + sizeof(T) > size_) {
      return 0;
    }
    return bela::cast_fromle<T>(data_ + offset);
  }

  template <typename I> [[nodiscard]] const I *cast() const { return reinterpret_cast<const I *>(data_); }
  [[nodiscard]] const uint8_t *data() const { return data_; }
  [[nodiscard]] uint8_t operator[](const size_t _Off) const noexcept { return *(data_ + _Off); }
  [[nodiscard]] uint8_t *data() { return data_; }
  std::span<uint8_t> MakeSpan() const { return std::span(data_, data_ + size_); }

private:
  uint8_t *data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};
} // namespace buffer
using Buffer = bela::buffer::Buffer<std::allocator<uint8_t>>;
} // namespace bela

#endif