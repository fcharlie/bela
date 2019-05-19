////////
#ifndef BELA_REPARSEPOINT_HPP
#define BELA_REPARSEPOINT_HPP
#include "base.hpp"
#include <optional>
#include <vector>
#include <variant>

namespace bela {

enum ReparsePointTagIndex : unsigned long {
  RpMOUNT_POINT = (0xA0000003L), // winnt
  RpHSM = (0xC0000004L),         // winnt
  RpDRIVE_EXTENDER = (0x80000005L),
  RpHSM2 = (0x80000006L), // winnt
  RpSIS = (0x80000007L),  // winnt
  RpWIM = (0x80000008L),  // winnt
  RpCSV = (0x80000009L),  // winnt
  RpDFS = (0x8000000AL),  // winnt
  RpFILTER_MANAGER = (0x8000000BL),
  RpSYMLINK = (0xA000000CL), // winnt
  RpIIS_CACHE = (0xA0000010L),
  RpDFSR = (0x80000012L),  // winnt
  RpDEDUP = (0x80000013L), // winnt
  RpAPPXSTRM = (0xC0000014L),
  RpNFS = (0x80000014L),              // winnt
  RpFILE_PLACEHOLDER = (0x80000015L), // winnt
  RpDFM = (0x80000016L),
  RpWOF = (0x80000017L),            // winnt
  RpWCI = (0x80000018L),            // winnt
  RpWCI_1 = (0x90001018L),          // winnt
  RpGLOBAL_REPARSE = (0xA0000019L), // winnt
  RpCLOUD = (0x9000001AL),          // winnt
  RpCLOUD_1 = (0x9000101AL),        // winnt
  RpCLOUD_2 = (0x9000201AL),        // winnt
  RpCLOUD_3 = (0x9000301AL),        // winnt
  RpCLOUD_4 = (0x9000401AL),        // winnt
  RpCLOUD_5 = (0x9000501AL),        // winnt
  RpCLOUD_6 = (0x9000601AL),        // winnt
  RpCLOUD_7 = (0x9000701AL),        // winnt
  RpCLOUD_8 = (0x9000801AL),        // winnt
  RpCLOUD_9 = (0x9000901AL),        // winnt
  RpCLOUD_A = (0x9000A01AL),        // winnt
  RpCLOUD_B = (0x9000B01AL),        // winnt
  RpCLOUD_C = (0x9000C01AL),        // winnt
  RpCLOUD_D = (0x9000D01AL),        // winnt
  RpCLOUD_E = (0x9000E01AL),        // winnt
  RpCLOUD_F = (0x9000F01AL),        // winnt
  RpCLOUD_MASK = (0x0000F000L),     // winnt
  RpAPPEXECLINK = (0x8000001BL),    // winnt
  RpPROJFS = (0x9000001CL),         // winnt
  RpLX_SYMLINK = (0xA000001DL),
  RpSTORAGE_SYNC = (0x8000001EL),     // winnt
  RpWCI_TOMBSTONE = (0xA000001FL),    // winnt
  RpUNHANDLED = (0x80000020L),        // winnt
  RpONEDRIVE = (0x80000021L),         // winnt
  RpPROJFS_TOMBSTONE = (0xA0000022L), // winnt
  RpAF_UNIX = (0x80000023L),          // winnt
  RpLX_FIFO = (0x80000024L),
  RpLX_CHR = (0x80000025L),
  RpLX_BLK = (0x80000026L)
};

// value
struct FileAttributePair {
  FileAttributePair() = default;
  FileAttributePair(std::wstring_view n, std::wstring_view v)
      : name(n), value(v) {}
  std::wstring name;
  std::wstring value;
};

class ReparsePoint {
public:
  ReparsePoint() = default;
  ReparsePoint(const ReparsePoint &) = delete;
  ReparsePoint &operator=(const ReparsePoint &) = delete;
  bool Analyze(std::wstring_view file, bela::error_code &ec);
  const std::vector<FileAttributePair> &Attributes() const { return values; }
  unsigned long ReparseTagValue() const { return tagvalue; }

private:
  unsigned long tagvalue{0};
  std::vector<FileAttributePair> values;
};

} // namespace bela

#endif