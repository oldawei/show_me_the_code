#include "ns16550.h"

// the UART control registers.
// some have different meanings for read vs write.
// see http://byterunner.com/16550.html
#define UART_RHR_OFFSET      0 /* receive holding register (for input bytes) */
#define UART_THR_OFFSET      0 /* transmit holding register (for output bytes) */
#define UART_DLL_OFFSET      0 /* Divisor Latch (Least Significant Byte) Register (LSB) */
#define UART_IER_OFFSET      1 /* interrupt enable register */
#define UART_DLM_OFFSET      1 /* Divisor Latch (Most Significant Byte) Register (MSB) */
#define UART_FCR_OFFSET      2 /* FIFO control register */
#define UART_ISR_OFFSET      2 /* interrupt status register */
#define UART_LCR_OFFSET      3 /* line control register */
#define UART_LSR_OFFSET      5 /* line status register */

#define UART_LCR_8N1         0x03 /* useful defaults for LCR */
#define UART_LCR_DLAB        0x80 /* Divisor latch access bit */

#define UART_IER_RDI         0x01 /* Enable receiver data interrupt */
#define UART_IER_THRI        0x02 /* Enable Transmitter holding register int. */

#define UART_FCR_FIFO_EN     0x01 /* FIFO Enable */
#define UART_FCR_RXSR        0x02 /* Receiver soft reset */
#define UART_FCR_TXSR        0x04 /* Transmitter soft reset */

#define UART_LSR_DR          0x01 /* Receiver data ready */
#define UART_LSR_THRE        0x20 /* Transmit-hold-register empty */


#define UART_ADDR(addr)     (gs_uart_base + (addr))
#define UART_REG(reg)       (*(volatile u8 *)UART_ADDR(reg))

static u32 gs_uart_base = 0;

int UartPutc(int c)
{
    /* wait for Transmit Holding Empty to be set in LSR */
    while ((UART_REG(UART_LSR_OFFSET) & UART_LSR_THRE) == 0) {
        ;
    }
    UART_REG(UART_THR_OFFSET) = (u8)c;
    return c;
}

int UartGetc(void)
{
    if (UART_REG(UART_LSR_OFFSET) & UART_LSR_DR) {
        return UART_REG(UART_RHR_OFFSET);
    } else {
        return -1;
    }
}

int UartOut(int c)
{
    return UartGetc();
}

void UartInit(u32 base_addr)
{
    gs_uart_base = base_addr;

    /* Disable all interrupts */
    UART_REG(UART_IER_OFFSET) = 0x00;

    /* special mode to set baud rate */
    UART_REG(UART_LCR_OFFSET) = UART_LCR_DLAB;

    /* Set divisor low byte, LSB for baud rate of 115.2K */
    UART_REG(UART_DLL_OFFSET) = 0x01;

    /* Set divisor high byte, LSB for baud rate of 115.2K */
    UART_REG(UART_DLM_OFFSET) = 0x00;

    /* leave set-baud mode, and set word length to 8 bits, no parity */
    UART_REG(UART_LCR_OFFSET) = UART_LCR_8N1;

    /* reset and enable FIFOs */
    UART_REG(UART_FCR_OFFSET) = UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR;

    /* enable transmit and receive interrupts */
    //UART_REG(UART_IER_OFFSET) = UART_IER_RDI | UART_IER_THRI;
    UART_REG(UART_IER_OFFSET) = UART_IER_RDI;
}

void cli_putchar(u8 c);
void UartIsr(void)
{
    // read and process incoming characters.
    while(1){
        int c = UartGetc();
        if(c == -1)
            break;
        // input echo to console
        cli_putchar(c);
        UartPutc(c);
        if (c == '\r') {
            UartPutc('\n');
            cli_putchar('\n');
        }
    }
}