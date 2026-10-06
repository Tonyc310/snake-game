FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.7.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(unity)

add_executable(test_game test_game.c)
target_link_libraries(test_game PRIVATE snake_core unity project_warnings)
add_test(NAME game COMMAND test_game)

# The STM32 front end's plain-C parts are tested on the host too.
add_executable(test_keys test_keys.c ${PROJECT_SOURCE_DIR}/stm32/keys.c)
target_include_directories(test_keys PRIVATE ${PROJECT_SOURCE_DIR}/stm32)
target_link_libraries(test_keys PRIVATE unity project_warnings)
add_test(NAME keys COMMAND test_keys)

add_executable(test_format test_format.c ${PROJECT_SOURCE_DIR}/stm32/format.c)
target_include_directories(test_format PRIVATE ${PROJECT_SOURCE_DIR}/stm32)
target_link_libraries(test_format PRIVATE unity project_warnings)
add_test(NAME format COMMAND test_format)

# The renderer draws only through lcd.h, so the test supplies a fake panel in its place.
add_executable(test_render test_render.c
    ${PROJECT_SOURCE_DIR}/stm32/render.c
    ${PROJECT_SOURCE_DIR}/stm32/font.c
    ${PROJECT_SOURCE_DIR}/stm32/format.c)
target_include_directories(test_render PRIVATE ${PROJECT_SOURCE_DIR}/stm32)
target_link_libraries(test_render PRIVATE snake_core unity project_warnings)
add_test(NAME render COMMAND test_render)
