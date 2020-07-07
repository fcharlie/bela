/// SM3
#include <bela/hash.hpp>
#include "hashinternal.hpp"

#define ROTATE(a, n) (((a) << (n)) | (((a)&0xffffffff) >> (32 - (n))))

#if IS_BIG_ENDIAN

#define HOST_c2l(c, l)                                                                             \
  (l = (((unsigned long)(*((c)++))) << 24), l |= (((unsigned long)(*((c)++))) << 16),              \
   l |= (((unsigned long)(*((c)++))) << 8), l |= (((unsigned long)(*((c)++)))))
#define HOST_l2c(l, c)                                                                             \
  (*((c)++) = (unsigned char)(((l) >> 24) & 0xff), *((c)++) = (unsigned char)(((l) >> 16) & 0xff), \
   *((c)++) = (unsigned char)(((l) >> 8) & 0xff), *((c)++) = (unsigned char)(((l)) & 0xff), l)

#else

#define HOST_c2l(c, l)                                                                             \
  (l = (((unsigned long)(*((c)++)))), l |= (((unsigned long)(*((c)++))) << 8),                     \
   l |= (((unsigned long)(*((c)++))) << 16), l |= (((unsigned long)(*((c)++))) << 24))
#define HOST_l2c(l, c)                                                                             \
  (*((c)++) = (unsigned char)(((l)) & 0xff), *((c)++) = (unsigned char)(((l) >> 8) & 0xff),        \
   *((c)++) = (unsigned char)(((l) >> 16) & 0xff), *((c)++) = (unsigned char)(((l) >> 24) & 0xff), \
   l)

#endif

#define P0(X) (X ^ ROTATE(X, 9) ^ ROTATE(X, 17))
#define P1(X) (X ^ ROTATE(X, 15) ^ ROTATE(X, 23))

#define FF0(X, Y, Z) (X ^ Y ^ Z)
#define GG0(X, Y, Z) (X ^ Y ^ Z)

#define FF1(X, Y, Z) ((X & Y) | ((X | Y) & Z))
#define GG1(X, Y, Z) ((Z ^ (X & (Y ^ Z))))

#define EXPAND(W0, W7, W13, W3, W10) (P1(W0 ^ W7 ^ ROTATE(W13, 15)) ^ ROTATE(W3, 7) ^ W10)

#define RND(A, B, C, D, E, F, G, H, TJ, Wi, Wj, FF, GG)                                            \
  do {                                                                                             \
    const uint32_t A12 = ROTATE(A, 12);                                                            \
    const uint32_t A12_SM = A12 + E + TJ;                                                          \
    const uint32_t SS1 = ROTATE(A12_SM, 7);                                                        \
    const uint32_t TT1 = FF(A, B, C) + D + (SS1 ^ A12) + (Wj);                                     \
    const uint32_t TT2 = GG(E, F, G) + H + SS1 + Wi;                                               \
    B = ROTATE(B, 9);                                                                              \
    D = TT1;                                                                                       \
    F = ROTATE(F, 19);                                                                             \
    H = P0(TT2);                                                                                   \
  } while (0)

#define R1(A, B, C, D, E, F, G, H, TJ, Wi, Wj) RND(A, B, C, D, E, F, G, H, TJ, Wi, Wj, FF0, GG0)

#define R2(A, B, C, D, E, F, G, H, TJ, Wi, Wj) RND(A, B, C, D, E, F, G, H, TJ, Wi, Wj, FF1, GG1)

