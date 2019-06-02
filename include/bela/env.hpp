/// Env helper
#ifndef BELA_ENV_HPP
#define BELA_ENV_HPP
#include <string>
#include <string_view>

namespace bela {
std::wstring ExpandEnv(std::wstring_view sv);
std::wstring PathUnExpand(std::wstring_view sv);
} // namespace bela

#endif