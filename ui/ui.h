#ifndef UI_H
#define UI_H

#define UI_WIDTH 80
#define UI_HEIGHT 25
#define UI_TASKBAR_HEIGHT 1
#define UI_MENU_WIDTH 20
#define UI_MENU_HEIGHT 8

typedef struct {
    int x;
    int y;
} UIMousePosition;

typedef struct {
    char title[32];
    int x;
    int y;
    int width;
    int height;
    int is_active;
    void (*render)(void);
} UIWindow;

// UI state
void ui_init(void);
void ui_main_loop(void);
void ui_cleanup(void);

// Mouse functions
void ui_move_mouse(int dx, int dy);
void ui_handle_click(int button);

// Window management
void ui_open_window(const char* title, int width, int height, void (*render)(void));
void ui_close_current_window(void);

// Menu functions
void ui_toggle_start_menu(void);

// Modifique as definições de teclas
#define EDITOR_KEY_ESC 27
#define EDITOR_KEY_ICA '\x1F'  // Mudando para um código não utilizado

#endif 