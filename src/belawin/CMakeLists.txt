# bela win libaray

add_library(belawin STATIC
  path.cc
  realpath.cc
  appexeclink.cc
)

target_link_libraries(belawin
  bela
)