#include "screen.h"

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

int cursor_x = 0;
int cursor_y = 0;
int scroll_offset = 0;
char screen_buffer[MAX_ROWS * MAX_COLS * 2];  // Buffer para armazenar o conteúdo da tela

void clear_screen() {
    char* video_memory = (char*)VIDEO_MEMORY;
    
    for(int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    set_cursor(cursor_x, cursor_y);
}

// Implemente a função real de impressão
static void real_print_char(char c) {
    char* video_memory = (char*)VIDEO_MEMORY;
    
    if(c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int offset = (cursor_y * MAX_COLS + cursor_x) * 2;
        video_memory[offset] = c;
        video_memory[offset + 1] = 0x07;
        screen_buffer[offset] = c;
        screen_buffer[offset + 1] = 0x07;
        cursor_x++;
    }
    
    if(cursor_x >= MAX_COLS) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if(cursor_y >= MAX_ROWS) {
        scroll_screen();
        cursor_y = MAX_ROWS - 1;
    }
    
    set_cursor(cursor_x, cursor_y);
}

// Inicialize o ponteiro para função com a função real
void (*print_char)(char) = real_print_char;

void print_string(const char* str) {
    while(*str) {
        print_char(*str);
        str++;
    }
}

void scroll_screen() {
    char* video_memory = (char*)VIDEO_MEMORY;
    
    // Move todas as linhas uma posição para cima
    for(int i = 0; i < (MAX_ROWS-1) * MAX_COLS * 2; i++) {
        video_memory[i] = video_memory[i + MAX_COLS * 2];
        screen_buffer[i] = screen_buffer[i + MAX_COLS * 2];
    }
    
    // Limpa a última linha
    for(int i = (MAX_ROWS-1) * MAX_COLS * 2; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
        screen_buffer[i] = ' ';
        screen_buffer[i + 1] = 0x07;
    }
}

void scroll_up() {
    if (scroll_offset > 0) {
        scroll_offset--;
        refresh_screen();
    }
}

void scroll_down() {
    // Calcula o número máximo de linhas no buffer
    int max_lines = 0;
    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        if (screen_buffer[i] != '\0') max_lines = i / (MAX_COLS * 2) + 1;
    }
    
    if (scroll_offset < max_lines - MAX_ROWS) {
        scroll_offset++;
        refresh_screen();
    }
}

void refresh_screen() {
    char* video_memory = (char*)VIDEO_MEMORY;
    int start = scroll_offset * MAX_COLS * 2;
    
    // Copia do buffer para a memória de vídeo
    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i++) {
        video_memory[i] = screen_buffer[start + i];
    }
}

// Implementação das funções de porta I/O
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

void set_cursor(int x, int y) {
    unsigned short position = (y * MAX_COLS) + x;

    // Diz ao controlador VGA que vamos definir o byte alto do cursor
    outb(0x3D4, 14);
    outb(0x3D5, (position >> 8) & 0xFF);
    
    // Agora define o byte baixo
    outb(0x3D4, 15);
    outb(0x3D5, position & 0xFF);
}

unsigned short inw(unsigned short port) {
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void outw(unsigned short port, unsigned short data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void int_to_string(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    // Trata números negativos
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Trata o caso especial do zero
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    
    // Converte dígitos para caracteres
    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }
    
    // Adiciona o sinal negativo se necessário
    if (is_negative) {
        str[i++] = '-';
    }
    
    // Adiciona o terminador nulo
    str[i] = '\0';
    
    // Inverte a string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
} 