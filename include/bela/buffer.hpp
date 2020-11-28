///
#ifndef BELA_BUFFER_HPP
#define BELA_BUFFER_HPP
#include <memory>
#include <optional>

namespace bela {
namespace buffer {
template <typename T = uint8_t, typename Allocator = std::allocator<T>> class Buffer {
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
    }
  }
  allocator_type alloc_;

public:
  Buffer(size_t maxsize) {
    data_ = get_allocator().allocate(maxsize);
    capacity_ = maxsize;
    size_ = 0;
  }
  Buffer(Buffer &&other) { MoveFrom(std::move(other)); }
  Buffer &operator=(Buffer &&other) {
    MoveFrom(std::move(other));
    return *this;
  }
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;
  ~Buffer() {}
  size_t size() const { return size_; }
  size_t &size() { return size_; }
  size_t capacity() const { return capacity_; }
  template <typename I> const I *cast() const { return reinterpret_cast<const I *>(data_); }
  const T *data() const { return data_; }
  T *data() { return data_; }

private:
  T *data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};
} // namespace buffer
using Buffer = bela::buffer::Buffer<uint8_t, std::allocator<uint8_t>>;
} // namespace bela

#endif