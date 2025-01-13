#ifndef SCREEN_H
#define SCREEN_H

// Constantes do vídeo
#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Funções para manipulação da tela
void clear_screen();
void print_string(const char* str);
void set_cursor(int x, int y);
void scroll_screen();
void scroll_up();
void scroll_down();
void refresh_screen();

// Funções de I/O
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
unsigned short inw(unsigned short port);
void outw(unsigned short port, unsigned short data);

// Variáveis de posição do cursor
extern int cursor_x;
extern int cursor_y;
extern int scroll_offset;

// Função para converter inteiro para string
void int_to_string(int num, char* str);

// Ponteiro para a função de impressão de caracteres
extern void (*print_char)(char);

#endif 