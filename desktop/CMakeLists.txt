add_executable(desktop-snake main.c)
target_link_libraries(desktop-snake PRIVATE SDL3::SDL3 project_warnings)
