#ifndef SYSTEM_H
#define SYSTEM_H

// Funções de controle do sistema
void system_reboot();
void system_shutdown();

// Funções de I/O
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short data);

#endif 