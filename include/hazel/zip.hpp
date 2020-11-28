//
#ifndef HAZEL_ZIP_HPP
#define HAZEL_ZIP_HPP
#include <bela/base.hpp>

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
} zip_method_t;

#pragma pack(2)
struct zip_file_header_t {
  uint8_t magic[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t bitflag;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
  uint32_t compressedsize;
  uint32_t uncompressedsize;
  uint16_t namelen;
  uint16_t fieldlength;
};

struct zip_file_info64_t {
  uint16_t version;
  uint16_t versionneeded;
  uint16_t flag;
  uint16_t method;
  uint32_t dosdate; // uint61_t + uint64_t
  uint32_t crc;
  uint64_t compressedsize;
  uint64_t uncompressedsize;
  uint16_t sizefilename;
  uint16_t sizefileextra;
  uint16_t sizefilecomment;
  uint16_t disknumstart;
  uint16_t internalfa;
  uint16_t externalfa;
};

#pragma pack()

} // namespace hazel::zip

#endif