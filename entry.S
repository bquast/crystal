    .section .text
    .global _start
_start:
    la sp, _stack_top      # setup stack
    call kernel_main
1:  j 1b                   # loop forever

    .section .bss
    .align 12
    .global _stack
_stack:
    .skip 4096             # 4KB stack
    .global _stack_top
_stack_top: