#include "system.h"
#include "screen.h"  // Para as funções de I/O

// Reboot usando o controlador de teclado
void system_reboot() {
    unsigned char good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    // Se chegou aqui, o reboot falhou
    while(1) { }
}

// Shutdown usando ACPI (simulado - apenas para QEMU)
void system_shutdown() {
    outw(0x604, 0x2000);  // Shutdown para QEMU
    
    // Se chegou aqui, o shutdown falhou
    while(1) { }
} 