find_package(Curses REQUIRED)

add_executable(terminal-snake main.c render.c)
# clock_gettime() and CLOCK_MONOTONIC are POSIX, not part of plain C11.
target_compile_definitions(terminal-snake PRIVATE _POSIX_C_SOURCE=200809L)
target_include_directories(terminal-snake PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(terminal-snake PRIVATE snake_core ${CURSES_LIBRARIES} project_warnings)
