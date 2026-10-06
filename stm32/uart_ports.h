#ifndef UART_PORTS_H
#define UART_PORTS_H

/* This board's UART ports; board.c binds each one to a USART and its pins. */
typedef enum {
    UART_CONSOLE, /* the ST-LINK's virtual COM port */
    UART_COUNT,
} uart_id_t;

#endif
