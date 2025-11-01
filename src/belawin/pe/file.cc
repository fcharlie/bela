//
#include <algorithm>
#include <bela/io.hpp>
#include "internal.hpp"
#include <utility>

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
// https://docs.microsoft.com/zh-cn/windows/win32/debug/pe-format
// https://en.wikipedia.org/wiki/Portable_Executable

namespace bela::pe {

inline void fromle(FileHeader &h) {
  if constexpr (bela::IsBigEndian()) {
    h.Characteristics = bela::fromle(h.Characteristics);
    h.Machine = bela::fromle(h.Machine);
    h.NumberOfSections = bela::fromle(h.NumberOfSections);
    h.NumberOfSymbols = bela::fromle(h.NumberOfSymbols);
    h.PointerToSymbolTable = bela::fromle(h.PointerToSymbolTable);
    h.TimeDateStamp = bela::fromle(h.TimeDateStamp);
    h.SizeOfOptionalHeader = bela::fromle(h.SizeOfOptionalHeader);
  }
}

inline void fromle(BigObjHeader &b) {
  if constexpr (bela::IsBigEndian()) {
    b.Sig1 = bela::fromle(b.Sig1);
    b.Sig2 = bela::fromle(b.Sig2);
    b.Version = bela::fromle(b.Version);
    b.Machine = bela::fromle(b.Machine);
    b.TimeDateStamp = bela::fromle(b.TimeDateStamp);
    b.SizeOfData = bela::fromle(b.SizeOfData);
    b.Flags = bela::fromle(b.Flags);
    b.MetaDataSize = bela::fromle(b.MetaDataSize);
    b.MetaDataOffset = bela::fromle(b.MetaDataOffset);
    b.NumberOfSections = bela::fromle(b.NumberOfSections);
    b.PointerToSymbolTable = bela::fromle(b.PointerToSymbolTable);
    b.NumberOfSymbols = bela::fromle(b.NumberOfSymbols);
  }
}

void fromle(OptionalHeader *o, const IMAGE_OPTIONAL_HEADER64 *o64) {
  o->Magic = bela::fromle(o64->Magic);
  o->MajorLinkerVersion = o64->MajorLinkerVersion;
  o->MinorLinkerVersion = o64->MinorLinkerVersion;
  o->SizeOfCode = bela::fromle(o64->SizeOfCode);
  o->SizeOfInitializedData = bela::fromle(o64->SizeOfInitializedData);
  o->SizeOfUninitializedData = bela::fromle(o64->SizeOfUninitializedData);
  o->AddressOfEntryPoint = bela::fromle(o64->AddressOfEntryPoint);
  o->BaseOfCode = bela::fromle(o64->BaseOfCode);
  o->ImageBase = bela::fromle(o64->ImageBase);
  o->SectionAlignment = bela::fromle(o64->SectionAlignment);
  o->FileAlignment = bela::fromle(o64->FileAlignment);
  o->MajorOperatingSystemVersion = bela::fromle(o64->MajorOperatingSystemVersion);
  o->MinorOperatingSystemVersion = bela::fromle(o64->MinorOperatingSystemVersion);
  o->MajorImageVersion = bela::fromle(o64->MajorImageVersion);
  o->MinorImageVersion = bela::fromle(o64->MinorImageVersion);
  o->MajorSubsystemVersion = bela::fromle(o64->MajorSubsystemVersion);
  o->MinorSubsystemVersion = bela::fromle(o64->MinorSubsystemVersion);
  o->Win32VersionValue = bela::fromle(o64->Win32VersionValue);
  o->SizeOfImage = bela::fromle(o64->SizeOfImage);
  o->SizeOfHeaders = bela::fromle(o64->SizeOfHeaders);
  o->CheckSum = bela::fromle(o64->CheckSum);
  o->Subsystem = bela::fromle(o64->Subsystem);
  o->DllCharacteristics = bela::fromle(o64->DllCharacteristics);
  o->SizeOfStackReserve = bela::fromle(o64->SizeOfStackReserve);
  o->SizeOfStackCommit = bela::fromle(o64->SizeOfStackCommit);
  o->SizeOfHeapReserve = bela::fromle(o64->SizeOfHeapReserve);
  o->SizeOfHeapCommit = bela::fromle(o64->SizeOfHeapCommit);
  o->LoaderFlags = bela::fromle(o64->LoaderFlags);
  o->NumberOfRvaAndSizes = bela::fromle(o64->NumberOfRvaAndSizes);
  for (int i = 0; i < DataDirEntries; i++) {
    o->DataDirectory[i].Size = bela::fromle(o64->DataDirectory[i].Size);
    o->DataDirectory[i].VirtualAddress = bela::fromle(o64->DataDirectory[i].VirtualAddress);
  }
}

void fromle(OptionalHeader *o, const IMAGE_OPTIONAL_HEADER32 *o32) {
  o->Magic = bela::fromle(o32->Magic);
  o->MajorLinkerVersion = o32->MajorLinkerVersion;
  o->MinorLinkerVersion = o32->MinorLinkerVersion;
  o->SizeOfCode = bela::fromle(o32->SizeOfCode);
  o->SizeOfInitializedData = bela::fromle(o32->SizeOfInitializedData);
  o->SizeOfUninitializedData = bela::fromle(o32->SizeOfUninitializedData);
  o->AddressOfEntryPoint = bela::fromle(o32->AddressOfEntryPoint);
  o->BaseOfCode = bela::fromle(o32->BaseOfCode);
  o->ImageBase = bela::fromle(o32->ImageBase);
  o->SectionAlignment = bela::fromle(o32->SectionAlignment);
  o->FileAlignment = bela::fromle(o32->FileAlignment);
  o->MajorOperatingSystemVersion = bela::fromle(o32->MajorOperatingSystemVersion);
  o->MinorOperatingSystemVersion = bela::fromle(o32->MinorOperatingSystemVersion);
  o->MajorImageVersion = bela::fromle(o32->MajorImageVersion);
  o->MinorImageVersion = bela::fromle(o32->MinorImageVersion);
  o->MajorSubsystemVersion = bela::fromle(o32->MajorSubsystemVersion);
  o->MinorSubsystemVersion = bela::fromle(o32->MinorSubsystemVersion);
  o->Win32VersionValue = bela::fromle(o32->Win32VersionValue);
  o->SizeOfImage = bela::fromle(o32->SizeOfImage);
  o->SizeOfHeaders = bela::fromle(o32->SizeOfHeaders);
  o->CheckSum = bela::fromle(o32->CheckSum);
  o->Subsystem = bela::fromle(o32->Subsystem);
  o->DllCharacteristics = bela::fromle(o32->DllCharacteristics);
  o->SizeOfStackReserve = bela::fromle(o32->SizeOfStackReserve);
  o->SizeOfStackCommit = bela::fromle(o32->SizeOfStackCommit);
  o->SizeOfHeapReserve = bela::fromle(o32->SizeOfHeapReserve);
  o->SizeOfHeapCommit = bela::fromle(o32->SizeOfHeapCommit);
  o->LoaderFlags = bela::fromle(o32->LoaderFlags);
  o->NumberOfRvaAndSizes = bela::fromle(o32->NumberOfRvaAndSizes);
  for (int i = 0; i < DataDirEntries; i++) {
    o->DataDirectory[i].Size = bela::fromle(o32->DataDirectory[i].Size);
    o->DataDirectory[i].VirtualAddress = bela::fromle(o32->DataDirectory[i].VirtualAddress);
  }
  o->BaseOfData32 = bela::fromle(o32->BaseOfData);
}

inline void fromle(SectionHeader32 &sh) {
  if constexpr (bela::IsBigEndian()) {
    sh.VirtualSize = bela::fromle(sh.VirtualSize);
    sh.VirtualAddress = bela::fromle(sh.VirtualAddress);
    sh.SizeOfRawData = bela::fromle(sh.SizeOfRawData);
    sh.PointerToRawData = bela::fromle(sh.PointerToRawData);
    sh.PointerToRelocations = bela::fromle(sh.PointerToRelocations);
    sh.PointerToLineNumbers = bela::fromle(sh.PointerToLineNumbers);
    sh.NumberOfRelocations = bela::fromle(sh.NumberOfRelocations);
    sh.NumberOfLineNumbers = bela::fromle(sh.NumberOfLineNumbers);
    sh.Characteristics = bela::fromle(sh.Characteristics);
  }
}

bool File::parseFile(bela::error_code &ec) {
  if (size == SizeUnInitialized) {
    if (size = fd.Size(ec); size == bela::SizeUnInitialized) {
      return false;
    }
  }
  DosHeader dh;
  if (!fd.ReadAt(dh, 0, ec)) {
    return false;
  }
  memset(&o, 0, sizeof(o));
  int64_t base = 0;
  if (bela::fromle(dh.e_magic) == IMAGE_DOS_SIGNATURE) {
    auto signoff = static_cast<int64_t>(bela::fromle(dh.e_lfanew));
    uint8_t sign[4];
    if (!fd.ReadAt(sign, signoff, ec)) {
      return false;
    }
    if (sign[0] != 'P' || sign[1] != 'E' || sign[2] != 0 || sign[3] != 0) {
      ec = bela::make_error_code(ErrGeneral, L"Invalid PE COFF file signature of ['", int(sign[0]), L"','",
                                 int(sign[1]), L"','", int(sign[2]), L"','", int(sign[3]), L"']");
      return false;
    }
    base = signoff + 4;
  }

  if (!fd.ReadAt(h, base, ec)) {
    return false;
  }
  fromle(h);
  if (h.Machine == IMAGE_FILE_MACHINE_UNKNOWN && h.NumberOfSections == 0xffff) {
    // try parse BigObjHeader
    if (!fd.ReadAt(b, base, ec)) {
      return false;
    }
    fromle(b);
    o.Is64Bit = MachineIs64Bit(static_cast<bela::pe::Machine>(b.Machine));
  } else {
    o.Is64Bit = (h.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64));
  }

