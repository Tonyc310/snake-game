*** Variables ***
${ELF}          ${CURDIR}/../../build/stm32/stm32/stm32-snake.elf
${SCRIPT}       ${CURDIR}/snake.resc
${FRAMES}       ${CURDIR}/frames

*** Keywords ***
Start Board
    Execute Command           $bin=@${ELF}
    Execute Script            ${SCRIPT}
    Execute Command           emulation CreateFrameBufferTester "screen" 2
    Execute Command           screen AttachTo sysbus.spi1.lcd

*** Test Cases ***
Clears The Screen To The Background Colour
    Start Board
    Execute Command           screen WaitForFrame @${FRAMES}/background.png
