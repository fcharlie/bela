//
#ifndef HAZEL_ZIP_HPP
#define HAZEL_ZIP_HPP
#include <bela/base.hpp>
#include <bela/buffer.hpp>
#include <bela/narrow/strcat.hpp>
#include <ctime>

#define HAZEL_COMPRESS_LEVEL_DEFAULT (-1)
#define HAZEL_COMPRESS_LEVEL_FAST (2)
#define HAZEL_COMPRESS_LEVEL_NORMAL (6)
#define HAZEL_COMPRESS_LEVEL_BEST (9)

/* HAZEL_ZIP_FLAG */
#define HAZEL_ZIP_FLAG_ENCRYPTED (1 << 0)
#define HAZEL_ZIP_FLAG_LZMA_EOS_MARKER (1 << 1)
#define HAZEL_ZIP_FLAG_DEFLATE_MAX (1 << 1)
#define HAZEL_ZIP_FLAG_DEFLATE_NORMAL (0)
#define HAZEL_ZIP_FLAG_DEFLATE_FAST (1 << 2)
#define HAZEL_ZIP_FLAG_DEFLATE_SUPER_FAST (HAZEL_ZIP_FLAG_DEFLATE_FAST | HAZEL_ZIP_FLAG_DEFLATE_MAX)
#define HAZEL_ZIP_FLAG_DATA_DESCRIPTOR (1 << 3)
#define HAZEL_ZIP_FLAG_UTF8 (1 << 11)
#define HAZEL_ZIP_FLAG_MASK_LOCAL_INFO (1 << 13)

/* HAZEL_ZIP_EXTENSION */
#define HAZEL_ZIP_EXTENSION_ZIP64 (0x0001)
#define HAZEL_ZIP_EXTENSION_NTFS (0x000a)
#define HAZEL_ZIP_EXTENSION_AES (0x9901)
#define HAZEL_ZIP_EXTENSION_UNIX1 (0x000d)
#define HAZEL_ZIP_EXTENSION_SIGN (0x10c5)
#define HAZEL_ZIP_EXTENSION_HASH (0x1a51)
#define HAZEL_ZIP_EXTENSION_CDCD (0xcdcd)

/* HAZEL_ZIP64 */
#define HAZEL_ZIP64_AUTO (0)
#define HAZEL_ZIP64_FORCE (1)
#define HAZEL_ZIP64_DISABLE (2)

/* HAZEL_HOST_SYSTEM */
#define HAZEL_HOST_SYSTEM(VERSION_MADEBY) ((uint8_t)(VERSION_MADEBY >> 8))
#define HAZEL_HOST_SYSTEM_MSDOS (0)
#define HAZEL_HOST_SYSTEM_UNIX (3)
#define HAZEL_HOST_SYSTEM_WINDOWS_NTFS (10)
#define HAZEL_HOST_SYSTEM_RISCOS (13)
#define HAZEL_HOST_SYSTEM_OSX_DARWIN (19)

/* HAZEL_PKCRYPT */
#define HAZEL_PKCRYPT_HEADER_SIZE (12)

/* HAZEL_AES */
#define HAZEL_AES_VERSION (1)
#define HAZEL_AES_ENCRYPTION_MODE_128 (0x01)
#define HAZEL_AES_ENCRYPTION_MODE_192 (0x02)
#define HAZEL_AES_ENCRYPTION_MODE_256 (0x03)
#define HAZEL_AES_KEY_LENGTH(MODE) (8 * (MODE & 3) + 8)
#define HAZEL_AES_KEY_LENGTH_MAX (32)
#define HAZEL_AES_BLOCK_SIZE (16)
#define HAZEL_AES_HEADER_SIZE(MODE) ((4 * (MODE & 3) + 4) + 2)
#define HAZEL_AES_FOOTER_SIZE (10)

/* HAZEL_HASH */
#define HAZEL_HASH_MD5 (10)
#define HAZEL_HASH_MD5_SIZE (16)
#define HAZEL_HASH_SHA1 (20)
#define HAZEL_HASH_SHA1_SIZE (20)
#define HAZEL_HASH_SHA256 (23)
#define HAZEL_HASH_SHA256_SIZE (32)
#define HAZEL_HASH_MAX_SIZE (256)

/* HAZEL_ENCODING */
#define HAZEL_ENCODING_CODEPAGE_437 (437)
#define HAZEL_ENCODING_CODEPAGE_932 (932)
#define HAZEL_ENCODING_CODEPAGE_936 (936)
#define HAZEL_ENCODING_CODEPAGE_950 (950)
#define HAZEL_ENCODING_UTF8 (65001)

