#ifndef BELA_BYTES_VIEW_HPP
#define BELA_BYTES_VIEW_HPP
#include <string_view>
#include <span>
#include "types.hpp"

namespace bela {
class bytes_view {
public:
  static constexpr size_t npos = static_cast<size_t>(-1);
  bytes_view() = default;
  template <typename T>
  requires bela::standard_layout<T> bytes_view(const T *d, size_t l)
      : data_(reinterpret_cast<const uint8_t *>(d)), size_(l * sizeof(T)) {}
  template <typename T>
  requires bela::standard_layout<T> bytes_view(std::span<const T> d)
      : data_(reinterpret_cast<const uint8_t *>(d.data())), size_(d.size() * sizeof(T)) {}
  bytes_view(const bytes_view &other) {
    data_ = other.data_;
    size_ = other.size_;
  }
  bytes_view &operator=(const bytes_view &other) {
    data_ = other.data_;
    size_ = other.size_;
    return *this;
  }
  template <typename T, size_t ArrayLen>
  requires bela::standard_layout<T>
  [[nodiscard]] bool starts_with(const T (&bv)[ArrayLen]) const {
    return ArrayLen <= size_ && (memcmp(data_, bv, ArrayLen * sizeof(T)) == 0);
  }
  [[nodiscard]] bool starts_with(std::string_view sv) const {
    return sv.size() <= size_ && (memcmp(data_, sv.data(), sv.size()) == 0);
  }
  [[nodiscard]] bool starts_with(std::u8string_view sv) const {
    return sv.size() <= size_ && (memcmp(data_, sv.data(), sv.size()) == 0);
  }
  [[nodiscard]] bool starts_with(const void *p, size_t n) { return (n <= size_ && memcmp(data_, p, n) == 0); }

  template <typename T, size_t ArrayLen>
  requires bela::standard_layout<T>
  [[nodiscard]] bool match_with(size_t pos, const T (&bv)[ArrayLen]) const {
    return ArrayLen * sizeof(T) + pos <= size_ && (memcmp(data_ + pos, bv, ArrayLen) == 0);
  }
  [[nodiscard]] bool match_with(size_t pos, std::string_view sv) const {
    return sv.size() + pos <= size_ && (memcmp(data_ + pos, sv.data(), sv.size()) == 0);
  }
  [[nodiscard]] bool match_with(size_t pos, std::u8string_view sv) const {
    return sv.size() + pos <= size_ && (memcmp(data_ + pos, sv.data(), sv.size()) == 0);
  }
  [[nodiscard]] bool match_with(size_t pos, const void *p, size_t n) const {
    return n + pos <= size_ && (memcmp(data_ + pos, p, n) == 0);
  }

  [[nodiscard]] bytes_view subview(std::size_t pos, std::size_t n = npos) const {
    return bytes_view(data_ + pos, (std::min)(n, size_ - pos));
  }
  [[nodiscard]] auto size() const { return size_; }
  [[nodiscard]] const auto *data() const { return data_; }
  [[nodiscard]] auto make_cstring_view(size_t offset, size_t cslength = std::string_view::npos) const {
    if (offset > size_) {
      return std::string_view();
    }
    cslength = (std::min)(cslength, size_);
    auto p = data_ + offset;
    if (auto end = memchr(p, 0, cslength); end != nullptr) {
      return std::string_view(reinterpret_cast<const char *>(p), reinterpret_cast<const uint8_t *>(end) - p);
    }
    return std::string_view(reinterpret_cast<const char *>(p), cslength);
  }
  template <typename T = char>
  requires bela::character<T>
  [[nodiscard]] auto make_string_view(const size_t offset = 0) const {
    if (offset > size_) {
      return std::basic_string_view<T>();
    }
    return std::basic_string_view<T>(reinterpret_cast<const T *>(data_) + offset, (size_ - offset) / sizeof(T));
  }
  [[nodiscard]] auto operator[](const std::size_t off) const {
    if (off >= size_) {
      return UINT8_MAX;
    }
    return data_[off];
  }
  [[nodiscard]] auto make_span() const { return std::span{data_, size_}; }
  template <typename T>
  requires bela::standard_layout<T>
  [[nodiscard]] const T *unsafe_cast() const { return reinterpret_cast<const T *>(data_); }
  template <typename T>
  requires bela::standard_layout<T>
  [[nodiscard]] const T *direct_cast(size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(data_ + offset);
  }
  template <typename T>
  requires bela::standard_layout<T>
  [[nodiscard]] const T *bit_cast(T *t, size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return nullptr;
    }
    return reinterpret_cast<T *>(memcpy(t, data_ + offset, sizeof(T)));
  }
  template <typename T>
  requires std::integral<T>
  [[nodiscard]] T cast_fromle(size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return 0;
    }
    return bela::cast_fromle<T>(data_ + offset);
  }
  template <typename T>
  requires std::integral<T>
  [[nodiscard]] T cast_frombe(size_t offset) const {
    if (offset + sizeof(T) > size_) {
      return 0;
    }
    return bela::cast_frombe<T>(data_ + offset);
  }

private:
  const uint8_t *data_{nullptr};
  size_t size_{0};
};

} // namespace bela

#endif