//
#ifndef HAZEL_ZIP_HPP
#define HAZEL_ZIP_HPP
#include <bela/base.hpp>
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