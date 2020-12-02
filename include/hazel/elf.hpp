//
#ifndef HAZEL_ELF_HPP
#define HAZEL_ELF_HPP
#include "hazel.hpp"
#include <bela/endian.hpp>

namespace hazel::elf {
#pragma pack(1)
struct FileHeader {
  uint8_t Class;
  uint8_t Data;
  uint8_t Version;
  uint8_t OSABI;
  uint8_t ABIVersion;
  uint8_t LSB;
  uint16_t Type;
  uint16_t Machine;
  uint64_t Entry;
};
#pragma pack()

struct Section {
  std::string Name;
  uint32_t Type{0};
  uint32_t Flags{0};
  uint64_t Addr{0};
  uint64_t Offset{0};
  uint64_t Size{0};
  uint32_t Link{0};
  uint32_t Info{0};
  uint64_t Addralign{0};
  uint64_t Entsize{0};
};

class File {
public:
  File(HANDLE fd_, int64_t size_) : fd(fd_), size(size_) {}
  File(const File &) = delete;
  File &operator=(const File &) = delete;

private:
  HANDLE fd{nullfile_t};
  int64_t size{0};
  bela::endian::Endian en{bela::endian::Endian::native};
};
} // namespace hazel::elf

#endif