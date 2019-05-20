# bela win libaray

add_library(belawin STATIC
  path.cc
  realpath.cc
  reparsepoint.cc
  appexeclink.cc
)

target_link_libraries(belawin
  bela
)
