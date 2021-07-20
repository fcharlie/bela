//
#include "internal.hpp"
#include <algorithm>

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
// https://docs.microsoft.com/zh-cn/windows/win32/debug/pe-format
// https://en.wikipedia.org/wiki/Portable_Executable

namespace bela::pe {

inline void fromle(FileHeader &fh) {
  if constexpr (bela::IsBigEndian()) {
    fh.Characteristics = bela::fromle(fh.Characteristics);
    fh.Machine = bela::fromle(fh.Machine);
    fh.NumberOfSections = bela::fromle(fh.NumberOfSections);
    fh.NumberOfSymbols = bela::fromle(fh.NumberOfSymbols);
    fh.PointerToSymbolTable = bela::fromle(fh.PointerToSymbolTable);
    fh.TimeDateStamp = bela::fromle(fh.TimeDateStamp);
    fh.SizeOfOptionalHeader = bela::fromle(fh.SizeOfOptionalHeader);
  }
}

inline void fromle(OptionalHeader64 *oh) {
  if constexpr (bela::IsBigEndian()) {
    oh->Magic = bela::fromle(oh->Magic);
    oh->SizeOfCode = bela::fromle(oh->SizeOfCode);
    oh->SizeOfInitializedData = bela::fromle(oh->SizeOfInitializedData);
    oh->SizeOfUninitializedData = bela::fromle(oh->SizeOfUninitializedData);
    oh->AddressOfEntryPoint = bela::fromle(oh->AddressOfEntryPoint);
    oh->BaseOfCode = bela::fromle(oh->BaseOfCode);
    oh->ImageBase = bela::fromle(oh->ImageBase);
    oh->SectionAlignment = bela::fromle(oh->SectionAlignment);
    oh->FileAlignment = bela::fromle(oh->FileAlignment);
    oh->MajorOperatingSystemVersion = bela::fromle(oh->MajorOperatingSystemVersion);
    oh->MinorOperatingSystemVersion = bela::fromle(oh->MinorOperatingSystemVersion);
    oh->MajorImageVersion = bela::fromle(oh->MajorImageVersion);
    oh->MinorImageVersion = bela::fromle(oh->MinorImageVersion);
    oh->MajorSubsystemVersion = bela::fromle(oh->MajorSubsystemVersion);
    oh->MinorSubsystemVersion = bela::fromle(oh->MinorSubsystemVersion);
    oh->Win32VersionValue = bela::fromle(oh->Win32VersionValue);
    oh->SizeOfImage = bela::fromle(oh->SizeOfImage);
    oh->SizeOfHeaders = bela::fromle(oh->SizeOfHeaders);
    oh->CheckSum = bela::fromle(oh->CheckSum);
    oh->Subsystem = bela::fromle(oh->Subsystem);
    oh->DllCharacteristics = bela::fromle(oh->DllCharacteristics);
    oh->SizeOfStackReserve = bela::fromle(oh->SizeOfStackReserve);
    oh->SizeOfStackCommit = bela::fromle(oh->SizeOfStackCommit);
    oh->SizeOfHeapReserve = bela::fromle(oh->SizeOfHeapReserve);
    oh->SizeOfHeapCommit = bela::fromle(oh->SizeOfHeapCommit);
    oh->LoaderFlags = bela::fromle(oh->LoaderFlags);
    oh->NumberOfRvaAndSizes = bela::fromle(oh->NumberOfRvaAndSizes);
    for (auto &d : oh->DataDirectory) {
      d.Size = bela::fromle(d.Size);
      d.VirtualAddress = bela::fromle(d.VirtualAddress);
    }
  }
}

inline void fromle(OptionalHeader32 *oh) {
  if constexpr (bela::IsBigEndian()) {
    oh->Magic = bela::fromle(oh->Magic);
    oh->SizeOfCode = bela::fromle(oh->SizeOfCode);
    oh->SizeOfInitializedData = bela::fromle(oh->SizeOfInitializedData);
    oh->SizeOfUninitializedData = bela::fromle(oh->SizeOfUninitializedData);
    oh->AddressOfEntryPoint = bela::fromle(oh->AddressOfEntryPoint);
    oh->BaseOfCode = bela::fromle(oh->BaseOfCode);
    oh->BaseOfData = bela::fromle(oh->BaseOfData);
    oh->ImageBase = bela::fromle(oh->ImageBase);
    oh->SectionAlignment = bela::fromle(oh->SectionAlignment);
    oh->FileAlignment = bela::fromle(oh->FileAlignment);
    oh->MajorOperatingSystemVersion = bela::fromle(oh->MajorOperatingSystemVersion);
    oh->MinorOperatingSystemVersion = bela::fromle(oh->MinorOperatingSystemVersion);
    oh->MajorImageVersion = bela::fromle(oh->MajorImageVersion);
    oh->MinorImageVersion = bela::fromle(oh->MinorImageVersion);
    oh->MajorSubsystemVersion = bela::fromle(oh->MajorSubsystemVersion);
    oh->MinorSubsystemVersion = bela::fromle(oh->MinorSubsystemVersion);
    oh->Win32VersionValue = bela::fromle(oh->Win32VersionValue);
    oh->SizeOfImage = bela::fromle(oh->SizeOfImage);
    oh->SizeOfHeaders = bela::fromle(oh->SizeOfHeaders);
    oh->CheckSum = bela::fromle(oh->CheckSum);
    oh->Subsystem = bela::fromle(oh->Subsystem);
    oh->DllCharacteristics = bela::fromle(oh->DllCharacteristics);
    oh->SizeOfStackReserve = bela::fromle(oh->SizeOfStackReserve);
    oh->SizeOfStackCommit = bela::fromle(oh->SizeOfStackCommit);
    oh->SizeOfHeapReserve = bela::fromle(oh->SizeOfHeapReserve);
    oh->SizeOfHeapCommit = bela::fromle(oh->SizeOfHeapCommit);
    oh->LoaderFlags = bela::fromle(oh->LoaderFlags);
    oh->NumberOfRvaAndSizes = bela::fromle(oh->NumberOfRvaAndSizes);
    for (auto &d : oh->DataDirectory) {
      d.Size = bela::fromle(d.Size);
      d.VirtualAddress = bela::fromle(d.VirtualAddress);
    }
  }
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
    if ((size = bela::os::file::Size(fd, ec)) == bela::SizeUnInitialized) {
      return false;
    }
  }
  DosHeader dh;
  if (!ReadAt(&dh, sizeof(DosHeader), 0, ec)) {
    return false;
  }

