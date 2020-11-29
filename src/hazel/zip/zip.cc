///
#include <hazel/zip.hpp>
#include <hazel/hazel.hpp>

namespace hazel::zip {
constexpr auto SPD = 3;
bool IsSuperficialPath(std::string_view sv) {
  //
  return true;
}

bool DissectZIP(const File &r, ZipDetails &zd, bela::error_code &ec) {
  bela::Buffer buffer(4096);
  if (!r.ReadAt(buffer, sizeof(zip_file_header_t), 4, ec)) {
    return false;
  }
  bela::flat_hash_set<std::string> superficialfiles;
  return true;
}

} // namespace hazel::zip