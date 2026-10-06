*** Variables ***
${ELF}          ${CURDIR}/../../build/stm32/stm32/stm32-snake.elf
${SCRIPT}       ${CURDIR}/snake.resc
${FRAMES}       ${CURDIR}/frames
${UART}         sysbus.usart2
${READY}        snake: arrow keys or WASD to steer, P to pause, R to restart
# Screen regions as x y width height. Food lands wherever the first key's timing seeds it, so
# checks after that look only at regions it can't be in.
${WHOLE}        0 0 320 240
${BANNER}       32 112 256 64
${COLUMN}       160 48 16 48

*** Keywords ***
Start Board
    Execute Command           $bin=@${ELF}
    Execute Script            ${SCRIPT}
    Create Terminal Tester    ${UART}    defaultPauseEmulation=true
    Wait For Line On Uart     ${READY}

Screen Shows
    [Arguments]    ${frame}    ${region}=${WHOLE}
    # A tester queues every frame since it was attached and checks one queued frame per new one,
    # so each check gets a fresh tester that sees only what's drawn from now on.
    ${tester}=    Evaluate    "${frame}_${region}".replace(" ", "_").replace("-", "_")
    Execute Command           emulation CreateFrameBufferTester "${tester}" 2
    Execute Command           ${tester} AttachTo sysbus.spi1.lcd
    Execute Command           ${tester} WaitForFrameROI @${FRAMES}/${frame}.png ${region}
    Execute Command           ${tester} DetachFrom sysbus.spi1.lcd

*** Test Cases ***
Draws The Opening Board
    Start Board
    Screen Shows              opening

Steers With The Keys
    Start Board
    Write To Uart             w
    Wait For Line On Uart     game over, score
    Screen Shows              steered-up    ${COLUMN}
    Screen Shows              steered-up    ${BANNER}

Pauses And Restarts
    Start Board
    Write To Uart             w
    Write To Uart             p
    Screen Shows              paused    ${BANNER}
    # Heading up, an unpaused snake would hit the wall within a second.
    Should Not Be On Uart     game over    timeout=3
    Write To Uart             p
    Wait For Line On Uart     game over, score
    Write To Uart             r
    Write To Uart             w
    Wait For Line On Uart     game over, score
