enable_language(ASM)

FetchContent_Declare(cmsis_core
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_6.git
    GIT_TAG        v6.3.0
    GIT_SHALLOW    TRUE)
FetchContent_Declare(cmsis_device_f4
    GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f4.git
    GIT_TAG        v2.6.11
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(cmsis_core cmsis_device_f4)

# ST's startup code and SystemInit, kept apart so our warning flags don't apply to vendor code.
add_library(cmsis_device OBJECT
    ${cmsis_device_f4_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f446xx.s
    ${cmsis_device_f4_SOURCE_DIR}/Source/Templates/system_stm32f4xx.c)
target_include_directories(cmsis_device SYSTEM PUBLIC
    ${cmsis_core_SOURCE_DIR}/CMSIS/Core/Include
    ${cmsis_device_f4_SOURCE_DIR}/Include)
target_compile_definitions(cmsis_device PUBLIC STM32F446xx)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/stm32f446re.ld)

add_executable(stm32-snake main.c board.c lcd.c render.c)
set_target_properties(stm32-snake PROPERTIES
    SUFFIX .elf
    LINK_DEPENDS ${LINKER_SCRIPT})
target_link_libraries(stm32-snake PRIVATE snake_core cmsis_device project_warnings)
target_link_options(stm32-snake PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=$<TARGET_FILE_DIR:stm32-snake>/stm32-snake.map
    -Wl,--print-memory-usage)

find_program(RENODE_TEST renode-test)
if(RENODE_TEST)
    set(RENODE_RESULTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/renode)
    file(MAKE_DIRECTORY ${RENODE_RESULTS_DIR})
    # renode-test always writes its results to the working directory, so keep them in the build tree.
    add_test(NAME renode_snake
        COMMAND ${RENODE_TEST} --variable ELF:$<TARGET_FILE:stm32-snake>
                ${CMAKE_CURRENT_SOURCE_DIR}/renode/snake.robot
        WORKING_DIRECTORY ${RENODE_RESULTS_DIR})
endif()
