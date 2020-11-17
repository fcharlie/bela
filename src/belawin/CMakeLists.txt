# bela win libaray

add_library(belawin STATIC
  env.cc
  io.cc
  fs.cc
  path.cc
  peimage.cc
  pe/file.cc
  pe/resource.cc
  pe/section.cc
  pe/string.cc
  pe/symbol.cc
  process.cc
  realpath.cc
  reparsepoint.cc
  simulator.cc
  version.cc
)

target_link_libraries(belawin
  bela
  version
)


if(BELA_ENABLE_LTO)
  set_property(TARGET belawin PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
