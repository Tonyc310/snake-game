FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.7.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(unity)

add_executable(test_game test_game.c)
target_link_libraries(test_game PRIVATE snake_core unity project_warnings)
add_test(NAME game COMMAND test_game)

# The STM32 front end's key decoder is plain C, so it's tested on the host too.
add_executable(test_keys test_keys.c ${PROJECT_SOURCE_DIR}/stm32/keys.c)
target_include_directories(test_keys PRIVATE ${PROJECT_SOURCE_DIR}/stm32)
target_link_libraries(test_keys PRIVATE unity project_warnings)
add_test(NAME keys COMMAND test_keys)
