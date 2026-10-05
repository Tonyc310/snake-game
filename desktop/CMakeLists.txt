add_executable(desktop-snake main.c render.c sound.c)
# A windowed program on Windows, so no console opens behind the game; SDL supplies WinMain.
set_target_properties(desktop-snake PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(desktop-snake PRIVATE snake_core SDL3::SDL3 project_warnings)
