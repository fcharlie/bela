// bela use C++17 and wchar_t rewrite cpptoml.h
// thanks https://github.com/skystrife/cpptoml/
// NO RTTI and use phmap support heterogeneous lookup
#ifndef BELA_TOML_HPP
#define BELA_TOML_HPP
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include "base.hpp"
#include "phmap.hpp"

namespace bela::toml {
[[maybe_unused]] constexpr size_t TOML_MAXIMUM_LENTH = 1024 * 1024 * 256;
class base;
using string_to_base_map =
    bela::flat_hash_map<std::wstring, std::shared_ptr<base>>;

enum class base_type {
  NONE, // --
  STRING,
  LOCAL_TIME,
  LOCAL_DATE,
  LOCAL_DATETIME,
  OFFSET_DATETIME,
  INT,
  FLOAT,
  TABLE,
  ARRAY,
  TABLE_ARRAY
};

template <typename T> struct base_type_traits;
template <> struct base_type_traits<std::wstring> {
  static constexpr const base_type type = base_type::STRING;
};

class table;

class table {
public:
  table() = default;
  //
  static std::optional<table> parse_buffer(std::string_view sv,
                                           bela::error_code &ec);

private:
};
class array {
public:
private:
};
std::optional<table> parse_file(std::wstring_view path, bela::error_code &ec);
std::optional<table> parse_buffer(std::wstring_view sv, bela::error_code &ec);
std::optional<table> parse_buffer(std::u16string_view sv, bela::error_code &ec);
} // namespace bela::toml

#endif