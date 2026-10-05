#include "board.h"

#include "stm32f4xx.h"

#include <stdatomic.h>

#define AF_SPI1 5u
#define TICK_HZ 1000u

static _Atomic uint32_t uptime_ms; /* written only by SysTick_Handler */

void board_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    /* Read back so the clocks are running before their registers are touched (STM32F4 errata). */
    (void)RCC->APB2ENR;

    /* SPI1: PA5 (SCK) and PA7 (MOSI). The panel is write-only here, so MISO stays unrouted. */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL7)) |
                    (AF_SPI1 << GPIO_AFRL_AFSEL5_Pos) | (AF_SPI1 << GPIO_AFRL_AFSEL7_Pos);
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE7)) | GPIO_MODER_MODE5_1 |
                   GPIO_MODER_MODE7_1;
    /* Low speed is rated to 4 MHz into 50 pF; the SPI clock runs at 8. */
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_0 | GPIO_OSPEEDR_OSPEED7_0;

    /* Chip select and reset idle high, so set them before the pins become outputs. */
    GPIOB->BSRR = GPIO_BSRR_BS6;
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6) | GPIO_MODER_MODE6_0;
    GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODE7) | GPIO_MODER_MODE7_0;
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE9) | GPIO_MODER_MODE9_0;

    /* SystemInit leaves the chip on its 16 MHz internal clock, so this is a 1 ms tick. */
    SysTick_Config(SystemCoreClock / TICK_HZ);
}

uint32_t board_uptime_ms(void)
{
    return atomic_load_explicit(&uptime_ms, memory_order_relaxed);
}

void board_delay_ms(uint32_t ms)
{
    const uint32_t start = board_uptime_ms();

    /* Unsigned subtraction stays correct across the counter wrapping. */
    while ((board_uptime_ms() - start) < ms) {
        __WFI();
    }
}

/* BSRR changes only the named pin in a single write, so it can't clobber the rest of the port. */
void board_lcd_select(bool selected)
{
    GPIOB->BSRR = selected ? GPIO_BSRR_BR6 : GPIO_BSRR_BS6;
}

void board_lcd_data_mode(bool data)
{
    GPIOC->BSRR = data ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7;
}

void board_lcd_reset(bool asserted)
{
    GPIOA->BSRR = asserted ? GPIO_BSRR_BR9 : GPIO_BSRR_BS9;
}

void SysTick_Handler(void)
{
    atomic_fetch_add_explicit(&uptime_ms, 1u, memory_order_relaxed);
}
