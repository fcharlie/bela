///
#ifndef BELA_HASH_INTERNAL_HPP
#define BELA_HASH_INTERNAL_HPP
#include <bela/bits.hpp>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define I64(x) x##ui64
#else
#define I64(x) x##ULL
#endif

#endif