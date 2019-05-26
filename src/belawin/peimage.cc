////
#include <type_traits>
#include <bela/base.hpp>
#include <bela/endian.hpp>
#include <bela/pe.hpp>
#include <bela/mapview.hpp>
#include <bela/codecvt.hpp>

namespace bela {
typedef enum ReplacesGeneralNumericDefines {
// Directory entry macro for CLR data.
#ifndef IMAGE_DIRECTORY_ENTRY_COMHEADER
  IMAGE_DIRECTORY_ENTRY_COMHEADER = 14,
#endif // IMAGE_DIRECTORY_ENTRY_COMHEADER
} ReplacesGeneralNumericDefines;
#define STORAGE_MAGIC_SIG 0x424A5342 // BSJB

#pragma pack(1)
struct STORAGESIGNATURE {
  ULONG Signature;     // Magic signature for physical metadata : 0x424A5342.
  USHORT MajorVersion; // Major version, 1 (ignore on read)
  USHORT MinorVersion; // Minor version, 0 (ignore on read)
  ULONG ExtraData;     // offset to next structure of information
  ULONG Length;        // Length of version string in bytes
};
#pragma pack()
// LE endian
static inline PVOID belarva(PVOID m, PVOID b) {
  return reinterpret_cast<PVOID>(reinterpret_cast<ULONG_PTR>(b) +
                                 reinterpret_cast<ULONG_PTR>(m));
}

PIMAGE_SECTION_HEADER
BelaImageRvaToSection(PIMAGE_NT_HEADERS nh, PVOID BaseAddress, ULONG rva) {
  ULONG count = bela::Swaple(nh->FileHeader.NumberOfSections);
  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nh);
  ULONG va = 0;
  while (count-- != 0) {
    va = bela::Swaple(section->VirtualAddress);
    if ((va <= rva) && (rva < va + bela::Swaple(section->SizeOfRawData))) {
      return section;
    }
    section++;
  }
  return nullptr;
}

// like RtlImageRvaToVa
PVOID
BelaImageRvaToVa(PIMAGE_NT_HEADERS nh, PVOID BaseAddress, ULONG rva,
                 PIMAGE_SECTION_HEADER *sh) {
  PIMAGE_SECTION_HEADER section = nullptr;
  if (sh != nullptr) {
    section = *sh;
  }
  if ((section == nullptr) || (rva < bela::Swaple(section->VirtualAddress)) ||
      (rva >= bela::Swaple(section->VirtualAddress) +
                  bela::Swaple(section->SizeOfRawData))) {
    section = BelaImageRvaToSection(nh, BaseAddress, rva);
    if (section == nullptr) {
      return nullptr;
    }
    if (sh) {
      *sh = section;
    }
  }
  auto va = reinterpret_cast<ULONG_PTR>(BaseAddress) + rva +
            static_cast<ULONG_PTR>(bela::Swaple(section->PointerToRawData)) -
            static_cast<ULONG_PTR>(bela::Swaple(section->VirtualAddress));
  return reinterpret_cast<PVOID>(va);
}
// DLL Name is CP_ACP
std::wstring fromascii(std::string_view sv) {
  auto sz =
      MultiByteToWideChar(CP_ACP, 0, sv.data(), (int)sv.size(), nullptr, 0);
  std::wstring output;
  output.resize(sz);
  // C++17 must output.data()
  MultiByteToWideChar(CP_ACP, 0, sv.data(), (int)sv.size(), output.data(), sz);
  return output;
}

inline std::wstring DllName(MemView mv, LPVOID nh, ULONG nva) {
  auto va =
      BelaImageRvaToVa((PIMAGE_NT_HEADERS)nh, (LPVOID)mv.data(), nva, nullptr);
  if (va == nullptr) {
    return L"";
  }
  auto begin = reinterpret_cast<const char *>(va);
  auto it = begin;
  auto end = reinterpret_cast<const char *>(mv.data() + mv.size());
  for (; it < end && *it != 0; it++) {
    ;
  }
  if (it >= end) {
    return L""; // BAD string table
  }
  std::string_view name(begin, it - begin);
  return fromascii(name);
}

inline std::wstring ClrMessage(MemView mv, LPVOID nh, ULONG clrva) {
  auto va = BelaImageRvaToVa((PIMAGE_NT_HEADERS)nh, (LPVOID)mv.data(), clrva,
                             nullptr);
  auto end = mv.data() + mv.size();
  if (va == nullptr ||
      reinterpret_cast<uint8_t *>(va) + sizeof(IMAGE_COR20_HEADER) > end) {
    return L"";
  }
  auto clrh = reinterpret_cast<PIMAGE_COR20_HEADER>(va);
  auto va2 = BelaImageRvaToVa((PIMAGE_NT_HEADERS)nh, (LPVOID)mv.data(),
                              clrh->MetaData.VirtualAddress, nullptr);
  if (va2 == nullptr ||
      reinterpret_cast<uint8_t *>(va2) + sizeof(STORAGESIGNATURE) > end) {
    return L"";
  }
  auto clrmsg = reinterpret_cast<const STORAGESIGNATURE *>(va2);
  if (reinterpret_cast<const uint8_t *>(clrmsg) + clrmsg->Length > end) {
    return L"";
  }
  std::string_view u8msg((const char *)clrmsg + sizeof(STORAGESIGNATURE),
                         clrmsg->Length);
  return bela::ToWide(u8msg);
}

