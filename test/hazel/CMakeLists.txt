add_executable(zipview
  zipview.cc
)

target_link_libraries(zipview
  belatime
  belawin
  hazel
)