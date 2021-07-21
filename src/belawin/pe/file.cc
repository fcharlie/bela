//
#include "internal.hpp"
#include <algorithm>

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
// https://docs.microsoft.com/zh-cn/windows/win32/debug/pe-format
// https://en.wikipedia.org/wiki/Portable_Executable

namespace bela::pe {
bool File::ReadFull(std::span<uint8_t> buffer, bela::error_code &ec) const {
  if (buffer.size() == 0) {
    return true;
  }
  auto p = reinterpret_cast<uint8_t *>(buffer.data());
  auto len = buffer.size();
  size_t total = 0;
  while (total < len) {
    DWORD dwSize = 0;
    if (ReadFile(fd, p + total, static_cast<DWORD>(len - total), &dwSize, nullptr) != TRUE) {
      ec = bela::make_system_error_code(L"ReadFile: ");
      return false;
    }
    if (dwSize == 0) {
      ec = bela::make_error_code(ERROR_HANDLE_EOF, L"Reached the end of the file");
      return false;
    }
    total += dwSize;
  }
  return true;
}

bool File::ReadAt(std::span<uint8_t> buffer, int64_t pos, bela::error_code &ec) const {
  if (!bela::os::file::Seek(fd, pos, ec)) {
    return false;
  }
  return ReadFull(buffer, ec);
}

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

void fromle(OptionalHeader *oh, const IMAGE_OPTIONAL_HEADER64 *oh64) {
  oh->Magic = bela::fromle(oh64->Magic);
  oh->SizeOfCode = bela::fromle(oh64->SizeOfCode);
  oh->SizeOfInitializedData = bela::fromle(oh64->SizeOfInitializedData);
  oh->SizeOfUninitializedData = bela::fromle(oh64->SizeOfUninitializedData);
  oh->AddressOfEntryPoint = bela::fromle(oh64->AddressOfEntryPoint);
  oh->BaseOfCode = bela::fromle(oh64->BaseOfCode);
  oh->ImageBase = bela::fromle(oh64->ImageBase);
  oh->SectionAlignment = bela::fromle(oh64->SectionAlignment);
  oh->FileAlignment = bela::fromle(oh64->FileAlignment);
  oh->MajorOperatingSystemVersion = bela::fromle(oh64->MajorOperatingSystemVersion);
  oh->MinorOperatingSystemVersion = bela::fromle(oh64->MinorOperatingSystemVersion);
  oh->MajorImageVersion = bela::fromle(oh64->MajorImageVersion);
  oh->MinorImageVersion = bela::fromle(oh64->MinorImageVersion);
  oh->MajorSubsystemVersion = bela::fromle(oh64->MajorSubsystemVersion);
  oh->MinorSubsystemVersion = bela::fromle(oh64->MinorSubsystemVersion);
  oh->Win32VersionValue = bela::fromle(oh64->Win32VersionValue);
  oh->SizeOfImage = bela::fromle(oh64->SizeOfImage);
  oh->SizeOfHeaders = bela::fromle(oh64->SizeOfHeaders);
  oh->CheckSum = bela::fromle(oh64->CheckSum);
  oh->Subsystem = bela::fromle(oh64->Subsystem);
  oh->DllCharacteristics = bela::fromle(oh64->DllCharacteristics);
  oh->SizeOfStackReserve = bela::fromle(oh64->SizeOfStackReserve);
  oh->SizeOfStackCommit = bela::fromle(oh64->SizeOfStackCommit);
  oh->SizeOfHeapReserve = bela::fromle(oh64->SizeOfHeapReserve);
  oh->SizeOfHeapCommit = bela::fromle(oh64->SizeOfHeapCommit);
  oh->LoaderFlags = bela::fromle(oh64->LoaderFlags);
  oh->NumberOfRvaAndSizes = bela::fromle(oh64->NumberOfRvaAndSizes);
  for (int i = 0; i < DataDirEntries; i++) {
    oh->DataDirectory[i].Size = bela::fromle(oh64->DataDirectory[i].Size);
    oh->DataDirectory[i].VirtualAddress = bela::fromle(oh64->DataDirectory[i].VirtualAddress);
  }
}

void fromle(OptionalHeader *oh, const IMAGE_OPTIONAL_HEADER32 *oh32) {
  oh->Magic = bela::fromle(oh32->Magic);
  oh->SizeOfCode = bela::fromle(oh32->SizeOfCode);
  oh->SizeOfInitializedData = bela::fromle(oh32->SizeOfInitializedData);
  oh->SizeOfUninitializedData = bela::fromle(oh32->SizeOfUninitializedData);
  oh->AddressOfEntryPoint = bela::fromle(oh32->AddressOfEntryPoint);
  oh->BaseOfCode = bela::fromle(oh32->BaseOfCode);
  oh->ImageBase = bela::fromle(oh32->ImageBase);
  oh->SectionAlignment = bela::fromle(oh32->SectionAlignment);
  oh->FileAlignment = bela::fromle(oh32->FileAlignment);
  oh->MajorOperatingSystemVersion = bela::fromle(oh32->MajorOperatingSystemVersion);
  oh->MinorOperatingSystemVersion = bela::fromle(oh32->MinorOperatingSystemVersion);
  oh->MajorImageVersion = bela::fromle(oh32->MajorImageVersion);
  oh->MinorImageVersion = bela::fromle(oh32->MinorImageVersion);
  oh->MajorSubsystemVersion = bela::fromle(oh32->MajorSubsystemVersion);
  oh->MinorSubsystemVersion = bela::fromle(oh32->MinorSubsystemVersion);
  oh->Win32VersionValue = bela::fromle(oh32->Win32VersionValue);
  oh->SizeOfImage = bela::fromle(oh32->SizeOfImage);
  oh->SizeOfHeaders = bela::fromle(oh32->SizeOfHeaders);
  oh->CheckSum = bela::fromle(oh32->CheckSum);
  oh->Subsystem = bela::fromle(oh32->Subsystem);
  oh->DllCharacteristics = bela::fromle(oh32->DllCharacteristics);
  oh->SizeOfStackReserve = bela::fromle(oh32->SizeOfStackReserve);
  oh->SizeOfStackCommit = bela::fromle(oh32->SizeOfStackCommit);
  oh->SizeOfHeapReserve = bela::fromle(oh32->SizeOfHeapReserve);
  oh->SizeOfHeapCommit = bela::fromle(oh32->SizeOfHeapCommit);
  oh->LoaderFlags = bela::fromle(oh32->LoaderFlags);
  oh->NumberOfRvaAndSizes = bela::fromle(oh32->NumberOfRvaAndSizes);
  for (int i = 0; i < DataDirEntries; i++) {
    oh->DataDirectory[i].Size = bela::fromle(oh32->DataDirectory[i].Size);
    oh->DataDirectory[i].VirtualAddress = bela::fromle(oh32->DataDirectory[i].VirtualAddress);
  }
  oh->BaseOfData32 = bela::fromle(oh32->BaseOfData);
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
  if (!readAt(dh, 0, ec)) {
    return false;
  }
  memset(&oh, 0, sizeof(oh));
  int64_t base = 0;
  if (bela::fromle(dh.e_magic) == IMAGE_DOS_SIGNATURE) {
    auto signoff = static_cast<int64_t>(bela::fromle(dh.e_lfanew));
    uint8_t sign[4];
    if (!ReadAt(sign, signoff, ec)) {
      return false;
    }
    if (!(sign[0] == 'P' && sign[1] == 'E' && sign[2] == 0 && sign[3] == 0)) {
      ec = bela::make_error_code(ErrGeneral, L"Invalid PE COFF file signature of ['", int(sign[0]), L"','",
                                 int(sign[1]), L"','", int(sign[2]), L"','", int(sign[3]), L"']");
      return false;
    }
    base = signoff + 4;
  }

  if (!readAt(fh, base, ec)) {
    return false;
  }
  fromle(fh);
  oh.Is64Bit = (fh.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64));
  if (!readStringTable(ec)) {
    return false;
  }

  if (oh.Is64Bit) {
    IMAGE_OPTIONAL_HEADER64 oh64;
    if (!readAt(oh64, base + sizeof(FileHeader), ec)) {
      return false;
    }
    fromle(&oh, &oh64);
  } else {
    IMAGE_OPTIONAL_HEADER32 oh32;
    if (!readAt(oh32, base + sizeof(FileHeader), ec)) {
      ec = bela::make_error_code(ErrGeneral, L"pe: not a valid pe file ", ec.message);
      return false;
    }
    fromle(&oh, &oh32);
  }
  sections.resize(fh.NumberOfSections);
  for (int i = 0; i < fh.NumberOfSections; i++) {
    SectionHeader32 sh;
    if (!readFull(sh, ec)) {
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
