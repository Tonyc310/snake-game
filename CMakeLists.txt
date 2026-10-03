cmake_minimum_required(VERSION 3.20)
project(snake_game LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Werror -pedantic)

add_subdirectory(core)
add_subdirectory(terminal)

enable_testing()
add_subdirectory(tests)
