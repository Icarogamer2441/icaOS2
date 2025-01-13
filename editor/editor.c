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
        if (editor->content[display_pos] == '\n') lines_skipped++;
        display_pos++;
    }
    
    int screen_line = 0;
    while (display_pos < editor->size && screen_line < MAX_ROWS) {
        char c = editor->content[display_pos];
        if (c == '\n') {
            print_char(c);
            screen_line++;
        } else {
            print_char(c);
        }
        display_pos++;
    }
}

int editor_handle_key(Editor* editor, char key) {
    if (key == KEY_UP) {
        if (editor->view_offset > 0) {
            editor->view_offset--;
            editor_display(editor);
        }
        return 0;
    }
    
    if (key == KEY_DOWN) {
        int total_lines = count_lines_until(editor->content, editor->size);
        if (editor->view_offset < total_lines - MAX_ROWS + 1) {
            editor->view_offset++;
            editor_display(editor);
        }
        return 0;
    }

    if (key == EDITOR_KEY_ESC || key == EDITOR_KEY_ICA) {
        return 1;
    }
    
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
        }
        editor_display(editor);
        return 0;
    }
    
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