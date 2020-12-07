//
#include <hazel/elf.hpp>

namespace bela::elf {

const char *ElfABIName(int abi) {
  static constexpr struct {
    int id;
    const char *name;
  } abinames[] = {
      {0, "ELFOSABI_NONE"},     {1, "ELFOSABI_HPUX"},      {2, "ELFOSABI_NETBSD"},  {3, "ELFOSABI_LINUX"},
      {4, "ELFOSABI_HURD"},     {5, "ELFOSABI_86OPEN"},    {6, "ELFOSABI_SOLARIS"}, {7, "ELFOSABI_AIX"},
      {8, "ELFOSABI_IRIX"},     {9, "ELFOSABI_FREEBSD"},   {10, "ELFOSABI_TRU64"},  {11, "ELFOSABI_MODESTO"},
      {12, "ELFOSABI_OPENBSD"}, {13, "ELFOSABI_OPENVMS"},  {14, "ELFOSABI_NSK"},    {15, "ELFOSABI_AROS"},
      {16, "ELFOSABI_FENIXOS"}, {17, "ELFOSABI_CLOUDABI"}, {97, "ELFOSABI_ARM"},    {255, "ELFOSABI_STANDALONE"},
  };
  for (const auto &e : abinames) {
    if (e.id == abi) {
      return e.name;
    }
  }
  return "ELFOSABI_ERROR";
}

} // namespace bela::elf