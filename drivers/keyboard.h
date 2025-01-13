#ifndef KEYBOARD_H
#define KEYBOARD_H

// Portas de I/O do teclado
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Scancodes especiais
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_ICAKEY 0x5B      // Tecla Windows esquerda
#define SCANCODE_ICAKEY_RIGHT 0x5C // Tecla Windows direita
#define SCANCODE_UP 0x48          // Seta para cima
#define SCANCODE_DOWN 0x50        // Seta para baixo
#define SCANCODE_LEFT 0x4B        // Seta para esquerda
#define SCANCODE_RIGHT 0x4D       // Seta para direita

// Códigos especiais para retorno
#define KEY_UP    '\x1A'
#define KEY_DOWN  '\x1B'
#define KEY_LEFT  '\x1C'
#define KEY_RIGHT '\x1D'
#define KEY_ICA   '\x1F'  // Código único para a tecla ICA

// Funções do teclado
char get_key();
char* get_line();

// Funções de I/O
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);

// Estado do teclado
extern int shift_pressed;

#endif 