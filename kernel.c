typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

#define UART0_BASE 0x10000000UL
#define UART0_THR  (UART0_BASE + 0x00)
#define QEMU_POWEROFF_ADDR 0x100000
#define SHELL_BUF_LEN 64
#define QEMU_RESET_ADDR 0x100000

void uart_putc(char c) {
    volatile uint8_t* thr = (volatile uint8_t*)UART0_THR;
    *thr = c;
}

void uart_puts(const char* s) {
    while(*s) uart_putc(*s++);
}

void qemu_reboot() {
    volatile uint32_t* p = (volatile uint32_t*)QEMU_RESET_ADDR;
    *p = 0x7777;
}

void print_info() {
    uart_puts("Crystal OS\n");
    uart_puts("Built: "__DATE__ " " __TIME__ "\n");
}

void print_mem(const char* arg) {
    uint64_t addr = 0x80000000;
    int count = 16;
    // minimal: try to parse hex after "mem "
    if (arg && arg[0]) {
        addr = 0;
        for (int i = 0; i < 16 && arg[i]; i++) {
            char c = arg[i];
            addr <<= 4;
            if (c >= '0' && c <= '9') addr |= (c - '0');
            else if (c >= 'a' && c <= 'f') addr |= (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') addr |= (c - 'A' + 10);
            else break;
        }
    }
    uart_puts("mem[");
    // print addr as hex
    for (int i = (sizeof(addr)*2)-1; i >= 0; i--) {
        char hex = ((addr >> (i*4)) & 0xF);
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
    }
    uart_puts("]: ");
    for (int i = 0; i < count; i++) {
        uint8_t* p = (uint8_t*)(addr + i);
        char hex = ((*p) >> 4) & 0xF;
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
        hex = (*p) & 0xF;
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
        uart_putc(' ');
    }
    uart_puts("\r\n");
}

void print_csr() {
    uint64_t misa = 0, mstatus = 0, mepc = 0;
    asm volatile ("csrr %0, misa" : "=r"(misa));
    asm volatile ("csrr %0, mstatus" : "=r"(mstatus));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));
    uart_puts("CSR misa: 0x");
    for (int i = (sizeof(misa)*2)-1; i >= 0; i--) {
        char hex = ((misa >> (i*4)) & 0xF);
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
    }
    uart_puts("\nCSR mstatus: 0x");
    for (int i = (sizeof(mstatus)*2)-1; i >= 0; i--) {
        char hex = ((mstatus >> (i*4)) & 0xF);
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
    }
    uart_puts("\nCSR mepc: 0x");
    for (int i = (sizeof(mepc)*2)-1; i >= 0; i--) {
        char hex = ((mepc >> (i*4)) & 0xF);
        uart_putc(hex < 10 ? ('0'+hex) : ('a'+hex-10));
    }
    uart_puts("\n");
}

void clear_screen() {
    for (int i = 0; i < 50; i++) uart_puts("\r\n");
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
    uart_puts("Crystal OS shell. Type 'hello', 'exit', 'info', 'mem', 'csr', 'clear', or 'reboot'.\r\n");
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
        } else if (str_eq(buf, "info")) {
            print_info();
        } else if (str_eq(buf, "clear")) {
            clear_screen();
        } else if (str_eq(buf, "csr")) {
            print_csr();
        } else if (str_eq(buf, "reboot")) {
            uart_puts("Rebooting.\r\n");
            qemu_reboot();
            while (1) {}
        } else if (buf[0] == 'm' && buf[1] == 'e' && buf[2] == 'm') {
            const char* arg = 0;
            if (buf[3] == ' ') arg = buf + 4;
            print_mem(arg);
        } else if (buf[0] == 0) {
        } else {
            uart_puts("Unknown command: ");
            uart_puts(buf);
            uart_puts("\r\n");
        }
    }
}