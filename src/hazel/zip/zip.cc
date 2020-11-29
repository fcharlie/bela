///
#include <hazel/zip.hpp>
#include <hazel/hazel.hpp>
#include <bela/path.hpp>
#include <bela/endian.hpp>
#include <bela/algorithm.hpp>

namespace hazel::zip {
// see https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
#pragma pack(2)

/*
      local file header signature     4 bytes  (0x04034b50)
      version needed to extract       2 bytes
      general purpose bit flag        2 bytes
      compression method              2 bytes
      last mod file time              2 bytes
      last mod file date              2 bytes
      crc-32                          4 bytes
      compressed size                 4 bytes
      uncompressed size               4 bytes
      file name length                2 bytes
      extra field length              2 bytes

      file name (variable size)
      extra field (variable size)
*/

struct zip_file_header_t {
  uint8_t signature[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t flags;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
  uint32_t compressed_size;
  uint32_t uncompressed_size;
  uint16_t namelen;
  uint16_t fieldlength;
  // file name (variable size)
  // extra field (variable size)
  // file data
};

struct zip_file_header64_t {
  uint8_t signature[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t flags;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
  uint64_t compressed_size;
  uint64_t uncompressed_size;
  uint16_t namelen;
  uint16_t fieldlength;
  // file name (variable size)
  // extra field (variable size)
  // file data
};

struct zip_partial_header_t {
  uint8_t signature[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t flags;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
};

struct zip_partial_header32_t {
  uint32_t compressed_size;
  uint32_t uncompressed_size;
  uint16_t namelen;
  uint16_t fieldlength;
};

struct zip_partial_header64_t {
  uint64_t compressed_size;
  uint64_t uncompressed_size;
  uint16_t namelen;
  uint16_t fieldlength;
};

// signature + version + flags + method + mtime + crc32
constexpr size_t algin_msize = sizeof(zip_partial_header_t);

// Zero-byte files, directories, and other file types that contain no content MUST NOT include file data.

// When compressing files, compressed and uncompressed sizes SHOULD be stored in ZIP64 format (as 8 byte values) when a
// file's size exceeds 0xFFFFFFFF.   However ZIP64 format MAY be used regardless of the size of a file.  When
// extracting, if the zip64 extended information extra field is present for the file the compressed and uncompressed
// sizes will be 8 byte values.

#pragma pack()

inline bool IsSuperficialPath(std::string_view sv) {
  auto pv = bela::SplitPath(sv);
  return pv.size() <= 3;
}
constexpr char lfh_signature[] = {"PK\3\4"};
constexpr char cfh_signature[] = {"PK\1\2"}; // 0x02014b50
constexpr char eocdr_signature[] = {"PK\5\6"};

constexpr int directoryEndLen = 22;
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
// github.com\klauspost\compress@v1.11.3\zip\reader.go
bool Reader::readde(directory_end &d, bela::error_code &ec) {
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
  }
  if (auto o = static_cast<int64_t>(d.directoryOffset); o < 0 || 0 >= size) {
    ec = bela::make_error_code(L"zip: not a valid zip file");
    return false;
  }
  return true;
}

bool Reader::initialize(bela::error_code &ec) {
  //

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

const wchar_t *ZipMethodName(uint16_t m) {
  switch (m) {
  case zip_method_t::ZIP_STORE:
    return L"store";
  case zip_method_t::ZIP_SHRINK:
    return L"shrunk";
  case zip_method_t::ZIP_REDUCE_1:
    return L"REDUCE_1";
  case zip_method_t::ZIP_REDUCE_2:
    return L"REDUCE_2";
  case zip_method_t::ZIP_REDUCE_3:
    return L"REDUCE_3";
  case zip_method_t::ZIP_REDUCE_4:
    return L"REDUCE_4";
  case zip_method_t::ZIP_IMPLODE:
    return L"IMPLODE";
  case zip_method_t::ZIP_DEFLATE:
    return L"deflate";
  case zip_method_t::ZIP_DEFLATE64:
    return L"deflate64";
  case zip_method_t::ZIP_PKWARE_IMPLODE:
    return L"PKWARE_IMPLODE";
  case zip_method_t::ZIP_BZIP2:
    return L"bzip2";
  case zip_method_t::ZIP_LZMA:
    return L"lzma";
  case zip_method_t::ZIP_TERSE:
    return L"IBM TERSE";
  case zip_method_t::ZIP_LZ77:
    return L"LZ77";
  case zip_method_t::ZIP_LZMA2:
    return L"lzma2";
  case zip_method_t::ZIP_ZSTD:
    return L"zstd";
  case zip_method_t::ZIP_XZ:
    return L"xz";
  case zip_method_t::ZIP_JPEG:
    return L"Jpeg";
  case zip_method_t::ZIP_WAVPACK:
    return L"WavPack";
  case zip_method_t::ZIP_PPMD:
    return L"PPMd";
  case zip_method_t::ZIP_AES:
    return L"AES";
  default:
    break;
  }
  return L"NONE";
}

} // namespace hazel::zip