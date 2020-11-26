//
#include <hazel/hazel.hpp>

namespace hazel {
using namespace hazel::types;
struct mime_value_t {
  hazel_types_t t;
  const wchar_t *mime;
};
// https://mediatemple.net/community/products/dv/204403964/mime-types
const wchar_t *lookup_mime(hazel_types_t t) {
  constexpr mime_value_t mimes[] = {
      {ascii, L"text/plain"},
      {utf7, L"text/plain;charset=UTF-7"},
      {utf8, L"text/plain;charset=UTF-8"},
      {utf8bom, L"text/plain;charset=UTF-8"},
      {utf16le, L"text/plain;charset=UTF-16LE"},
      {utf16be, L"text/plain;charset=UTF-16BE"},
      {utf32le, L"text/plain;charset=UTF-32LE"},
      {utf32be, L"text/plain;charset=UTF-32BE"},
      // text index end
      // binary
      {bitcode, L"application/octet-stream"},                                   ///< Bitcode file
      {archive, L"application/octet-stream"},                                   ///< ar style archive file
      {elf, L"application/x-executable"},                                       ///< ELF Unknown type
      {elf, L"application/x-executable"},                                       ///< ELF Unknown type
      {elf_relocatable, L"application/x-executable"},                           ///< ELF Relocatable object file
      {elf_executable, L"application/x-executable"},                            ///< ELF Executable image
      {elf_shared_object, L"application/x-executable"},                         ///< ELF dynamically linked shared lib
      {elf_core, L"application/x-executable"},                                  ///< ELF core image
      {macho_object, L"application/x-mach-binary"},                             ///< Mach-O Object file
      {macho_executable, L"application/x-mach-binary"},                         ///< Mach-O Executable
      {macho_fixed_virtual_memory_shared_lib, L"application/x-mach-binary"},    ///< Mach-O Shared Lib, FVM
      {macho_core, L"application/x-mach-binary"},                               ///< Mach-O Core File
      {macho_preload_executable, L"application/x-mach-binary"},                 ///< Mach-O Preloaded Executable
      {macho_dynamically_linked_shared_lib, L"application/x-mach-binary"},      ///< Mach-O dynlinked shared lib
      {macho_dynamic_linker, L"application/x-mach-binary"},                     ///< The Mach-O dynamic linker
      {macho_bundle, L"application/x-mach-binary"},                             ///< Mach-O Bundle file
      {macho_dynamically_linked_shared_lib_stub, L"application/x-mach-binary"}, ///< Mach-O Shared lib stub
      {macho_dsym_companion, L"application/x-mach-binary"},                     ///< Mach-O dSYM companion file
      {macho_kext_bundle, L"application/x-mach-binary"},                        ///< Mach-O kext bundle file
      {macho_universal_binary, L"application/x-mach-binary"},                   ///< Mach-O universal binary
      {coff_cl_gl_object, L"application/vnd.microsoft.coff"},   ///< Microsoft cl.exe's intermediate code file
      {coff_object, L"application/vnd.microsoft.coff"},         ///< COFF object file
      {coff_import_library, L"application/vnd.microsoft.coff"}, ///< COFF import library
      {pecoff_executable, L"application/vnd.microsoft.portable-executable"}, ///< PECOFF executable file
      {windows_resource, L"application/vnd.microsoft.resource"},             ///< Windows compiled resource file (.res)
      {wasm_object, L"application/wasm"},                                    ///< WebAssembly Object file
      {pdb, L"application/octet-stream"},                                    ///< Windows PDB debug info file
      /// archive
      {epub, L"application/epub츪"},
      {zip, L"application/zip"},
      {tar, L"application/x-tar"},
      {rar, L"application/vnd.rar"},
      {gz, L"application/gzip"},
      {bz2, L"application/x-bzip2"},
      {p7z, L"application/x-7z-compressed"},
      {xz, L"application/x-xz"},
      {pdf, L"application/pdf"},
      {swf, L"application/x-shockwave-flash"},
      {rtf, L"application/rtf"},
      {eot, L"application/octet-stream"},
      {ps, L"application/postscript"},
      {sqlite, L"application/vnd.sqlite3"},
      {nes, L"application/x-nintendo-nes-rom"},
      {crx, L"application/x-google-chrome-extension"},
      {deb, L"application/vnd.debian.binary-package"},
      {lz, L"application/x-lzip"},
      {rpm, L"application/x-rpm"},
      {cab, L"application/vnd.ms-cab-compressed"},
      {msi, L"application/x-msi"},
      {dmg, L"application/x-apple-diskimage"},
      {xar, L"application/x-xar"},
      {wim, L"application/x-ms-wim"},
      {z, L"application/x-compress"},
      // image
      {jpg, L"image/jpeg"},
      {jp2, L"image/jp2"},
      {png, L"image/png"},
      {gif, L"image/gif"},
      {webp, L"image/webp"},
      {cr2, L"image/x-canon-cr2"},
      {tif, L"image/tiff"},
      {bmp, L"image/bmp"},
      {jxr, L"image/vnd.ms-photo"},
      {psd, L"image/vnd.adobe.photoshop"},
      {ico, L"image/vnd.microsoft.icon"}, // image/x-icon
      // docs
      {doc, L"application/msword"},
      {docx, L"application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
      {xls, L"application/vnd.ms-excel"},
      {xlsx, L"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
      {ppt, L"application/vnd.ms-powerpoint"},
      {pptx, L"application/vnd.openxmlformats-officedocument.presentationml.presentation"},
      //
      {ofd, L"application/ofd"}, // Open Fixed layout Document
      // font
      {woff, L"application/font-woff"},
      {woff2, L"application/font-woff"},
      {ttf, L"application/font-sfnt"},
      {otf, L"application/font-sfnt"},
      // Media
      {midi, L"audio/x-midi"},
      {mp3, L"audio/mpeg"},
      {m4a, L"audio/m4a"},
      {ogg, L"audio/ogg"},
      {flac, L"audio/flac"},
      {wav, L"audio/wave"},
      {amr, L"audio/3gpp"},
      {aac, L"application/vnd.americandynamics.acc"},
      {mp4, L"video/mp4"},
      {m4v, L"video/x-m4v"},
      {mkv, L"video/x-matroska"},
      {webm, L"video/webm"},
      {mov, L"video/quicktime"},
      {avi, L"video/x-msvideo"},
      {wmv, L"video/x-ms-wmv"},
      {mpeg, L"video/mpeg"},
      {flv, L""},
      // support git
      {gitpack, L"application/x-git-pack"},
      {gitpkindex, L"application/x-git-pack-index"},
      {gitmidx, L"application/x-git-pack-multi-index"},
      {shelllink, L"application/vnd.microsoft.shelllink"}, // Windows shelllink
      {iso, L"application/x-iso9660-image"},
  };
  //
  for (const auto &m : mimes) {
    if (m.t == t) {
      return m.mime;
    }
  }
  return L"application/octet-stream";
}

} // namespace hazel