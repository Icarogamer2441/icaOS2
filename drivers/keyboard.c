#include "keyboard.h"

// Mapa de scancodes para ASCII (layout US) - Sem Shift
static char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Mapa de scancodes para ASCII (layout US) - Com Shift
static char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static int shift_pressed = 0;

char get_key() {
    char c = 0;
    
    if(inb(KEYBOARD_STATUS_PORT) & 0x01) {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);
        
        // Verifica se é uma tecla ICA
        if(scancode == SCANCODE_ICAKEY || scancode == SCANCODE_ICAKEY_RIGHT) {
            return '\x1C';  // Código especial para a tecla ICA
        }
        
        // Verifica se são as setas
        if(scancode == SCANCODE_UP) {
            return KEY_UP;
        }
        if(scancode == SCANCODE_DOWN) {
            return KEY_DOWN;
        }
        
        // Verifica se é uma tecla Shift
        if(scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
            shift_pressed = 1;
            return 0;
        }
        // Verifica se é uma liberação de Shift
        else if(scancode == SCANCODE_LSHIFT_RELEASE || scancode == SCANCODE_RSHIFT_RELEASE) {
            shift_pressed = 0;
            return 0;
        }
        // Ignora outras teclas liberadas
        else if(scancode & 0x80) {
            return 0;
        }
        // Converte scancode para ASCII
        else if(scancode < sizeof(scancode_to_ascii)) {
            if(shift_pressed) {
                c = scancode_to_ascii_shift[scancode];
            } else {
                c = scancode_to_ascii[scancode];
            }
        }
    }
    
    return c;
}

char* get_line() {
    static char buffer[256];
    int i = 0;
    
    while(1) {
        char c = get_key();
        if(c == '\n') {
            buffer[i] = '\0';
            return buffer;
        } else if(c != 0) {
            buffer[i++] = c;
        }
    }
} 