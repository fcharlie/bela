//
#ifndef HAZEL_HAZEL_HPP
#define HAZEL_HAZEL_HPP
#include <variant>
#include <bela/base.hpp>
#include <bela/phmap.hpp>
#include <bela/buffer.hpp>
#include <bela/time.hpp>
#include "types.hpp"
#include "io.hpp"

namespace hazel {

/*

#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

// the variant to visit
using var_t = std::variant<int, long, double, std::string>;

// helper constant for the visitor #3
template<class> inline constexpr bool always_false_v = false;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main() {
    std::vector<var_t> vec = {10, 15l, 1.5, "hello"};
    for(auto& v: vec) {

        // 1. void visitor, only called for side-effects (here, for I/O)
        std::visit([](auto&& arg){std::cout << arg;}, v);

        // 2. value-returning visitor, demonstrates the idiom of returning another variant
        var_t w = std::visit([](auto&& arg) -> var_t {return arg + arg;}, v);

        // 3. type-matching visitor: a lambda that handles each type differently
        std::cout << ". After doubling, variant holds ";
        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>)
                std::cout << "int with value " << arg << '\n';
            else if constexpr (std::is_same_v<T, long>)
                std::cout << "long with value " << arg << '\n';
            else if constexpr (std::is_same_v<T, double>)
                std::cout << "double with value " << arg << '\n';
            else if constexpr (std::is_same_v<T, std::string>)
                std::cout << "std::string with value " << std::quoted(arg) << '\n';
            else
                static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }, w);
    }

    for (auto& v: vec) {
        // 4. another type-matching visitor: a class with 3 overloaded operator()'s
        std::visit(overloaded {
            [](auto arg) { std::cout << arg << ' '; },
            [](double arg) { std::cout << std::fixed << arg << ' '; },
            [](const std::string& arg) { std::cout << std::quoted(arg) << ' '; },
        }, v);
    }
}
*/

using hazel_value_t = std::variant<std::string, std::wstring, std::vector<std::string>, std::vector<std::wstring>,
                                   int32_t, int64_t, uint32_t, uint64_t, bela::Time>;
class hazel_result {
public:
  hazel_result() = default;
  hazel_result(const hazel_result &) = delete;
  hazel_result &operator=(const hazel_result &) = delete;
  const auto &values() const { return values_; }
  hazel_result &assgin(types::hazel_types_t ty, std::wstring_view desc) {
    t = ty;
    values_.emplace(L"description", std::wstring(desc));
    return *this;
  }
  template <typename T> hazel_result &append(std::wstring_view key, T &&value) {
    values_.emplace(key, std::move(value));
    return *this;
  }
  auto type() const { return t; }

private:
  bela::flat_hash_map<std::wstring, hazel_value_t> values_;
  types::hazel_types_t t{types::none};
};
// file attribute table
struct FileAttributeTable {
  bela::flat_hash_map<std::wstring, std::wstring> attributes;
  bela::flat_hash_map<std::wstring, std::vector<std::wstring>> multi_attributes;
  int64_t size{0};
  types::hazel_types_t type{types::none};
  FileAttributeTable &assign(std::wstring_view desc, types::hazel_types_t t = types::none) {
    attributes.emplace(L"Description", desc);
    type = t;
    return *this;
  }
  FileAttributeTable &append(const std::wstring_view &name, const std::wstring_view &value) {
    attributes.emplace(name, value);
    return *this;
  }
  FileAttributeTable &append(std::wstring &&name, std::wstring &&value) {
    attributes.emplace(std::move(name), std::move(value));
    return *this;
  }
  FileAttributeTable &append(std::wstring &&name, std::vector<std::wstring> &&value) {
    multi_attributes.emplace(std::move(name), std::move(value));
    return *this;
  }

  bool LooksLikeELF() const {
    return type == types::elf || type == types::elf_executable || type == types::elf_relocatable ||
           type == types::elf_shared_object;
  }
  bool LooksLikeMachO() const {
    return type == types::macho_bundle || type == types::macho_core || type == types::macho_dsym_companion ||
           type == types::macho_dynamic_linker || type == types::macho_dynamically_linked_shared_lib ||
           type == types::macho_dynamically_linked_shared_lib_stub || type == types::macho_executable ||
           type == types::macho_fixed_virtual_memory_shared_lib || type == types::macho_kext_bundle ||
           type == types::macho_object || type == types::macho_preload_executable ||
           type == types::macho_universal_binary;
  }
  bool LooksLikePE() const { return type == types::pecoff_executable; }
  bool LooksLikeZIP() const {
    return type == types::zip || type == types::docx || type == types::xlsx || type == types::pptx ||
           type == types::ofd;
  }
};

bool LookupFile(hazel::io::File &fd, FileAttributeTable &fat, bela::error_code &ec);
} // namespace hazel

#endif