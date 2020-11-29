//
#ifndef HAZEL_ZIP_HPP
#define HAZEL_ZIP_HPP
#include <bela/base.hpp>
#include <ctime>

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

// Thanks Minizip
typedef struct hazel_zip_file_s {
  uint16_t version_madeby;     /* version made by */
  uint16_t version_needed;     /* version needed to extract */
  uint16_t flag;               /* general purpose bit flag */
  uint16_t compression_method; /* compression method */
  time_t modified_date;        /* last modified date in unix time */
  time_t accessed_date;        /* last accessed date in unix time */
  time_t creation_date;        /* creation date in unix time */
  uint32_t crc;                /* crc-32 */
  int64_t compressed_size;     /* compressed size */
  int64_t uncompressed_size;   /* uncompressed size */
  uint16_t filename_size;      /* filename length */
  uint16_t extrafield_size;    /* extra field length */
  uint16_t comment_size;       /* file comment length */
  uint32_t disk_number;        /* disk number start */
  int64_t disk_offset;         /* relative offset of local header */
  uint16_t internal_fa;        /* internal file attributes */
  uint32_t external_fa;        /* external file attributes */

  const char *filename;      /* filename utf8 null-terminated string */
  const uint8_t *extrafield; /* extrafield data */
  const char *comment;       /* comment utf8 null-terminated string */
  const char *linkname;      /* sym-link filename utf8 null-terminated string */

  uint16_t zip64;              /* zip64 extension mode */
  uint16_t aes_version;        /* winzip aes extension if not 0 */
  uint8_t aes_encryption_mode; /* winzip aes encryption mode */
} hazel_zip_file;

#pragma pack()

} // namespace hazel::zip

#endif