namespace hazel::zip {
// https://www.hanshq.net/zip.html

// https://github.com/nih-at/libzip/blob/master/lib/zip.h
typedef enum zip_method_e : uint16_t {
  ZIP_STORE = 0,    /* stored (uncompressed) */
  ZIP_SHRINK = 1,   /* shrunk */
  ZIP_REDUCE_1 = 2, /* reduced with factor 1 */
  ZIP_REDUCE_2 = 3, /* reduced with factor 2 */
  ZIP_REDUCE_3 = 4, /* reduced with factor 3 */
  ZIP_REDUCE_4 = 5, /* reduced with factor 4 */
  ZIP_IMPLODE = 6,  /* imploded */
  /* 7 - Reserved for Tokenizing compression algorithm */
  ZIP_DEFLATE = 8,         /* deflated */
  ZIP_DEFLATE64 = 9,       /* deflate64 */
  ZIP_PKWARE_IMPLODE = 10, /* PKWARE imploding */
  /* 11 - Reserved by PKWARE */
  ZIP_BZIP2 = 12, /* compressed using BZIP2 algorithm */
  /* 13 - Reserved by PKWARE */
  ZIP_LZMA = 14, /* LZMA (EFS) */
  /* 15-17 - Reserved by PKWARE */
  ZIP_TERSE = 18, /* compressed using IBM TERSE (new) */
  ZIP_LZ77 = 19,  /* IBM LZ77 z Architecture (PFS) */
  /* 20 - old value for Zstandard */
  ZIP_LZMA2 = 33,
  ZIP_ZSTD = 93,    /* Zstandard compressed data */
  ZIP_XZ = 95,      /* XZ compressed data */
  ZIP_JPEG = 96,    /* Compressed Jpeg data */
  ZIP_WAVPACK = 97, /* WavPack compressed data */
  ZIP_PPMD = 98,    /* PPMd version I, Rev 1 */
  ZIP_AES = 99,     /* AE-x encryption marker (see APPENDIX E) */
} zip_method_t;

struct directoryEnd {
  uint32_t diskNbr;            // unused
  uint32_t dirDiskNbr;         // unused
  uint64_t dirRecordsThisDisk; // unused
  uint64_t directoryRecords;
  uint64_t directorySize;
  uint64_t directoryOffset; // relative to file
  uint16_t commentLen;
  std::string comment;
};

inline const char *AESStrength(uint8_t i) {
  switch (i) {
  case 1:
    return "AES-128";
  case 2:
    return "AES-192";
  case 3:
    return "AES-256";
  default:
    break;
  }
  return "AES-???";
}

struct File {
  std::string name;
  std::string comment;
  std::string extra;
  uint64_t compressedSize{0};
  uint64_t uncompressedSize{0};
  uint64_t position{0}; // file position
  time_t time{0};
  uint32_t crc32{0};
  uint32_t externalAttrs{0};
  uint16_t cversion{0};
  uint16_t rversion{0};
  uint16_t flags{0};
  uint16_t method{0};
  uint16_t aesVersion{0};
  uint8_t aesStrength{0};
  bool utf8{false};
  bool IsEncrypted() const { return (flags & 0x1) != 0; }
  std::string AesText() const { return bela::narrow::StringCat("AE-", aesVersion, "/", AESStrength(aesStrength)); }
};

class Reader {
public:
  Reader(HANDLE fd_, int64_t sz) : fd(fd_), size(sz) {}
  Reader(const Reader &r) { CopyFrom(r); }
  Reader &operator=(const Reader &r) {
    CopyFrom(r);
    return *this;
  }
  Reader(Reader &&r) { MoveFrom(std::move(r)); }
  Reader &operator=(Reader &&r) {
    MoveFrom(std::move(r));
    return *this;
  }
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
  bool ReadAt(void *buffer, size_t len, uint64_t pos, size_t &outlen, bela::error_code &ec) {
    if (!PositionAt(pos, ec)) {
      return false;
    }
    return Read(buffer, len, outlen, ec);
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
  static std::optional<Reader> NewReader(HANDLE fd, bela::error_code &ec);
  std::string_view Comment() const { return comment; }
  const auto &Files() const { return files; }
  int64_t CompressedSize() const { return compressedSize; }
  int64_t UncompressedSize() const { return uncompressedSize; }

private:
  HANDLE fd;
  int64_t size;
  int64_t uncompressedSize{0};
  int64_t compressedSize{0};
  std::string comment;
  std::vector<File> files;
  void CopyFrom(const Reader &r) {
    fd = r.fd;
    size = r.size;
    comment = r.comment;
    files = r.files;
    uncompressedSize = r.uncompressedSize;
    compressedSize = r.compressedSize;
  }
  void MoveFrom(Reader &&r) {
    fd = r.fd;
    r.fd = INVALID_HANDLE_VALUE;
    size = r.size;
    uncompressedSize = r.uncompressedSize;
    compressedSize = r.compressedSize;
    r.uncompressedSize = 0;
    r.compressedSize = 0;
    r.size = 0;
    comment = std::move(r.comment);
    files = std::move(r.files);
  }

  bool initialize(bela::error_code &ec);
  bool readde(directoryEnd &d, bela::error_code &ec);
  bool readd64e(int64_t offset, directoryEnd &d, bela::error_code &ec);
  int64_t findd64e(int64_t directoryEndOffset, bela::error_code &ec);
};
inline std::optional<Reader> NewReader(HANDLE fd, bela::error_code &ec) { return Reader::NewReader(fd, ec); }
const wchar_t *Method(uint16_t m);

// WindowsTickToUnixTime
// https://stackoverflow.com/questions/20370920/convert-current-time-from-windows-to-unix-timestamp-in-c-or-c
inline time_t WindowsTickToUnixTime(uint64_t tick) {
  constexpr auto tickPerSecond = 10'000'000ll;
  constexpr auto unixTimeStart = 116444736000000000ui64;
  return static_cast<time_t>(tick - unixTimeStart) / tickPerSecond;
}
// DosDateTimeToUnixTime dos time to unix time
// https://msdn.microsoft.com/en-us/library/ms724247(v=VS.85).aspx
// Windows Epoch start 1601
time_t DosDateTimeToUnixTime(uint16_t dosDate, uint16_t dosTime);
} // namespace hazel::zip

#endif