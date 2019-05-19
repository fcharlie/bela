# bela win libaray

add_library(belawin STATIC
  path.cc
)

target_link_libraries(belawin
  bela
)