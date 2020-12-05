#

# bela win libaray

add_library(belatime STATIC
 clock.cc
 duration.cc
 int128.cc
 time.cc
 unscaledcycleclock.cc
)

target_link_libraries(belatime
  bela
)

if(BELA_ENABLE_LTO)
  set_property(TARGET belatime PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
