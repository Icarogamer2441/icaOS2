#ifndef EDITOR_H
#define EDITOR_H

#include "../fs/filesystem.h"

#define EDITOR_MAX_LINES 100
#define EDITOR_MAX_LINE_LENGTH 80
#define EDITOR_KEY_ESC 27
#define EDITOR_KEY_ICA KEY_ICA

typedef struct {
    char* content;
    int size;
    int cursor_pos;
    int is_modified;
    int view_offset;
} Editor;

void editor_init(Editor* editor, const char* content);
void editor_clear(Editor* editor);
void editor_display(Editor* editor);
int editor_handle_key(Editor* editor, char key);
char* editor_get_content(Editor* editor);
void editor_free(Editor* editor);

#endif 