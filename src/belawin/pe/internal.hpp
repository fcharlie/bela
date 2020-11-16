///
#ifndef BELA_PE_INTERNAL_HPP
#define BELA_PE_INTERNAL_HPP
#include <bela/pe.hpp>

namespace bela::pe {
bool readStringTable(FileHeader *fh, FILE *fd, StringTable &table, bela::error_code &ec);
bool readCOFFSymbols(FileHeader *fh, FILE *fd, std::vector<COFFSymbol> &symbols, bela::error_code &ec);
} // namespace bela::pe

#endif