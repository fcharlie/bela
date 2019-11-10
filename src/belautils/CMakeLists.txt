# bela win libaray

add_library(belautils STATIC
 toml.cc
)

target_link_libraries(belautils
  belawin
)
