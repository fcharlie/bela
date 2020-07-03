#ifndef BELA_HASH_HPP
#define BELA_HASH_HPP
#include <cstdint>
#include <string>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

#define BLAKE3_KEY_LEN 32
#define BLAKE3_OUT_LEN 32
#define BLAKE3_BLOCK_LEN 64
#define BLAKE3_CHUNK_LEN 1024
#define BLAKE3_MAX_DEPTH 54
#define BLAKE3_MAX_SIMD_DEGREE 16

// This struct is a private implementation detail. It has to be here because
// it's part of blake3_hasher below.
typedef struct {
  uint32_t cv[8];
  uint64_t chunk_counter;
  uint8_t buf[BLAKE3_BLOCK_LEN];
  uint8_t buf_len;
  uint8_t blocks_compressed;
  uint8_t flags;
} blake3_chunk_state;

typedef struct {
  uint32_t key[8];
  blake3_chunk_state chunk;
  uint8_t cv_stack_len;
  // The stack size is MAX_DEPTH + 1 because we do lazy merging. For example,
  // with 7 chunks, we have 3 entries in the stack. Adding an 8th chunk
  // requires a 4th entry, rather than merging everything down to 1, because we
  // don't know whether more input is coming. This is different from how the
  // reference implementation does things.
  uint8_t cv_stack[(BLAKE3_MAX_DEPTH + 1) * BLAKE3_OUT_LEN];
} blake3_hasher;

void blake3_hasher_init(blake3_hasher *self);
void blake3_hasher_init_keyed(blake3_hasher *self, const uint8_t key[BLAKE3_KEY_LEN]);
void blake3_hasher_init_derive_key(blake3_hasher *self, const char *context);
void blake3_hasher_update(blake3_hasher *self, const void *input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher *self, uint8_t *out, size_t out_len);
void blake3_hasher_finalize_seek(const blake3_hasher *self, uint64_t seek, uint8_t *out,
                                 size_t out_len);

#ifdef __cplusplus
}
#endif

namespace bela::hash {
namespace sha256 {
constexpr auto sha256_block_size = 64;
constexpr auto sha256_hash_size = 32;
constexpr auto sha224_hash_size = 28;

struct hasher {
  uint32_t message[16];   /* 512-bit buffer for leftovers */
  uint64_t length;        /* number of processed bytes */
  uint32_t hash[8];       /* 256-bit algorithm internal hashing state */
  uint32_t digest_length; /* length of the algorithm digest in bytes */
  void init224();
  void init256();
  void update(const void *input, size_t input_len);
  void finalize(uint8_t *out, size_t out_len);
};
} // namespace sha256
namespace sha512 {
constexpr auto sha512_block_size = 128;
constexpr auto sha512_hash_size = 64;
constexpr auto sha384_hash_size = 48;

struct hasher {
  uint64_t message[16];   /* 1024-bit buffer for leftovers */
  uint64_t length;        /* number of processed bytes */
  uint64_t hash[8];       /* 512-bit algorithm internal hashing state */
  uint32_t digest_length; /* length of the algorithm digest in bytes */
  void init384();
  void init512();
  void update(const void *input, size_t input_len);
  void finalize(uint8_t *out, size_t out_len);
};
} // namespace sha512

namespace sha3 {
constexpr auto sha3_224_hash_size = 28;
constexpr auto sha3_256_hash_size = 32;
constexpr auto sha3_384_hash_size = 48;
constexpr auto sha3_512_hash_size = 64;
constexpr auto sha3_max_permutation_size = 25;
constexpr auto sha3_max_rate_in_qwords = 24;

struct hasher {
  /* 1600 bits algorithm hashing state */
  uint64_t hash[sha3_max_permutation_size];
  /* 1536-bit buffer for leftovers */
  uint64_t message[sha3_max_rate_in_qwords];
  /* count of bytes in the message[] buffer */
  uint32_t rest;
  /* size of a message block processed at once */
  uint32_t block_size;
  void init224();
  void init256();
  void init384();
  void init512();
  void update(const void *input, size_t input_len);
  void finalize(uint8_t *out, size_t out_len);
};
} // namespace sha3
namespace blake3 {
struct hasher {
  blake3_hasher h;
  inline void init() { //
    blake3_hasher_init(&h);
  }
  inline void init_keyed(const uint8_t key[BLAKE3_KEY_LEN]) { //
    blake3_hasher_init_keyed(&h, key);
  }
  inline void init_derive_key(const char *context) { //
    blake3_hasher_init_derive_key(&h, context);
  }
  inline void update(const void *input, size_t input_len) {
    blake3_hasher_update(&h, input, input_len);
  }
  inline void finalize(uint8_t *out, size_t out_len) { //
    blake3_hasher_finalize(&h, out, out_len);
  }
  inline void finalize_seek(uint64_t seek, uint8_t *out, size_t out_len) { //
    blake3_hasher_finalize_seek(&h, seek, out, out_len);
  }
};
} // namespace blake3

inline void HashEncode(const uint8_t *b, size_t len, std::wstring &hv) {
  hv.resize(len * 2);
  auto p = hv.data();
  constexpr char hex[] = "0123456789abcdef";
  for (size_t i = 0; i < len; i++) {
    uint32_t val = b[i];
    *p++ = hex[val >> 4];
    *p++ = hex[val & 0xf];
  }
}

} // namespace bela::hash

#endif