///
#ifndef BELA_FS_HPP
#define BELA_FS_HPP
#include "base.hpp"

namespace bela::fs {
// Remove remove file force
bool Remove(std::wstring_view path, bela::error_code &ec);
} // namespace bela::fs

#endif