template <typename H = IMAGE_NT_HEADERS64>
std::optional<PESimpleDetails>
PESimpleDetailsInternal(bela::MemView mv, const H *nh, bela::error_code &ec) {
  PESimpleDetails pm;
  pm.machine = static_cast<Machine>(bela::Swaple(nh->FileHeader.Machine));
  pm.characteristics = bela::Swaple(nh->FileHeader.Characteristics);
  pm.dllcharacteristics = bela::Swaple(nh->OptionalHeader.DllCharacteristics);
  pm.osver = {bela::Swaple(nh->OptionalHeader.MajorOperatingSystemVersion),
              bela::Swaple(nh->OptionalHeader.MinorOperatingSystemVersion)};
  pm.subsystem =
      static_cast<Subsytem>(bela::Swaple(nh->OptionalHeader.Subsystem));
  pm.linkver = {bela::Swaple(nh->OptionalHeader.MajorLinkerVersion),
                bela::Swaple(nh->OptionalHeader.MinorLinkerVersion)};
  pm.imagever = {bela::Swaple(nh->OptionalHeader.MajorImageVersion),
                 bela::Swaple(nh->OptionalHeader.MinorImageVersion)};
  auto clre =
      &(nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER]);
  auto end = mv.data() + mv.size();
  if (bela::Swaple(clre->Size) == sizeof(IMAGE_COR20_HEADER)) {
    // Exists IMAGE_COR20_HEADER
    pm.clrmsg = ClrMessage(mv, (PVOID)nh, bela::Swaple(clre->VirtualAddress));
  }

  // Import
  auto import_ =
      &(nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);
  if (bela::Swaple(import_->Size) != 0) {
    auto va = BelaImageRvaToVa((PIMAGE_NT_HEADERS)nh, (PVOID)mv.data(),
                               bela::Swaple(import_->VirtualAddress), nullptr);
    if (va == nullptr ||
        reinterpret_cast<uint8_t *>(va) + bela::Swaple(import_->Size) >= end) {
      return std::make_optional<>(pm);
    }
    auto imdes = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(va);
    while (bela::Swaple(imdes->Name) != 0) {
      //
      // ASCIIZ
      auto dnw = DllName(mv, (LPVOID)nh, bela::Swaple(imdes->Name));
      if (!dnw.empty()) {
        pm.depends.push_back(dnw);
      }
      imdes++;
    }
  }

  /// Delay import
  auto delay_ =
      &(nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]);
  if (bela::Swaple(delay_->Size) != 0) {
    auto va = BelaImageRvaToVa((PIMAGE_NT_HEADERS)nh, (PVOID)mv.data(),
                               bela::Swaple(delay_->VirtualAddress), nullptr);
    if (va == nullptr ||
        reinterpret_cast<uint8_t *>(va) + bela::Swaple(delay_->Size) >= end) {
      return std::make_optional<>(pm);
    }
    auto imdes = reinterpret_cast<PIMAGE_DELAYLOAD_DESCRIPTOR>(va);
    while (bela::Swaple(imdes->DllNameRVA) != 0) {
      //
      // ASCIIZ
      auto dnw = DllName(mv, (LPVOID)nh, bela::Swaple(imdes->DllNameRVA));
      if (!dnw.empty()) {
        pm.delays.push_back(dnw);
      }
      imdes++;
    }
  }

  // IMAGE_DIRECTORY_ENTRY_RESOURCE resolve copyright

  return std::make_optional<PESimpleDetails>(std::move(pm));
}

std::optional<PESimpleDetails> PESimpleDetailsAze(std::wstring_view file,
                                                  bela::error_code &ec) {
  constexpr size_t peminsize =
      sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32);
  bela::MapView mapview;
  if (!mapview.MappingView(file, ec, peminsize)) {
    return std::nullopt;
  }
  auto mv = mapview.subview();
  auto h = mv.cast<IMAGE_DOS_HEADER>(0);
  if (h == nullptr) {
    ec = bela::make_error_code(
        bela::FileSizeTooSmall,
        L"PE file size tool small, less IMAGE_DOS_HEADER");
    return std::nullopt;
  }
  auto nh = mv.cast<IMAGE_NT_HEADERS32>(bela::Swaple(h->e_lfanew));
  if (nh == nullptr) {
    ec = bela::make_error_code(
        bela::FileSizeTooSmall,
        L"PE file size tool small, less IMAGE_NT_HEADERS32");
    return std::nullopt;
  }
  switch (bela::Swaple(nh->OptionalHeader.Magic)) {
  case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
    return PESimpleDetailsInternal(
        mv, reinterpret_cast<const IMAGE_NT_HEADERS64 *>(nh), ec);
  case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
    return PESimpleDetailsInternal(
        mv, reinterpret_cast<const IMAGE_NT_HEADERS32 *>(nh), ec);
  case IMAGE_ROM_OPTIONAL_HDR_MAGIC: {
    // Not implemented
  } break;
  default:
    break;
  }
  return std::nullopt;
}

} // namespace bela