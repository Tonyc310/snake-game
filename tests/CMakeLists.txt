include(FetchContent)
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.7.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(unity)

add_executable(test_game test_game.c)
target_link_libraries(test_game PRIVATE snake_core unity project_warnings)
add_test(NAME game COMMAND test_game)
