///
#include <hazel/zip.hpp>
#include <hazel/hazel.hpp>
#include <bela/path.hpp>
#include <bela/endian.hpp>
#include <bela/algorithm.hpp>
#include <numeric>

namespace hazel::zip {
// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
// https://en.wikipedia.org/wiki/ZIP_(file_format)
// https://en.wikipedia.org/wiki/Comparison_of_file_archivers
// https://en.wikipedia.org/wiki/List_of_archive_formats
constexpr int fileHeaderSignature = 0x04034b50;
constexpr int directoryHeaderSignature = 0x02014b50;
constexpr int directoryEndSignature = 0x06054b50;
constexpr int directory64LocSignature = 0x07064b50;
constexpr int directory64EndSignature = 0x06064b50;
constexpr uint32_t dataDescriptorSignature = 0x08074b50; // de-facto standard; required by OS X Finder
constexpr uint32_t fileHeaderLen = 30;                   // + filename + extra
constexpr int directoryHeaderLen = 46;                   // + filename + extra + comment
constexpr int directoryEndLen = 22;                      // + comment
constexpr int dataDescriptorLen = 16;   // four uint32: descriptor signature, crc32, compressed size, size
constexpr int dataDescriptor64Len = 24; // descriptor with 8 byte sizes
constexpr int directory64LocLen = 20;   //
constexpr int directory64EndLen = 56;   // + extra

// Constants for the first byte in CreatorVersion.
constexpr int creatorFAT = 0;
constexpr int creatorUnix = 3;
constexpr int creatorNTFS = 11;
constexpr int creatorVFAT = 14;
constexpr int creatorMacOSX = 19;

// Version numbers.
constexpr int zipVersion20 = 20; // 2.0
constexpr int zipVersion45 = 45; // 4.5 (reads and writes zip64 archives)

// Limits for non zip64 files.
constexpr auto uint16max = (std::numeric_limits<uint16_t>::max)();
constexpr auto uint32max = (std::numeric_limits<uint32_t>::max)();

// Extra header IDs.
//
// IDs 0..31 are reserved for official use by PKWARE.
// IDs above that range are defined by third-party vendors.
// Since ZIP lacked high precision timestamps (nor a official specification
// of the timezone used for the date fields), many competing extra fields
// have been invented. Pervasive use effectively makes them "official".
//
// See http://mdfs.net/Docs/Comp/Archiving/Zip/ExtraField
constexpr int zip64ExtraID = 0x0001;       // Zip64 extended information
constexpr int ntfsExtraID = 0x000a;        // NTFS
constexpr int unixExtraID = 0x000d;        // UNIX
constexpr int extTimeExtraID = 0x5455;     // Extended timestamp
constexpr int infoZipUnixExtraID = 0x5855; // Info-ZIP Unix extension

inline bool IsSuperficialPath(std::string_view sv) {
  auto pv = bela::SplitPath(sv);
  return pv.size() <= 3;
}

int findSignatureInBlock(const bela::Buffer &b) {
  for (auto i = static_cast<int>(b.size()) - directoryEndLen; i >= 0; i--) {
    if (b[i] == 'P' && b[i + 1] == 'K' && b[i + 2] == 0x05 && b[i + 3] == 0x06) {
      auto n = static_cast<int>(b[i + directoryEndLen - 2]) | (static_cast<int>(b[i + directoryEndLen - 1]) << 8);
      if (n + directoryEndLen + i <= static_cast<int>(b.size())) {
        return i;
      }
    }
  }
  return -1;
}
bool Reader::readd64e(int64_t offset, directoryEnd &d, bela::error_code &ec) {
  bela::Buffer b(directory64EndLen);
  if (!ReadAt(b, directory64EndLen, offset, ec)) {
    return false;
  }
  std::string_view sv{reinterpret_cast<const char *>(b.data()), b.size()};
  if (auto sig = bela::readle<uint32_t>(sv.data()); sig != directory64EndSignature) {
    ec = bela::make_error_code(L"zip: not a valid zip file");
    return false;
  }
  sv.remove_prefix(16);
  d.diskNbr = bela::readle<uint32_t>(sv.data());        // number of this disk
  d.dirDiskNbr = bela::readle<uint32_t>(sv.data() + 4); // number of the disk with the start of the central directory
                                                        // total number of entries in the central directory on this disk
  d.dirRecordsThisDisk = bela::readle<uint64_t>(sv.data() + 8);
  d.directoryRecords = bela::readle<uint64_t>(sv.data() + 16); // total number of entries in the central directory
  d.directorySize = bela::readle<uint64_t>(sv.data() + 24);    // size of the central directory
  // offset of start of central directory with respect to the starting disk number
  d.directoryOffset = bela::readle<uint64_t>(sv.data() + 32);

  return true;
}
int64_t Reader::findd64e(int64_t directoryEndOffset, bela::error_code &ec) {
  auto locOffset = directoryEndOffset - directory64LocLen;
  if (locOffset < 0) {
    return -1;
  }
  bela::Buffer b(directory64LocLen);
  if (!ReadAt(b, directory64LocLen, locOffset, ec)) {
    return -1;
  }
  std::string_view sv{reinterpret_cast<const char *>(b.data()), b.size()};
  if (auto sig = bela::readle<uint32_t>(sv.data()); sig != directory64LocSignature) {
    return -1;
  }
  if (bela::readle<uint32_t>(sv.data() + 4) != 0) {
    return -1;
  }
  auto p = bela::readle<uint64_t>(sv.data() + 8);
  if (bela::readle<uint32_t>(sv.data() + 16) != 1) {
    return -1;
  }
  return static_cast<int64_t>(p);
}

// github.com\klauspost\compress@v1.11.3\zip\reader.go
bool Reader::readde(directoryEnd &d, bela::error_code &ec) {
  bela::Buffer b(16 * 1024);
  int64_t deoffset = 0;
  constexpr int64_t offrange[] = {1024, 65 * 1024};
  std::string_view sv;
  for (size_t i = 0; i < bela::ArrayLength(offrange); i++) {
    auto blen = offrange[i];
    if (blen > size) {
      blen = size;
    }
    b.grow(blen);
    if (!ReadAt(b, blen, size - blen, ec)) {
      return false;
    }
    if (auto p = findSignatureInBlock(b); p >= 0) {
      sv = std::string_view{reinterpret_cast<const char *>(b.data()) + p, b.size() - p};
      deoffset = size - blen + p;
      break;
    }
    if (i == 1 || blen == size) {
      ec = bela::make_error_code(L"zip: not a valid zip file");
      return false;
    }
  }
  sv.remove_prefix(4);
  d.diskNbr = bela::readle<uint16_t>(sv.data());
  d.dirDiskNbr = bela::readle<uint16_t>(sv.data() + 2);
  d.dirRecordsThisDisk = bela::readle<uint16_t>(sv.data() + 4);
  d.directoryRecords = bela::readle<uint16_t>(sv.data() + 6);
  d.directorySize = bela::readle<uint32_t>(sv.data() + 8);
  d.directoryOffset = bela::readle<uint32_t>(sv.data() + 12);
  d.commentLen = bela::readle<uint16_t>(sv.data() + 16);
  if (static_cast<size_t>(d.commentLen + 18) > sv.size()) {
    ec = bela::make_error_code(L"zip: invalid comment length");
    return false;
  }
  d.comment.assign(sv.data() + 18, d.commentLen);
  if (d.directoryRecords == 0xFFFF || d.directorySize == 0xFFFF || d.directoryOffset == 0xFFFFFFFF) {
    /// TODO
    bela::error_code e2;
    auto p = findd64e(deoffset, ec);
    if (!ec && p > 0) {
      readd64e(p, d, ec);
    }
    if (ec) {
      return false;
    }
  }
  if (auto o = static_cast<int64_t>(d.directoryOffset); o < 0 || 0 >= size) {
    ec = bela::make_error_code(L"zip: not a valid zip file");
    return false;
  }
  return true;
}

bool Reader::initialize(bela::error_code &ec) {
  directoryEnd d;
  if (!readde(d, ec)) {
    return false;
  }
  if (d.directoryRecords > static_cast<uint64_t>(size) / fileHeaderLen) {
    ec = bela::make_error_code(1, L"zip: TOC declares impossible ", d.directoryRecords, L" files in ", size,
                               L" byte zip");
    return false;
  }
  comment = std::move(d.comment);
  // file counst d.directoryRecords
  return true;
}

std::optional<Reader> Reader::NewReader(HANDLE fd, bela::error_code &ec) {
  LARGE_INTEGER li;
  if (!GetFileSizeEx(fd, &li)) {
    ec = bela::make_system_error_code(L"GetFileSizeEx: ");
    return std::nullopt;
  }
  Reader r(fd, li.QuadPart);
  if (!r.initialize(ec)) {
    return std::nullopt;
  }
  return std::make_optional(std::move(r));
}

const wchar_t *MethodString(hazel::zip::zip_method_t m) {
  struct method_kv_t {
    hazel::zip::zip_method_t m;
    const wchar_t *name;
  };
  constexpr const method_kv_t methods[] = {
      {zip_method_t::ZIP_STORE, L"store"},
      {zip_method_t::ZIP_SHRINK, L"shrunk"},
      {zip_method_t::ZIP_REDUCE_1, L"ZIP_REDUCE_1"},
      {zip_method_t::ZIP_REDUCE_2, L"ZIP_REDUCE_2"},
      {zip_method_t::ZIP_REDUCE_3, L"ZIP_REDUCE_3"},
      {zip_method_t::ZIP_REDUCE_4, L"ZIP_REDUCE_4"},
      {zip_method_t::ZIP_IMPLODE, L"IMPLODE"},
      {zip_method_t::ZIP_DEFLATE, L"deflate"},
      {zip_method_t::ZIP_DEFLATE64, L"deflate64"},
      {zip_method_t::ZIP_PKWARE_IMPLODE, L"ZIP_PKWARE_IMPLODE"},
      {zip_method_t::ZIP_BZIP2, L"bzip2"},
      {zip_method_t::ZIP_LZMA, L"lzma"},
      {zip_method_t::ZIP_TERSE, L"IBM TERSE"},
      {zip_method_t::ZIP_LZ77, L"LZ77"},
      {zip_method_t::ZIP_LZMA2, L"lzma2"},
      {zip_method_t::ZIP_ZSTD, L"zstd"},
      {zip_method_t::ZIP_XZ, L"xz"},
      {zip_method_t::ZIP_JPEG, L"Jpeg"},
      {zip_method_t::ZIP_WAVPACK, L"WavPack"},
      {zip_method_t::ZIP_PPMD, L"PPMd"},
      {zip_method_t::ZIP_AES, L"AES"},
  };
  for (const auto &i : methods) {
    if (i.m == m) {
      return i.name;
    }
  }
  return L"NONE";
}

} // namespace hazel::zip