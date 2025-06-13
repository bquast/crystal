typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

#define UART0_BASE 0x10000000UL
#define UART0_THR  (UART0_BASE + 0x00)
#define QEMU_POWEROFF_ADDR 0x100000
#define SHELL_BUF_LEN 64

void uart_putc(char c) {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    *thr = c;
}

void uart_puts(const char* s) {
    while(*s) uart_putc(*s++);
}

char uart_getc() {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    volatile uint8_t* lsr = (volatile uint8_t*)(UART0_BASE + 0x05);
    while(((*lsr) & 0x01) == 0) {}
    return *thr;
}

int str_eq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

void qemu_poweroff() {
    volatile uint32_t* p = (volatile uint32_t*)QEMU_POWEROFF_ADDR;
    *p = 0x5555;
}

void kernel_main() {
    char buf[SHELL_BUF_LEN];
    uart_puts("Crystal OS shell. Type 'hello' or 'exit'.\r\n");
    while (1) {
        uart_puts("> ");
        int idx = 0;
        while (1) {
            char c = uart_getc();
            if (c == '\r' || c == '\n') {
                uart_puts("\r\n");
                buf[idx] = 0;
                break;
            }
            if (c == 127 || c == 8) {
                if (idx > 0) {
                    idx--;
                    uart_puts("\b \b");
                }
                continue;
            }
            if (idx < SHELL_BUF_LEN - 1) {
                uart_putc(c);
                buf[idx++] = c;
            }
        }
        if (str_eq(buf, "hello")) {
            uart_puts("Hello, user!\r\n");
        } else if (str_eq(buf, "exit")) {
            uart_puts("Shutting down.\r\n");
            qemu_poweroff();
            while (1) {}
        } else if (buf[0] == 0) {
        } else {
            uart_puts("Unknown command: ");
            uart_puts(buf);
            uart_puts("\r\n");
        }
    }
}