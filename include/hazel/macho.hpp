//
#ifndef HAZEL_MACHO_HPP
#define HAZEL_MACHO_HPP
#include "hazel.hpp"

namespace hazel::macho {
class File {
public:
  File(HANDLE fd_) : fd(fd_) {}
  File(const File &) = delete;
  File &operator=(const File &) = delete;

private:
  HANDLE fd{nullfile_t};
};
} // namespace hazel::macho

#endif