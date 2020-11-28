# bela hazel library

add_library(hazel STATIC
  fs.cc
  hazel.cc
  mime.cc
  zip.cc
)

target_link_libraries(hazel
  bela
)

if(BELA_ENABLE_LTO)
  set_property(TARGET hazel PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
