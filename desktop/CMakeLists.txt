add_executable(desktop-snake main.c render.c sound.c)
target_link_libraries(desktop-snake PRIVATE snake_core SDL3::SDL3 project_warnings)
