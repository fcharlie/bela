//
#include <bela/pe.hpp>

// https://github.com/llvm/llvm-project/blob/master/llvm/lib/Demangle/ItaniumDemangle.cpp
// https://github.com/llvm/llvm-project/blob/master/llvm/lib/Demangle/MicrosoftDemangle.cpp
// https://github.com/llvm/llvm-project/blob/master/llvm/lib/Demangle/MicrosoftDemangleNodes.cpp
namespace bela::pe {

static bool isItaniumEncoding(const std::string_view MangledName) {
  size_t Pos = MangledName.find_first_not_of('_');
  // A valid Itanium encoding requires 1-4 leading underscores, followed by 'Z'.
  return Pos > 0 && Pos <= 4 && MangledName[Pos] == 'Z';
}

bool demangleItaniumABI(std::string_view fn, std::string &ofn) {
  //
  return false;
}

bool demangleMsvcABI(std::string_view fn, std::string &ofn) {
  //
  return false;
}

std::optional<std::string> Demangle(std::string_view fn) {
  std::string ofn;
  bool isdemangled = false;
  if (isItaniumEncoding(fn)) {
    isdemangled = demangleItaniumABI(fn, ofn);
  } else {
    isdemangled = demangleMsvcABI(fn, ofn);
  }
  if (isdemangled) {
    return std::make_optional(std::move(ofn));
  }
  return std::nullopt;
}

} // namespace bela::pe