  if (!readStringTable(ec)) {
    return false;
  }

  if (o.Is64Bit) {
    IMAGE_OPTIONAL_HEADER64 o64;
    if (!fd.ReadAt(o64, base + sizeof(FileHeader), ec)) {
      ec = bela::make_error_code(ErrGeneral, L"pe: not a valid pe file ", ec.message);
      return false;
    }
    fromle(&o, &o64);
  } else {
    IMAGE_OPTIONAL_HEADER32 o32;
    if (!fd.ReadAt(o32, base + sizeof(FileHeader), ec)) {
      ec = bela::make_error_code(ErrGeneral, L"pe: not a valid pe file ", ec.message);
      return false;
    }
    fromle(&o, &o32);
  }
  auto sc = NumberOfSections();
  sections.resize(sc);
  for (int i = 0; std::cmp_less(i, sc); i++) {
    SectionHeader32 sh;
    if (!fd.ReadFull(sh, ec)) {
      return false;
    }
    fromle(sh);
    auto sec = &sections[i];
    sec->Name = sectionFullName(sh);
    sec->VirtualSize = sh.VirtualSize;
    sec->VirtualAddress = sh.VirtualAddress;
    sec->Size = sh.SizeOfRawData;
    sec->Offset = sh.PointerToRawData;
    sec->PointerToRelocations = sh.PointerToRelocations;
    sec->PointerToLineNumbers = sh.PointerToLineNumbers;
    sec->NumberOfRelocations = sh.NumberOfRelocations;
    sec->NumberOfLineNumbers = sh.NumberOfLineNumbers;
    sec->Characteristics = sh.Characteristics;
    overlayOffset = (std::max)(static_cast<int64_t>(sec->Offset + sec->Size), overlayOffset);
  }
  for (auto &sec : sections) {
    readRelocs(sec);
  }
  return true;
}

// Lookup function table
bool File::LookupFunctionTable(FunctionTable &ft, bela::error_code &ec) const {
  if (!LookupImports(ft.imports, ec)) {
    return false;
  }
  if (!LookupDelayImports(ft.delayimprots, ec)) {
    return false;
  }
  return LookupExports(ft.exports, ec);
}

bool File::NewFile(std::wstring_view p, bela::error_code &ec) {
  auto fd_ = bela::io::NewFile(p, ec);
  if (!fd_) {
    return false;
  }
  fd = std::move(*fd_);
  return parseFile(ec);
}

bool File::NewFile(HANDLE fd_, int64_t sz, bela::error_code &ec) {
  fd.Assgin(fd_, false);
  size = sz;
  return parseFile(ec);
}

} // namespace bela::pe
