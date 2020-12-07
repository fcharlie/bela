# bela hazel library

add_library(hazel STATIC
  ina/archive.cc
  ina/binexeobj.cc
  ina/docs.cc
  ina/font.cc
  ina/git.cc
  ina/image.cc
  ina/media.cc
  ina/shl.cc
  ina/text.cc
  zip/time.cc
  zip/zip.cc
  elf/elf.cc
  elf/elfdata.cc
  macho/macho.cc
  pe/file.cc
  fs.cc
  hazel.cc
  mime.cc
)

target_link_libraries(hazel
  bela
)

if(BELA_ENABLE_LTO)
  set_property(TARGET hazel PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
