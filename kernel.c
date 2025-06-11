// all code is flat, no includes, all defines inline

typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

// Virt board UART0 MMIO address (from QEMU virt, see docs)
#define UART0_BASE 0x10000000UL
#define UART0_THR  (UART0_BASE + 0x00) // Transmit Holding Register

// minimal input buffer size
#define SHELL_BUF_LEN 64

// send one character
void uart_putc(char c) {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    *thr = c;
}

// send a string
void uart_puts(const char* s) {
    while(*s) uart_putc(*s++);
}

// get a character from UART (polling)
char uart_getc() {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    volatile uint8_t* lsr = (volatile uint8_t*)(UART0_BASE + 0x05);
    while(((*lsr) & 0x01) == 0) {} // wait for data ready
    return *thr;
}

// super minimal string compare, returns 1 if same
int str_eq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
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
                buf[idx] = 0; // null-terminate
                break;
            }
            if (c == 127 || c == 8) { // backspace
                if (idx > 0) {
                    idx--;
                    uart_puts("\b \b");
                }
                continue;
            }
            if (idx < SHELL_BUF_LEN - 1) {
                uart_putc(c); // echo
                buf[idx++] = c;
            }
        }

        // process command
        if (str_eq(buf, "hello")) {
            uart_puts("Hello, user!\r\n");
        } else if (str_eq(buf, "exit")) {
            uart_puts("Shutting down.\r\n");
            while (1) {}
        } else if (buf[0] == 0) {
            // do nothing on empty input
        } else {
            uart_puts("Unknown command: ");
            uart_puts(buf);
            uart_puts("\r\n");
        }
    }
}