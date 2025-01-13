; Constantes do Multiboot
MULTIBOOT_PAGE_ALIGN  equ  1<<0   ; Alinhamento em limites de página
MULTIBOOT_MEMORY_INFO equ  1<<1   ; Informações de memória
MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

; Seção multiboot deve estar no início
section .multiboot
align 4
multiboot_header:
    dd MULTIBOOT_HEADER_MAGIC   ; magic
    dd MULTIBOOT_HEADER_FLAGS   ; flags
    dd MULTIBOOT_CHECKSUM       ; checksum

; Seção de código
section .text
global start
extern kernel_main

start:
    cli                     ; Desabilita interrupções
    mov esp, stack_space    ; Configura a stack
    push ebx                ; Passa o ponteiro da estrutura multiboot
    call kernel_main        ; Chama o kernel
    cli
.hang:
    hlt                     ; Halt se o kernel retornar
    jmp .hang              ; Loop infinito

; Seção BSS para a stack
section .bss
align 16
stack_bottom:
    resb 16384             ; 16 KB para a stack
stack_space: 