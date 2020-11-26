# bela hazel library

add_library(hazel STATIC
  mime.cc
)

if(BELA_ENABLE_LTO)
  set_property(TARGET hazel PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
