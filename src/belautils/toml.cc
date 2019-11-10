///
#include <bela/toml.hpp>
#include <bela/codecvt.hpp>
#include <bela/mapview.hpp>

namespace bela::toml {

std::optional<table> parse_file(std::wstring_view path, bela::error_code &ec) {
  constexpr std::string_view BOM = "\xEF\xBB\xBF";
  bela::MapView mv;
  if (!mv.MappingView(path, ec, 0, TOML_MAXIMUM_LENTH)) {
    return std::nullopt;
  }
  auto sv = mv.subview();
  if (sv.IndexsWith(0, BOM)) {
    return table::parse_buffer(sv.submv(3).sv(), ec);
  }
  return table::parse_buffer(sv.sv(), ec);
}
std::optional<table> parse_buffer(std::wstring_view sv, bela::error_code &ec) {
  return table::parse_buffer(bela::ToNarrow(sv), ec);
}
std::optional<table> parse_buffer(std::u16string_view sv,
                                  bela::error_code &ec) {
  return table::parse_buffer(bela::ToNarrow(sv), ec);
}

std::optional<table> table::parse_buffer(std::string_view sv,
                                         bela::error_code &ec) {
  if (sv.empty()) {
    ec = bela::make_error_code(1, L"buffer empty");
    return std::nullopt;
  }
  return std::nullopt;
}

} // namespace bela::toml