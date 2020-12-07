///
#include <hazel/hazel.hpp>
#include <hazel/elf.hpp>
#include "elf.h"

namespace hazel::elf {
//
bool File::NewFile(HANDLE fd_, int64_t sz, bela::error_code &ec) {
  if (fd != INVALID_HANDLE_VALUE) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = fd_;
  size = sz;
  return ParseFile(ec);
}

bool File::ParseFile(bela::error_code &ec) {
  uint8_t ident[16];
  size_t outlen = 0;
  constexpr auto x = sizeof(FileHeader);
  if (!ReadAt(ident, sizeof(ident), 0, outlen, ec)) {
    return false;
  }
  if (outlen != 16) {
    ec = bela::make_error_code(L"elf: not a valid elf file ");
    return false;
  }
  constexpr uint8_t elfmagic[4] = {'\x7f', 'E', 'L', 'F'};
  if (memcmp(ident, elfmagic, sizeof(elfmagic)) != 0) {
    ec = bela::make_error_code(1, L"elf: bad magic number ['", static_cast<int>(ident[0]), L"', '",
                               static_cast<int>(ident[1]), L"', '", static_cast<int>(ident[2]), L"', '",
                               static_cast<int>(ident[3]), L"']");
    return false;
  }
  auto ElfClass = ident[EI_CLASS];
  switch (ElfClass) {
  case ELFCLASS32:
    break;
  case ELFCLASS64:
    is64bit = true;
    break;
  default:
    ec = bela::make_error_code(1, L"unkonw ELF class ", static_cast<int>(ElfClass));
    return false;
  }
  fh.Data = ident[EI_DATA];
  switch (fh.Data) {
  case ELFDATA2LSB:
    en = bela::endian::Endian::little;
    break;
  case ELFDATA2MSB:
    en = bela::endian::Endian::big;
    break;
  default:
    ec = bela::make_error_code(1, L"unkonw ELF data encoding ", static_cast<int>(fh.Data));
    return false;
  }
  fh.Version = ident[EI_VERSION];
  fh.OSABI = ident[EI_OSABI];
  // buna/debug/elf/file.go
  int64_t phoff = 0;
  int phentsize = 0;
  int phnum = 0;
  int64_t shoff = 0;
  int shentsize = 0;
  int shnum = 0;
  int shstrndx = 0;
  switch (fh.Class) {
  case ELFCLASS32: {
    Elf32_Ehdr hdr;
    if (!ReadAt(&hdr, sizeof(hdr), 0, outlen, ec)) {
      return false;
    }
    if (outlen != sizeof(hdr)) {
      ec = bela::make_error_code(L"elf: not a valid elf file ");
      return false;
    }
    fh.Type = SwapByte(hdr.e_type);
    fh.Machine = SwapByte(hdr.e_machine);
    fh.Entry = SwapByte(hdr.e_entry);
    if (auto version = SwapByte(hdr.e_version); version != static_cast<uint32_t>(fh.Version)) {
      ec = bela::make_error_code(1, L"mismatched ELF version, got ", version, L" want ",
                                 static_cast<uint32_t>(fh.Version));
      return false;
    }
    phoff = SwapByte(hdr.e_phoff);
    phentsize = SwapByte(hdr.e_phentsize);
    phnum = SwapByte(hdr.e_phnum);
    shoff = SwapByte(hdr.e_shoff);
    shentsize = SwapByte(hdr.e_shentsize);
    shnum = SwapByte(hdr.e_shnum);
    shstrndx = SwapByte(hdr.e_shstrndx);
  } break;
  case ELFCLASS64: {
    Elf64_Ehdr hdr;
    if (!ReadAt(&hdr, sizeof(hdr), 0, outlen, ec)) {
      return false;
    }
    if (outlen != sizeof(hdr)) {
      ec = bela::make_error_code(L"elf: not a valid elf file ");
      return false;
    }
    fh.Type = SwapByte(hdr.e_type);
    fh.Machine = SwapByte(hdr.e_machine);
    fh.Entry = SwapByte(hdr.e_entry);
    if (auto version = SwapByte(hdr.e_version); version != static_cast<uint32_t>(fh.Version)) {
      ec = bela::make_error_code(1, L"mismatched ELF version, got ", version, L" want ",
                                 static_cast<uint32_t>(fh.Version));
      return false;
    }
    phoff = SwapByte(hdr.e_phoff);
    phentsize = SwapByte(hdr.e_phentsize);
    phnum = SwapByte(hdr.e_phnum);
    shoff = SwapByte(hdr.e_shoff);
    shentsize = SwapByte(hdr.e_shentsize);
    shnum = SwapByte(hdr.e_shnum);
    shstrndx = SwapByte(hdr.e_shstrndx);
  } break;
  }
  //
  if (shoff == 0 && shnum != 0) {
    ec = bela::make_error_code(1, L"invalid ELF shnum for shoff=0 shnum=", shnum);
    return false;
  }
  if (shnum > 0 && shstrndx >= shnum) {
    ec = bela::make_error_code(1, L"invalid ELF shstrndx ", shstrndx);
    return false;
  }

  return true;
}

} // namespace hazel::elf