*** Variables ***
${ELF}          ${CURDIR}/../../build/stm32/stm32/stm32-snake.elf
${SCRIPT}       ${CURDIR}/snake.resc
${FRAMES}       ${CURDIR}/frames
${UART}         sysbus.usart2

*** Keywords ***
Start Board
    Execute Command           $bin=@${ELF}
    Execute Script            ${SCRIPT}
    Execute Command           emulation CreateFrameBufferTester "screen" 2
    Execute Command           screen AttachTo sysbus.spi1.lcd
    Create Terminal Tester    ${UART}    defaultPauseEmulation=true

*** Test Cases ***
Draws The Opening Board
    Start Board
    Execute Command           screen WaitForFrame @${FRAMES}/opening.png

Steers With The Keys
    Start Board
    Wait For Line On Uart     snake: steer with the arrow keys or WASD
    Write To Uart             w
    Wait For Line On Uart     game over, score
    # Only the snake's column is compared, since food lands wherever the key's timing seeds it.
    Execute Command           screen WaitForFrameROI @${FRAMES}/steered-up.png 160 48 16 48
