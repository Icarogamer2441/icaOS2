#include "editor.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../fs/filesystem.h"

void editor_init(Editor* editor, const char* content) {
    editor->content = malloc(MAX_FILE_SIZE);
    if (content) {
        strcpy(editor->content, content);
        editor->size = strlen(content);
    } else {
        editor->content[0] = '\0';
        editor->size = 0;
    }
    editor->cursor_pos = editor->size;
    editor->is_modified = 0;
    editor->view_offset = 0;
}

void editor_clear(Editor* editor) {
    editor->content[0] = '\0';
    editor->size = 0;
    editor->cursor_pos = 0;
    editor->is_modified = 1;
    editor->view_offset = 0;
}

static int count_lines_until(const char* text, int pos) {
    int lines = 0;
    for (int i = 0; i < pos; i++) {
        if (text[i] == '\n') lines++;
    }
    return lines;
}

static int find_line_start(const char* text, int pos) {
    while (pos > 0 && text[pos - 1] != '\n') pos--;
    return pos;
}

void editor_display(Editor* editor) {
    clear_screen();
    
    int display_pos = 0;
    int lines_skipped = 0;
    while (display_pos < editor->size && lines_skipped < editor->view_offset) {
        if (editor->content[display_pos] == '\n') {
            lines_skipped++;
        }
        display_pos++;
    }
    
    int screen_line = 0;
    int chars_in_line = 0;
    while (display_pos < editor->size && screen_line < MAX_ROWS - 1) {
        char c = editor->content[display_pos];
        
        if (c == '\n') {
            print_char(c);
            screen_line++;
            chars_in_line = 0;
        } else {
            if (chars_in_line < MAX_COLS - 1) {
                print_char(c);
                chars_in_line++;
            }
        }
        display_pos++;
    }
}

int editor_handle_key(Editor* editor, char key) {
    // Primeiro verifica teclas especiais
    if (key == KEY_UP) {
        if (editor->view_offset > 0) {
            editor->view_offset--;
            editor_display(editor);
        }
        return 0;
    }
    
    if (key == KEY_DOWN) {
        int total_lines = 1;
        for (int i = 0; i < editor->size; i++) {
            if (editor->content[i] == '\n') {
                total_lines++;
            }
        }
        
        int visible_lines = MAX_ROWS - 1;
        if (editor->view_offset < total_lines - visible_lines) {
            editor->view_offset++;
            editor_display(editor);
        }
        return 0;
    }

    // Depois verifica ICA e ESC
    if (key == KEY_ICA) {
        if (editor->is_modified) {
            print_string("\nICA key pressed. ");
            print_string("(s)ave and exit, (e)xit without saving, or (c)ontinue editing? ");
            
            while(1) {
                char choice = get_key();
                if (choice == 's' || choice == 'S') {
                    return 1;  // Salvar e sair
                }
                else if (choice == 'e' || choice == 'E') {
                    print_string("\nExiting without saving...\n");
                    return 2;  // Sair sem salvar
                }
                else if (choice == 'c' || choice == 'C') {
                    editor_display(editor);
                    return 0;  // Continuar editando
                }
            }
        }
        return 1;  // Se não foi modificado, apenas sai
    }

    if (key == EDITOR_KEY_ESC) {
        if (editor->is_modified) {
            print_string("\nDo you want to (s)ave and exit or (c)ontinue editing? ");
            
            while(1) {
                char choice = get_key();
                if (choice == 's' || choice == 'S') {
                    return 1;  // Salvar e sair
                }
                else if (choice == 'c' || choice == 'C') {
                    editor_display(editor);
                    return 0;  // Continuar editando
                }
            }
        }
        return 1;  // Se não foi modificado, apenas sai
    }

    // Por fim, trata entrada normal de texto
    if (key == '\b') {
        if (editor->cursor_pos > 0) {
            for (int i = editor->cursor_pos - 1; i < editor->size; i++) {
                editor->content[i] = editor->content[i + 1];
            }
            editor->cursor_pos--;
            editor->size--;
            editor->is_modified = 1;
            
            int cursor_line = count_lines_until(editor->content, editor->cursor_pos);
            if (cursor_line < editor->view_offset) {
                editor->view_offset = cursor_line;
            }
            editor_display(editor);
        }
        return 0;
    }
    
    // Entrada normal de texto
    if (editor->size < MAX_FILE_SIZE - 1) {
        for (int i = editor->size; i > editor->cursor_pos; i--) {
            editor->content[i] = editor->content[i - 1];
        }
        editor->content[editor->cursor_pos] = key;
        editor->cursor_pos++;
        editor->size++;
        editor->content[editor->size] = '\0';
        editor->is_modified = 1;
        
        if (key == '\n') {
            int cursor_line = count_lines_until(editor->content, editor->cursor_pos);
            if (cursor_line >= editor->view_offset + MAX_ROWS) {
                editor->view_offset++;
            }
        }
        
        editor_display(editor);
    }
    
    return 0;
}

char* editor_get_content(Editor* editor) {
    return editor->content;
}

void editor_free(Editor* editor) {
    free(editor->content);
} 