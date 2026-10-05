#include "lcd.h"

#include "board.h"
#include "stm32f4xx.h"

#include <stddef.h>

#define CMD_SLEEP_OUT 0x11u
#define CMD_DISPLAY_ON 0x29u
#define CMD_COLUMN_ADDRESS_SET 0x2Au
#define CMD_PAGE_ADDRESS_SET 0x2Bu
#define CMD_MEMORY_WRITE 0x2Cu
#define CMD_MEMORY_ACCESS_CONTROL 0x36u
#define CMD_PIXEL_FORMAT_SET 0x3Au

/* Row/column exchange turns the 240x320 panel to landscape; BGR matches its subpixel order. */
#define MEMORY_ACCESS_LANDSCAPE 0x28u
#define PIXEL_FORMAT_16_BITS 0x55u

#define RESET_PULSE_MS 1u      /* the datasheet asks for at least 10 us */
#define RESET_WAIT_MS 120u     /* a reset can take this long if the panel was awake */
#define SLEEP_OUT_WAIT_MS 120u /* the panel's supplies settle before it takes more commands */

static void spi_init(void)
{
    /* Master at PCLK/2 = 8 MHz (the ILI9341 takes writes up to 10 MHz), mode 0, 8-bit frames.
     * Chip select is a GPIO, so the hardware NSS input is held inactive in software. */
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void spi_send(uint8_t byte)
{
    while ((SPI1->SR & SPI_SR_TXE) == 0u) {
    }
    SPI1->DR = byte;
}

/* TXE only means the byte reached the shift register; BSY clears once it has left the pin. */
static void spi_wait_idle(void)
{
    while ((SPI1->SR & SPI_SR_TXE) == 0u) {
    }
    while ((SPI1->SR & SPI_SR_BSY) != 0u) {
    }
}

/* Selects the panel and sends a command byte; what follows is data until end_transfer(). */
static void begin_command(uint8_t command)
{
    board_lcd_select(true);
    board_lcd_data_mode(false);
    spi_send(command);
    spi_wait_idle(); /* the data/command line is sampled with the last bit */
    board_lcd_data_mode(true);
}

static void end_transfer(void)
{
    spi_wait_idle();
    board_lcd_select(false);
}

static void send_command(uint8_t command, const uint8_t *params, size_t count)
{
    begin_command(command);
    for (size_t i = 0u; i < count; i++) {
        spi_send(params[i]);
    }
    end_transfer();
}

static void send_range(uint8_t command, uint16_t first, uint16_t last)
{
    const uint8_t params[] = {(uint8_t)(first >> 8), (uint8_t)first, (uint8_t)(last >> 8),
                              (uint8_t)last};

    send_command(command, params, sizeof params);
}

void lcd_init(void)
{
    static const uint8_t memory_access = MEMORY_ACCESS_LANDSCAPE;
    static const uint8_t pixel_format = PIXEL_FORMAT_16_BITS;

    spi_init();

    board_lcd_reset(true);
    board_delay_ms(RESET_PULSE_MS);
    board_lcd_reset(false);
    board_delay_ms(RESET_WAIT_MS);

    send_command(CMD_SLEEP_OUT, NULL, 0u);
    board_delay_ms(SLEEP_OUT_WAIT_MS);
    send_command(CMD_PIXEL_FORMAT_SET, &pixel_format, 1u);
    send_command(CMD_MEMORY_ACCESS_CONTROL, &memory_access, 1u);
    send_command(CMD_DISPLAY_ON, NULL, 0u);
}

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT) || (width == 0u) || (height == 0u)) {
        return;
    }
    if (width > (LCD_WIDTH - x)) {
        width = (uint16_t)(LCD_WIDTH - x);
    }
    if (height > (LCD_HEIGHT - y)) {
        height = (uint16_t)(LCD_HEIGHT - y);
    }

    send_range(CMD_COLUMN_ADDRESS_SET, x, (uint16_t)(x + width - 1u));
    send_range(CMD_PAGE_ADDRESS_SET, y, (uint16_t)(y + height - 1u));

    /* Pixels go high byte first and fill the window row by row. */
    begin_command(CMD_MEMORY_WRITE);
    for (uint32_t count = (uint32_t)width * height; count > 0u; count--) {
        spi_send((uint8_t)(colour >> 8));
        spi_send((uint8_t)colour);
    }
    end_transfer();
}
