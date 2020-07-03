//
#include <bela/hash.hpp>

namespace bela::hash::sha3 {
void Hasher::Initialize(HashLength hl_) {
  hl = hl_;
  //
}
void Hasher::Update(const void *input, size_t input_len) {
  //
}
void Hasher::Finalize(uint8_t *out, size_t out_len) {
  //
}
} // namespace bela::hash::sha3