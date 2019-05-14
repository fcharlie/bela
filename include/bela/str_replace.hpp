//
#ifndef BELA_STR_REPLACE_HPP
#define BELA_STR_REPLACE_HPP
#pragma once
#include <string>
#include <utility>
#include <vector>
#include <string_view>

namespace bela {
[[nodiscard]] std::wstring StrReplaceAll(
    std::wstring_view s,
    std::initializer_list<std::pair<std::wstring_view, std::wstring_view>>
        replacements);
template <typename StrToStrMapping>
[[nodiscard]] std::wstring StrReplaceAll(std::wstring_view s,
                           const StrToStrMapping &replacements);
int StrReplaceAll(
    std::initializer_list<std::pair<std::wstring_view, std::wstring_view>>
        replacements,
    std::wstring *target);
} // namespace bela

#endif
