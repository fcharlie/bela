//
#ifndef HAZEL_HAZEL_HPP
#define HAZEL_HAZEL_HPP
#include <bela/base.hpp>
#include <bela/phmap.hpp>
#include <bela/buffer.hpp>
#include "types.hpp"

#ifndef nullfile_t
#define nullfile_t INVALID_HANDLE_VALUE
#endif

namespace hazel {
// file attribute table
struct FileAttributeTable {
  bela::flat_hash_map<std::wstring, std::wstring> attributes;
  bela::flat_hash_map<std::wstring, std::vector<std::wstring>> multi_attributes;
  types::hazel_types_t type{types::none};
  FileAttributeTable &assign(std::wstring_view desc, types::hazel_types_t t = types::none) {
    attributes.emplace(L"Description", desc);
    type = t;
    return *this;
  }
  FileAttributeTable &append(const std::wstring_view &name, const std::wstring_view &value) {
    attributes.emplace(name, value);
    return *this;
  }
  FileAttributeTable &append(std::wstring &&name, std::wstring &&value) {
    attributes.emplace(std::move(name), std::move(value));
    return *this;
  }
  FileAttributeTable &append(std::wstring &&name, std::vector<std::wstring> &&value) {
    multi_attributes.emplace(std::move(name), std::move(value));
    return *this;
  }

  bool LooksLikeELF() const {
    return type == types::elf || type == types::elf_executable || type == types::elf_relocatable ||
           type == types::elf_shared_object;
  }
  bool LooksLikeMachO() const {
    return type == types::macho_bundle || type == types::macho_core || type == types::macho_dsym_companion ||
           type == types::macho_dynamic_linker || type == types::macho_dynamically_linked_shared_lib ||
           type == types::macho_dynamically_linked_shared_lib_stub || type == types::macho_executable ||
           type == types::macho_fixed_virtual_memory_shared_lib || type == types::macho_kext_bundle ||
           type == types::macho_object || type == types::macho_preload_executable ||
           type == types::macho_universal_binary;
  }
  bool LooksLikePE() const { return type == types::pecoff_executable; }
  bool LooksLikeZIP() const {
    return type == types::zip || type == types::docx || type == types::xlsx || type == types::pptx ||
           type == types::ofd;
  }
};

// zip details

struct ZipDetails {
  std::wstring_view mime;
  bela::flat_hash_map<uint16_t, uint32_t> methodsmap;
  uint64_t uncompressedsize{0};
  uint32_t filecounts{0};
  uint32_t folders{0};
  bool hassymlink{false};
  types::hazel_types_t subtype;
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
  bool PositionAt(uint64_t pos, bela::error_code &ec) const {
    auto li = *reinterpret_cast<LARGE_INTEGER *>(&pos);
    LARGE_INTEGER oli{0};
    if (SetFilePointerEx(fd, li, &oli, SEEK_SET) != TRUE) {
      ec = bela::make_error_code(L"SetFilePointerEx: ");
      return false;
    }
    return true;
  }
  bool Read(void *buffer, size_t len, size_t &outlen, bela::error_code &ec) const {
    DWORD dwSize = {0};
    if (ReadFile(fd, buffer, static_cast<DWORD>(len), &dwSize, nullptr) != TRUE) {
      ec = bela::make_system_error_code(L"ReadFile: ");
      return false;
    }
    outlen = static_cast<size_t>(len);
    return true;
  }
  bool ReadAt(bela::Buffer &b, size_t len, uint64_t pos, bela::error_code &ec) const {
    if (len > b.capacity()) {
      b.grow(bela::align_length(len));
    }
    if (!PositionAt(pos, ec)) {
      return false;
    }
    return Read(b.data(), len, b.size(), ec);
  }
  bool ReadAt(void *buffer, size_t len, uint64_t pos, size_t &outlen, bela::error_code &ec) {
    if (!PositionAt(pos, ec)) {
      return false;
    }
    return Read(buffer, len, outlen, ec);
  }
  bool Read(bela::Buffer &b, bela::error_code &ec) const { return Read(b.data(), b.capacity(), b.size(), ec); }
  bool Read(bela::Buffer &b, size_t len, bela::error_code &ec) const { return Read(b.data(), len, b.size(), ec); }
  bool ReadAt(bela::Buffer &b, uint64_t pos, bela::error_code &ec) const { return ReadAt(b, b.capacity(), pos, ec); }
  std::wstring_view FullPath() const { return fullpath; }
  bool Lookup(FileAttributeTable &fat, bela::error_code &ec);

private:
  HANDLE fd{nullfile_t};
  std::wstring fullpath;
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