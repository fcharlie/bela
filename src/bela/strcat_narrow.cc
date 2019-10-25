////
#include <cassert>
#include <cstring>
#include <bela/narrow/strcat.hpp>

namespace bela::narrow {

// ----------------------------------------------------------------------
// StringCat()
//    This merges the given strings or integers, with no delimiter. This
//    is designed to be the fastest possible way to construct a string out
//    of a mix of raw C strings, string_views, strings, and integer values.
// ----------------------------------------------------------------------

// Append is merely a version of memcpy that returns the address of the byte
// after the area just overwritten.
static char *Append(char *out, const AlphaNum &x) {
  // memcpy is allowed to overwrite arbitrary memory, so doing this after the
  // call would force an extra fetch of x.size().
  char *after = out + x.size();
  memcpy(out, x.data(), x.size());
  return after;
}

std::string StringCat(const AlphaNum &a, const AlphaNum &b) {
  std::string result;
  result.resize(a.size() + b.size());
  char *const begin = &*result.begin();
  char *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  assert(out == begin + result.size());
  return result;
}

std::string StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c) {
  std::string result;
  result.resize(a.size() + b.size() + c.size());
  char *const begin = &*result.begin();
  char *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  assert(out == begin + result.size());
  return result;
}

std::string StringCat(const AlphaNum &a, const AlphaNum &b, const AlphaNum &c,
                      const AlphaNum &d) {
  std::string result;
  result.resize(a.size() + b.size() + c.size() + d.size());
  char *const begin = &*result.begin();
  char *out = begin;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  out = Append(out, d);
  assert(out == begin + result.size());
  return result;
}

namespace strings_internal {

// Do not call directly - these are not part of the public API.
std::string CatPieces(std::initializer_list<std::string_view> pieces) {
  std::string result;
  size_t total_size = 0;
  for (const std::string_view piece : pieces) {
    total_size += piece.size();
  }
  result.resize(total_size);

  char *const begin = &*result.begin();
  char *out = begin;
  for (const std::string_view piece : pieces) {
    const size_t this_size = piece.size();
    memcpy(out, piece.data(), this_size);
    out += this_size;
  }
  assert(out == begin + result.size());
  return result;
}

// It's possible to call StrAppend with an std::string_view that is itself a
// fragment of the string we're appending to.  However the results of this are
// random. Therefore, check for this in debug mode.  Use unsigned math so we
// only have to do one comparison. Note, there's an exception case: appending an
// empty string is always allowed.
#define ASSERT_NO_OVERLAP(dest, src)                                           \
  assert(((src).size() == 0) ||                                                \
         (uintptr_t((src).data() - (dest).data()) > uintptr_t((dest).size())))

void AppendPieces(std::string *dest,
                  std::initializer_list<std::string_view> pieces) {
  size_t old_size = dest->size();
  size_t total_size = old_size;
  for (const std::string_view piece : pieces) {
    total_size += piece.size();
  }
  dest->resize(total_size);

  char *const begin = &*dest->begin();
  char *out = begin + old_size;
  for (const std::string_view piece : pieces) {
    const size_t this_size = piece.size();
    memcpy(out, piece.data(), this_size);
    out += this_size;
  }
  assert(out == begin + dest->size());
}

} // namespace strings_internal

void StrAppend(std::string *dest, const AlphaNum &a) {
  ASSERT_NO_OVERLAP(*dest, a);
  dest->append(a.data(), a.size());
}

void StrAppend(std::string *dest, const AlphaNum &a, const AlphaNum &b) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  std::string::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size());
  char *const begin = &*dest->begin();
  char *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  assert(out == begin + dest->size());
}

void StrAppend(std::string *dest, const AlphaNum &a, const AlphaNum &b,
               const AlphaNum &c) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  ASSERT_NO_OVERLAP(*dest, c);
  std::string::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size());
  char *const begin = &*dest->begin();
  char *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  assert(out == begin + dest->size());
}

void StrAppend(std::string *dest, const AlphaNum &a, const AlphaNum &b,
               const AlphaNum &c, const AlphaNum &d) {
  ASSERT_NO_OVERLAP(*dest, a);
  ASSERT_NO_OVERLAP(*dest, b);
  ASSERT_NO_OVERLAP(*dest, c);
  ASSERT_NO_OVERLAP(*dest, d);
  std::string::size_type old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size() + d.size());
  char *const begin = &*dest->begin();
  char *out = begin + old_size;
  out = Append(out, a);
  out = Append(out, b);
  out = Append(out, c);
  out = Append(out, d);
  assert(out == begin + dest->size());
}
} // namespace bela::narrow
