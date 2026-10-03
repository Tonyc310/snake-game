add_library(snake_core STATIC game.c)
target_include_directories(snake_core PUBLIC .)
target_link_libraries(snake_core PRIVATE project_warnings)
