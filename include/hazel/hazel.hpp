//
#ifndef HAZEL_HAZEL_HPP
#define HAZEL_HAZEL_HPP
#include <bela/base.hpp>
#include <bela/phmap.hpp>
#include "types.hpp"

namespace hazel {
#define nullfile_t INVALID_HANDLE_VALUE
struct FileAttributeTable {
  bela::flat_hash_map<std::wstring, std::wstring> attributes;
  bela::flat_hash_map<std::wstring, std::vector<std::wstring>> multi_attributes;
  std::wstring_view mime;
  types::hazel_types_t type;
};
// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointerex
// SetFilePointerEx
class File {
public:
  File() = default;
  File(const File &) = delete;
  File(File &&other) { MoveFrom(std::move(other)); }
  ~File() { Free(); }
  File &operator=(const File &) = delete;
  File &operator=(File &&other) {
    MoveFrom(std::move(other));
    return *this;
  }
  bool NewFile(std::wstring_view file, bela::error_code &ec);
  bool Lookup(FileAttributeTable &fat, bela::error_code &ec);
  std::wstring_view FullPath() const { return fullpath; }

private:
  HANDLE fd{nullfile_t};
  std::wstring fullpath;
  // inline
  bool SeekStart(uint64_t offset) {
    LARGE_INTEGER li;
    li.QuadPart = offset;
    LARGE_INTEGER oli{0};
    if (SetFilePointerEx(fd, li, &li, FILE_BEGIN) == 0) {
      return false;
    }
    return true;
  }
  void Free() {
    if (fd != nullfile_t) {
      CloseHandle(fd);
      fd = nullfile_t;
    }
  }
  void MoveFrom(File &&other) {
    Free();
    fd = other.fd;
    other.fd = nullfile_t;
    fullpath = std::move(other.fullpath);
  }
};

} // namespace hazel

#endif