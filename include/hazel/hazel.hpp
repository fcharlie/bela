//
#ifndef HAZEL_HAZEL_HPP
#define HAZEL_HAZEL_HPP
#include <bela/base.hpp>
#include <bela/phmap.hpp>
#include "types.hpp"

namespace hazel {
constexpr auto nullfile_t{INVALID_HANDLE_VALUE};

struct FileAttributes {
  bela::flat_hash_map<std::wstring, std::wstring> attributes;
  bela::flat_hash_map<std::wstring, std::vector<std::wstring>> multi_attributes;
  std::wstring_view mime;
  types::hazel_types_t type;
};

class File {
public:
  File() = default;
  File(HANDLE fd_) : fd{fd_} {}
  File(const File &) = delete;
  File &operator=(const File &) = delete;
  bool Lookup(FileAttributes &fa);

private:
  HANDLE fd{nullfile_t};
  bool Seek(uint64_t offset);
};

} // namespace hazel

#endif