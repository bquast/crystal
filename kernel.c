// all code is flat, no includes, all defines inline

typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

// Virt board UART0 MMIO address (from QEMU virt, see docs)
#define UART0_BASE 0x10000000UL
#define UART0_THR  (UART0_BASE + 0x00) // Transmit Holding Register

// send one character
void uart_putc(char c) {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    *thr = c;
}

// send a string
void uart_puts(const char* s) {
    while(*s) uart_putc(*s++);
}

void kernel_main() {
    uart_puts("Hello from crystal OS!\r\n");
    while (1) {} // spin forever
}