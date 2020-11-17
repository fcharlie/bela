//
#include "internal.hpp"

namespace bela::pe {

void swaple(FileHeader &fh) {
  if constexpr (bela::IsBigEndian()) {
    fh.Characteristics = bela::swaple(fh.Characteristics);
    fh.Machine = bela::swaple(fh.Machine);
    fh.NumberOfSections = bela::swaple(fh.NumberOfSections);
    fh.NumberOfSymbols = bela::swaple(fh.NumberOfSymbols);
    fh.PointerToSymbolTable = bela::swaple(fh.PointerToSymbolTable);
    fh.TimeDateStamp = bela::swaple(fh.TimeDateStamp);
    fh.SizeOfOptionalHeader = bela::swaple(fh.SizeOfOptionalHeader);
  }
}

void swaple(OptionalHeader64 *oh) {
  if constexpr (bela::IsBigEndian()) {
    oh->Magic = bela::swaple(oh->Magic);
    oh->SizeOfCode = bela::swaple(oh->SizeOfCode);
    oh->SizeOfInitializedData = bela::swaple(oh->SizeOfInitializedData);
    oh->SizeOfUninitializedData = bela::swaple(oh->SizeOfUninitializedData);
    oh->AddressOfEntryPoint = bela::swaple(oh->AddressOfEntryPoint);
    oh->BaseOfCode = bela::swaple(oh->BaseOfCode);
    oh->ImageBase = bela::swaple(oh->ImageBase);
    oh->SectionAlignment = bela::swaple(oh->SectionAlignment);
    oh->FileAlignment = bela::swaple(oh->FileAlignment);
    oh->MajorOperatingSystemVersion = bela::swaple(oh->MajorOperatingSystemVersion);
    oh->MinorOperatingSystemVersion = bela::swaple(oh->MinorOperatingSystemVersion);
    oh->MajorImageVersion = bela::swaple(oh->MajorImageVersion);
    oh->MinorImageVersion = bela::swaple(oh->MinorImageVersion);
    oh->MajorSubsystemVersion = bela::swaple(oh->MajorSubsystemVersion);
    oh->MinorSubsystemVersion = bela::swaple(oh->MinorSubsystemVersion);
    oh->Win32VersionValue = bela::swaple(oh->Win32VersionValue);
    oh->SizeOfImage = bela::swaple(oh->SizeOfImage);
    oh->SizeOfHeaders = bela::swaple(oh->SizeOfHeaders);
    oh->CheckSum = bela::swaple(oh->CheckSum);
    oh->Subsystem = bela::swaple(oh->Subsystem);
    oh->DllCharacteristics = bela::swaple(oh->DllCharacteristics);
    oh->SizeOfStackReserve = bela::swaple(oh->SizeOfStackReserve);
    oh->SizeOfStackCommit = bela::swaple(oh->SizeOfStackCommit);
    oh->SizeOfHeapReserve = bela::swaple(oh->SizeOfHeapReserve);
    oh->SizeOfHeapCommit = bela::swaple(oh->SizeOfHeapCommit);
    oh->LoaderFlags = bela::swaple(oh->LoaderFlags);
    oh->NumberOfRvaAndSizes = bela::swaple(oh->NumberOfRvaAndSizes);
    for (auto &d : oh->DataDirectory) {
      d.Size = bela::swaple(d.Size);
      d.VirtualAddress = bela::swaple(d.VirtualAddress);
    }
  }
}

void swaple(OptionalHeader32 *oh) {
  if constexpr (bela::IsBigEndian()) {
    oh->Magic = bela::swaple(oh->Magic);
    oh->SizeOfCode = bela::swaple(oh->SizeOfCode);
    oh->SizeOfInitializedData = bela::swaple(oh->SizeOfInitializedData);
    oh->SizeOfUninitializedData = bela::swaple(oh->SizeOfUninitializedData);
    oh->AddressOfEntryPoint = bela::swaple(oh->AddressOfEntryPoint);
    oh->BaseOfCode = bela::swaple(oh->BaseOfCode);
    oh->BaseOfData = bela::swaple(oh->BaseOfData);
    oh->ImageBase = bela::swaple(oh->ImageBase);
    oh->SectionAlignment = bela::swaple(oh->SectionAlignment);
    oh->FileAlignment = bela::swaple(oh->FileAlignment);
    oh->MajorOperatingSystemVersion = bela::swaple(oh->MajorOperatingSystemVersion);
    oh->MinorOperatingSystemVersion = bela::swaple(oh->MinorOperatingSystemVersion);
    oh->MajorImageVersion = bela::swaple(oh->MajorImageVersion);
    oh->MinorImageVersion = bela::swaple(oh->MinorImageVersion);
    oh->MajorSubsystemVersion = bela::swaple(oh->MajorSubsystemVersion);
    oh->MinorSubsystemVersion = bela::swaple(oh->MinorSubsystemVersion);
    oh->Win32VersionValue = bela::swaple(oh->Win32VersionValue);
    oh->SizeOfImage = bela::swaple(oh->SizeOfImage);
    oh->SizeOfHeaders = bela::swaple(oh->SizeOfHeaders);
    oh->CheckSum = bela::swaple(oh->CheckSum);
    oh->Subsystem = bela::swaple(oh->Subsystem);
    oh->DllCharacteristics = bela::swaple(oh->DllCharacteristics);
    oh->SizeOfStackReserve = bela::swaple(oh->SizeOfStackReserve);
    oh->SizeOfStackCommit = bela::swaple(oh->SizeOfStackCommit);
    oh->SizeOfHeapReserve = bela::swaple(oh->SizeOfHeapReserve);
    oh->SizeOfHeapCommit = bela::swaple(oh->SizeOfHeapCommit);
    oh->LoaderFlags = bela::swaple(oh->LoaderFlags);
    oh->NumberOfRvaAndSizes = bela::swaple(oh->NumberOfRvaAndSizes);
    for (auto &d : oh->DataDirectory) {
      d.Size = bela::swaple(d.Size);
      d.VirtualAddress = bela::swaple(d.VirtualAddress);
    }
  }
}

void File::Free() {
  if (fd != nullptr) {
    fclose(fd);
    fd = nullptr;
  }
}

void File::FileMove(File &&other) {
  Free();
  fd = other.fd;
  is64bit = other.is64bit;
  other.fd = nullptr;
  coffsymbol = std::move(other.coffsymbol);
  stringTable = std::move(other.stringTable);
  memcpy(&fh, &other.fh, sizeof(FileHeader));
  memcpy(&oh64, &other.oh64, sizeof(OptionalHeader64));
}

std::optional<File> File::NewFile(std::wstring_view p, bela::error_code &ec) {
  FILE *fd = nullptr;
  if (auto eno = _wfopen_s(&fd, p.data(), L"rb"); eno != 0) {
    ec = bela::make_stdc_error_code(eno, L"open file: ");
    return std::nullopt;
  }
  File file;
  file.fd = fd;
  DosHeader dh;
  if (fread(&dh, 1, sizeof(DosHeader), fd) != sizeof(DosHeader)) {
    ec = bela::make_stdc_error_code(ferror(fd), L"open file: ");
    return std::nullopt;
  }
  constexpr auto x = 0x3c;

  int64_t base = 0;
  if (bela::swaple(dh.e_magic) == IMAGE_DOS_SIGNATURE) {
    auto signoff = static_cast<int64_t>(bela::swaple(dh.e_lfanew));
    uint8_t sign[4];
    if (auto eno = _fseeki64(fd, signoff, SEEK_SET); eno != 0) {
      ec = bela::make_stdc_error_code(eno, L"Invalid PE COFF file signature of ");
      return std::nullopt;
    }
    if (fread(sign, 1, 4, fd) != 4) {
      ec = bela::make_stdc_error_code(ferror(fd), L"Invalid PE COFF file signature of ");
      return std::nullopt;
    }
    if (!(sign[0] == 'P' && sign[1] == 'E' && sign[2] == 0 && sign[3] == 0)) {
      ec = bela::make_error_code(1, L"Invalid PE COFF file signature of ['", int(sign[0]), L"','", int(sign[1]), L"','",
                                 int(sign[2]), L"','", int(sign[3]), L"']");
      return std::nullopt;
    }
    base = signoff + 4;
  }
  if (auto eno = _fseeki64(fd, base, SEEK_SET); eno != 0) {
    ec = bela::make_stdc_error_code(eno, L"unable seek to base");
    return std::nullopt;
  }
  if (fread(&file.fh, 1, sizeof(FileHeader), file.fd) != sizeof(FileHeader)) {
    ec = bela::make_stdc_error_code(ferror(file.fd), L"Invalid PE COFF file FileHeader ");
    return std::nullopt;
  }
  swaple(file.fh);
  file.is64bit = (file.fh.SizeOfOptionalHeader == sizeof(OptionalHeader64));
  if (!readStringTable(&file.fh, fd, file.stringTable, ec)) {
    return std::nullopt;
  }
  if (!readCOFFSymbols(&file.fh, file.fd, file.coffsymbol, ec)) {
    return std::nullopt;
  }

  if (auto eno = _fseeki64(fd, base + sizeof(FileHeader), SEEK_SET); eno != 0) {
    ec = bela::make_stdc_error_code(eno, L"unable seek to base");
    return std::nullopt;
  }
  if (file.is64bit) {
    if (fread(&file.oh64, 1, sizeof(OptionalHeader64), fd) != sizeof(OptionalHeader64)) {
      ec = bela::make_stdc_error_code(ferror(file.fd), L"Invalid PE COFF file OptionalHeader64 ");
      return std::nullopt;
    }
    swaple(&file.oh64);
  } else {
    if (fread(&file.oh64, 1, sizeof(OptionalHeader32), fd) != sizeof(OptionalHeader32)) {
      ec = bela::make_stdc_error_code(ferror(file.fd), L"Invalid PE COFF file OptionalHeader32 ");
      return std::nullopt;
    }
    swaple(reinterpret_cast<OptionalHeader32 *>(&file.oh64));
  }
  return std::make_optional(std::move(file));
}

bool File::LookupImports(symbols_map_t &sm, bela::error_code &ec) {
  uint32_t ddlen = 0;
  if (is64bit) {
    ddlen = Oh64()->NumberOfRvaAndSizes;
  } else {
    ddlen = Oh32()->NumberOfRvaAndSizes;
  }
  return false;
}

} // namespace bela::pe