  int64_t base = 0;
  if (bela::fromle(dh.e_magic) == IMAGE_DOS_SIGNATURE) {
    auto signoff = static_cast<int64_t>(bela::fromle(dh.e_lfanew));
    uint8_t sign[4];
    if (!ReadAt(sign, 4, signoff, ec)) {
      return false;
    }
    if (!(sign[0] == 'P' && sign[1] == 'E' && sign[2] == 0 && sign[3] == 0)) {
      ec = bela::make_error_code(ErrGeneral, L"Invalid PE COFF file signature of ['", int(sign[0]), L"','",
                                 int(sign[1]), L"','", int(sign[2]), L"','", int(sign[3]), L"']");
      return false;
    }
    base = signoff + 4;
  }

  if (!ReadAt(&fh, sizeof(FileHeader), base, ec)) {
    return false;
  }
  fromle(fh);
  is64bit = (fh.SizeOfOptionalHeader == sizeof(OptionalHeader64));
  if (!readStringTable(ec)) {
    return false;
  }

  if (is64bit) {
    if (!ReadAt(&oh, sizeof(OptionalHeader64), base + sizeof(FileHeader), ec)) {
      return false;
    }
    fromle(&oh);
  } else {
    if (!ReadAt(&oh, sizeof(OptionalHeader32), base + sizeof(FileHeader), ec)) {
      ec = bela::make_error_code(ErrGeneral, L"pe: not a valid pe file ", ec.message);
      return false;
    }
    fromle(reinterpret_cast<OptionalHeader32 *>(&oh));
  }
  sections.resize(fh.NumberOfSections);
  for (int i = 0; i < fh.NumberOfSections; i++) {
    SectionHeader32 sh;
    if (!ReadFull(&sh, sizeof(SectionHeader32), ec)) {
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
    if (auto sectionEnd = static_cast<int64_t>(sec->Offset + sec->Size); sectionEnd > overlayOffset) {
      overlayOffset = sectionEnd;
    }
  }
  for (auto &sec : sections) {
    readRelocs(sec);
  }

  return true;
}

bool File::NewFile(std::wstring_view p, bela::error_code &ec) {
  if (fd != INVALID_HANDLE_VALUE) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = CreateFileW(p.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL, nullptr);
  if (fd == INVALID_HANDLE_VALUE) {
    ec = bela::make_system_error_code(L"CreateFileW: ");
    return false;
  }
  needClosed = true;
  return parseFile(ec);
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

} // namespace bela::pe
