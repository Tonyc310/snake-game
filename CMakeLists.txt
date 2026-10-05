cmake_minimum_required(VERSION 3.20)
project(snake_game LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(SNAKE_FETCH_SDL3 "Download and build SDL3 when it isn't installed" OFF)

include(FetchContent)

add_library(project_warnings INTERFACE)
if(MSVC)
    target_compile_options(project_warnings INTERFACE /W4 /WX)
else()
    target_compile_options(project_warnings INTERFACE -Wall -Wextra -Werror -pedantic)
endif()

add_subdirectory(core)

# Each front end builds only where its library exists: Windows has no curses, for one.
find_package(Curses QUIET)
if(CURSES_FOUND)
    add_subdirectory(terminal)
else()
    message(STATUS "curses not found: skipping the terminal front end")
endif()

find_package(SDL3 CONFIG QUIET)
if(NOT SDL3_FOUND AND SNAKE_FETCH_SDL3)
    # Static, so the program runs without an SDL3 library installed next to it.
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.18
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(SDL3)
endif()
if(TARGET SDL3::SDL3)
    add_subdirectory(desktop)
else()
    message(STATUS "SDL3 not found: skipping the desktop front end (SNAKE_FETCH_SDL3=ON downloads it)")
endif()

enable_testing()
add_subdirectory(tests)