namespace bela::hash::sm3 {
void Hasher::Initialize() {
  constexpr uint32_t SM3_S0[] = {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
                                 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e};
  A = SM3_S0[0];
  B = SM3_S0[1];
  C = SM3_S0[2];
  D = SM3_S0[3];
  E = SM3_S0[4];
  F = SM3_S0[5];
  G = SM3_S0[6];
  H = SM3_S0[7];
}

void sm3_block_data_order(Hasher *ctx, const void *p, size_t num) {
  const unsigned char *data = reinterpret_cast<const uint8_t *>(p);
  uint32_t A{0};
  uint32_t B{0};
  uint32_t C{0};
  uint32_t D{0};
  uint32_t E{0};
  uint32_t F{0};
  uint32_t G{0};
  uint32_t H{0};

  uint32_t W00{0};
  uint32_t W01{0};
  uint32_t W02{0};
  uint32_t W03{0};
  uint32_t W04{0};
  uint32_t W05{0};
  uint32_t W06{0};
  uint32_t W07{0};
  uint32_t W08{0};
  uint32_t W09{0};
  uint32_t W10{0};
  uint32_t W11{0};
  uint32_t W12{0};
  uint32_t W13{0};
  uint32_t W14{0};
  uint32_t W15{0};

  for (; num--;) {

    A = ctx->A;
    B = ctx->B;
    C = ctx->C;
    D = ctx->D;
    E = ctx->E;
    F = ctx->F;
    G = ctx->G;
    H = ctx->H;

    /*
     * We have to load all message bytes immediately since SM3 reads
     * them slightly out of order.
     */
    (void)HOST_c2l(data, W00);
    (void)HOST_c2l(data, W01);
    (void)HOST_c2l(data, W02);
    (void)HOST_c2l(data, W03);
    (void)HOST_c2l(data, W04);
    (void)HOST_c2l(data, W05);
    (void)HOST_c2l(data, W06);
    (void)HOST_c2l(data, W07);
    (void)HOST_c2l(data, W08);
    (void)HOST_c2l(data, W09);
    (void)HOST_c2l(data, W10);
    (void)HOST_c2l(data, W11);
    (void)HOST_c2l(data, W12);
    (void)HOST_c2l(data, W13);
    (void)HOST_c2l(data, W14);
    (void)HOST_c2l(data, W15);

    R1(A, B, C, D, E, F, G, H, 0x79CC4519, W00, W00 ^ W04);
    W00 = EXPAND(W00, W07, W13, W03, W10);
    R1(D, A, B, C, H, E, F, G, 0xF3988A32, W01, W01 ^ W05);
    W01 = EXPAND(W01, W08, W14, W04, W11);
    R1(C, D, A, B, G, H, E, F, 0xE7311465, W02, W02 ^ W06);
    W02 = EXPAND(W02, W09, W15, W05, W12);
    R1(B, C, D, A, F, G, H, E, 0xCE6228CB, W03, W03 ^ W07);
    W03 = EXPAND(W03, W10, W00, W06, W13);
    R1(A, B, C, D, E, F, G, H, 0x9CC45197, W04, W04 ^ W08);
    W04 = EXPAND(W04, W11, W01, W07, W14);
    R1(D, A, B, C, H, E, F, G, 0x3988A32F, W05, W05 ^ W09);
    W05 = EXPAND(W05, W12, W02, W08, W15);
    R1(C, D, A, B, G, H, E, F, 0x7311465E, W06, W06 ^ W10);
    W06 = EXPAND(W06, W13, W03, W09, W00);
    R1(B, C, D, A, F, G, H, E, 0xE6228CBC, W07, W07 ^ W11);
    W07 = EXPAND(W07, W14, W04, W10, W01);
    R1(A, B, C, D, E, F, G, H, 0xCC451979, W08, W08 ^ W12);
    W08 = EXPAND(W08, W15, W05, W11, W02);
    R1(D, A, B, C, H, E, F, G, 0x988A32F3, W09, W09 ^ W13);
    W09 = EXPAND(W09, W00, W06, W12, W03);
    R1(C, D, A, B, G, H, E, F, 0x311465E7, W10, W10 ^ W14);
    W10 = EXPAND(W10, W01, W07, W13, W04);
    R1(B, C, D, A, F, G, H, E, 0x6228CBCE, W11, W11 ^ W15);
    W11 = EXPAND(W11, W02, W08, W14, W05);
    R1(A, B, C, D, E, F, G, H, 0xC451979C, W12, W12 ^ W00);
    W12 = EXPAND(W12, W03, W09, W15, W06);
    R1(D, A, B, C, H, E, F, G, 0x88A32F39, W13, W13 ^ W01);
    W13 = EXPAND(W13, W04, W10, W00, W07);
    R1(C, D, A, B, G, H, E, F, 0x11465E73, W14, W14 ^ W02);
    W14 = EXPAND(W14, W05, W11, W01, W08);
    R1(B, C, D, A, F, G, H, E, 0x228CBCE6, W15, W15 ^ W03);
    W15 = EXPAND(W15, W06, W12, W02, W09);
    R2(A, B, C, D, E, F, G, H, 0x9D8A7A87, W00, W00 ^ W04);
    W00 = EXPAND(W00, W07, W13, W03, W10);
    R2(D, A, B, C, H, E, F, G, 0x3B14F50F, W01, W01 ^ W05);
    W01 = EXPAND(W01, W08, W14, W04, W11);
    R2(C, D, A, B, G, H, E, F, 0x7629EA1E, W02, W02 ^ W06);
    W02 = EXPAND(W02, W09, W15, W05, W12);
    R2(B, C, D, A, F, G, H, E, 0xEC53D43C, W03, W03 ^ W07);
    W03 = EXPAND(W03, W10, W00, W06, W13);
    R2(A, B, C, D, E, F, G, H, 0xD8A7A879, W04, W04 ^ W08);
    W04 = EXPAND(W04, W11, W01, W07, W14);
    R2(D, A, B, C, H, E, F, G, 0xB14F50F3, W05, W05 ^ W09);
    W05 = EXPAND(W05, W12, W02, W08, W15);
    R2(C, D, A, B, G, H, E, F, 0x629EA1E7, W06, W06 ^ W10);
    W06 = EXPAND(W06, W13, W03, W09, W00);
    R2(B, C, D, A, F, G, H, E, 0xC53D43CE, W07, W07 ^ W11);
    W07 = EXPAND(W07, W14, W04, W10, W01);
    R2(A, B, C, D, E, F, G, H, 0x8A7A879D, W08, W08 ^ W12);
    W08 = EXPAND(W08, W15, W05, W11, W02);
    R2(D, A, B, C, H, E, F, G, 0x14F50F3B, W09, W09 ^ W13);
    W09 = EXPAND(W09, W00, W06, W12, W03);
    R2(C, D, A, B, G, H, E, F, 0x29EA1E76, W10, W10 ^ W14);
    W10 = EXPAND(W10, W01, W07, W13, W04);
    R2(B, C, D, A, F, G, H, E, 0x53D43CEC, W11, W11 ^ W15);
    W11 = EXPAND(W11, W02, W08, W14, W05);
    R2(A, B, C, D, E, F, G, H, 0xA7A879D8, W12, W12 ^ W00);
    W12 = EXPAND(W12, W03, W09, W15, W06);
    R2(D, A, B, C, H, E, F, G, 0x4F50F3B1, W13, W13 ^ W01);
    W13 = EXPAND(W13, W04, W10, W00, W07);
    R2(C, D, A, B, G, H, E, F, 0x9EA1E762, W14, W14 ^ W02);
    W14 = EXPAND(W14, W05, W11, W01, W08);
    R2(B, C, D, A, F, G, H, E, 0x3D43CEC5, W15, W15 ^ W03);
    W15 = EXPAND(W15, W06, W12, W02, W09);
    R2(A, B, C, D, E, F, G, H, 0x7A879D8A, W00, W00 ^ W04);
    W00 = EXPAND(W00, W07, W13, W03, W10);
    R2(D, A, B, C, H, E, F, G, 0xF50F3B14, W01, W01 ^ W05);
    W01 = EXPAND(W01, W08, W14, W04, W11);
    R2(C, D, A, B, G, H, E, F, 0xEA1E7629, W02, W02 ^ W06);
    W02 = EXPAND(W02, W09, W15, W05, W12);
    R2(B, C, D, A, F, G, H, E, 0xD43CEC53, W03, W03 ^ W07);
    W03 = EXPAND(W03, W10, W00, W06, W13);
    R2(A, B, C, D, E, F, G, H, 0xA879D8A7, W04, W04 ^ W08);
    W04 = EXPAND(W04, W11, W01, W07, W14);
    R2(D, A, B, C, H, E, F, G, 0x50F3B14F, W05, W05 ^ W09);
    W05 = EXPAND(W05, W12, W02, W08, W15);
    R2(C, D, A, B, G, H, E, F, 0xA1E7629E, W06, W06 ^ W10);
    W06 = EXPAND(W06, W13, W03, W09, W00);
    R2(B, C, D, A, F, G, H, E, 0x43CEC53D, W07, W07 ^ W11);
    W07 = EXPAND(W07, W14, W04, W10, W01);
    R2(A, B, C, D, E, F, G, H, 0x879D8A7A, W08, W08 ^ W12);
    W08 = EXPAND(W08, W15, W05, W11, W02);
    R2(D, A, B, C, H, E, F, G, 0x0F3B14F5, W09, W09 ^ W13);
    W09 = EXPAND(W09, W00, W06, W12, W03);
    R2(C, D, A, B, G, H, E, F, 0x1E7629EA, W10, W10 ^ W14);
    W10 = EXPAND(W10, W01, W07, W13, W04);
    R2(B, C, D, A, F, G, H, E, 0x3CEC53D4, W11, W11 ^ W15);
    W11 = EXPAND(W11, W02, W08, W14, W05);
    R2(A, B, C, D, E, F, G, H, 0x79D8A7A8, W12, W12 ^ W00);
    W12 = EXPAND(W12, W03, W09, W15, W06);
    R2(D, A, B, C, H, E, F, G, 0xF3B14F50, W13, W13 ^ W01);
    W13 = EXPAND(W13, W04, W10, W00, W07);
    R2(C, D, A, B, G, H, E, F, 0xE7629EA1, W14, W14 ^ W02);
    W14 = EXPAND(W14, W05, W11, W01, W08);
    R2(B, C, D, A, F, G, H, E, 0xCEC53D43, W15, W15 ^ W03);
    W15 = EXPAND(W15, W06, W12, W02, W09);
    R2(A, B, C, D, E, F, G, H, 0x9D8A7A87, W00, W00 ^ W04);
    W00 = EXPAND(W00, W07, W13, W03, W10);
    R2(D, A, B, C, H, E, F, G, 0x3B14F50F, W01, W01 ^ W05);
    W01 = EXPAND(W01, W08, W14, W04, W11);
    R2(C, D, A, B, G, H, E, F, 0x7629EA1E, W02, W02 ^ W06);
    W02 = EXPAND(W02, W09, W15, W05, W12);
    R2(B, C, D, A, F, G, H, E, 0xEC53D43C, W03, W03 ^ W07);
    W03 = EXPAND(W03, W10, W00, W06, W13);
    R2(A, B, C, D, E, F, G, H, 0xD8A7A879, W04, W04 ^ W08);
    R2(D, A, B, C, H, E, F, G, 0xB14F50F3, W05, W05 ^ W09);
    R2(C, D, A, B, G, H, E, F, 0x629EA1E7, W06, W06 ^ W10);
    R2(B, C, D, A, F, G, H, E, 0xC53D43CE, W07, W07 ^ W11);
    R2(A, B, C, D, E, F, G, H, 0x8A7A879D, W08, W08 ^ W12);
    R2(D, A, B, C, H, E, F, G, 0x14F50F3B, W09, W09 ^ W13);
    R2(C, D, A, B, G, H, E, F, 0x29EA1E76, W10, W10 ^ W14);
    R2(B, C, D, A, F, G, H, E, 0x53D43CEC, W11, W11 ^ W15);
    R2(A, B, C, D, E, F, G, H, 0xA7A879D8, W12, W12 ^ W00);
    R2(D, A, B, C, H, E, F, G, 0x4F50F3B1, W13, W13 ^ W01);
    R2(C, D, A, B, G, H, E, F, 0x9EA1E762, W14, W14 ^ W02);
    R2(B, C, D, A, F, G, H, E, 0x3D43CEC5, W15, W15 ^ W03);

    ctx->A ^= A;
    ctx->B ^= B;
    ctx->C ^= C;
    ctx->D ^= D;
    ctx->E ^= E;
    ctx->F ^= F;
    ctx->G ^= G;
    ctx->H ^= H;
  }
}

void Hasher::Update(const void *input, size_t input_len) {
  if (input_len == 0) {
    return;
  }
  auto *data = reinterpret_cast<const uint8_t *>(input);
  unsigned char *p = nullptr;
  uint32_t l = 0;
  size_t n = 0;

  l = (Nl + (((uint32_t)input_len) << 3)) & 0xffffffffUL;
  if (l < Nl) /* overflow */
    Nh++;
  Nh += (uint32_t)(input_len >> 29); /* might cause compiler warning on
                                      * 16-bit */
  Nl = l;

  n = num;
  if (n != 0) {
    p = (unsigned char *)data;

    if (input_len >= sm3_cblock || input_len + n >= sm3_cblock) {
      memcpy(p + n, data, sm3_cblock - n);
      sm3_block_data_order(this, p, 1);
      n = sm3_cblock - n;
      data += n;
      input_len -= n;
      num = 0;
      /*
       * We use memset rather than OPENSSL_cleanse() here deliberately.
       * Using OPENSSL_cleanse() here could be a performance issue. It
       * will get properly cleansed on finalisation so this isn't a
       * security problem.
       */
      memset(p, 0, sm3_cblock); /* keep it zeroed */
    } else {
      memcpy(p + n, data, input_len);
      num += (unsigned int)input_len;
      return;
    }
  }

  n = input_len / sm3_cblock;
  if (n > 0) {
    sm3_block_data_order(this, data, n);
    n *= sm3_cblock;
    data += n;
    input_len -= n;
  }

  if (input_len != 0) {
    p = (unsigned char *)data;
    num = (unsigned int)input_len;
    memcpy(p, data, input_len);
  }
}
#define HASH_MAKE_STRING(c, s)                                                                     \
  do {                                                                                             \
    unsigned long ll;                                                                              \
    ll = (c)->A;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->B;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->C;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->D;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->E;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->F;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->G;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
    ll = (c)->H;                                                                                   \
    (void)HOST_l2c(ll, (s));                                                                       \
  } while (0)

void Hasher::Finalize(uint8_t *out, size_t out_len) {
  unsigned char *p = (unsigned char *)data;
  size_t n = num;

  p[n] = 0x80; /* there is always room for one */
  n++;

  if (n > (sm3_cblock - 8)) {
    memset(p + n, 0, sm3_cblock - n);
    n = 0;
    sm3_block_data_order(this, p, 1);
  }
  memset(p + n, 0, sm3_cblock - 8 - n);

  p += sm3_cblock - 8;
#if IS_BIG_ENDIAN
  (void)HOST_l2c(Nh, p);
  (void)HOST_l2c(Nl, p);
#else
  (void)HOST_l2c(Nl, p);
  (void)HOST_l2c(Nh, p);
#endif
  p -= sm3_cblock;
  sm3_block_data_order(this, p, 1);
  num = 0;
  if (out_len < 32) {
    return;
  }
  HASH_MAKE_STRING(this, out);
}
} // namespace bela::hash::sm3