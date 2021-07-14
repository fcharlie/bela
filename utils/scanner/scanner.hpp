#ifndef GIT_SCANNER_HPP
#define GIT_SCANNER_HPP
#include <functional>
#include <bela/base.hpp>
#include <bela/endian.hpp>

namespace git {
using Filter = std::function<bool(std::wstring_view oid, std::int64_t size)>;

class Scanner {
public:
  Scanner(std::int64_t ls) : limitSize(ls) {}
  bool Execute(const std::wstring_view gitdir, const Filter &filter, bela::error_code &ec);
  auto DiskSize() const { return diskSize; }

private:
  const std::int64_t limitSize;
  std::int64_t diskSize{0};
};

} // namespace git

#endif