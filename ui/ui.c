#include "ui.h"
#include "../drivers/screen.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../drivers/keyboard.h"
#include "../kernel/kernel.h"  // Para execute_command
#include "../fs/filesystem.h"  // Para funções do sistema de arquivos

#define MAX_WINDOWS 1
#define MOUSE_CHAR 'X'
#define WINDOW_BORDER_CHAR '+'
#define WINDOW_TITLE_CHAR '-'
#define TERMINAL_BUFFER_SIZE 1000
#define TERMINAL_VISIBLE_LINES 13  // height - 2 para bordas
#define TERMINAL_WIDTH 58          // width - 2 para bordas
#define TERMINAL_PROMPT "icaOS2$ "
#define CURSOR_CHAR '_'
#define CURSOR_BLINK_RATE 15  // Taxa de piscar do cursor
#define KEY_COMBO_TIMEOUT 10  // Tempo para detectar combinação de teclas
#define MAX_FILES_VISIBLE 12  // Número máximo de arquivos visíveis na janela
#define FILE_LIST_START_Y 6   // Linha onde começa a lista de arquivos
#define EDITOR_WIDTH 58          // width - 2 para bordas
#define EDITOR_HEIGHT 15         // height - 2 para bordas
#define EDITOR_BUFFER_SIZE 4096

static UIMousePosition mouse = {0, 0};
static int start_menu_open = 0;
static UIWindow* current_window = NULL;
static char screen_buffer[UI_HEIGHT][UI_WIDTH];
static int ui_active = 0;

typedef struct {
    char lines[TERMINAL_BUFFER_SIZE][TERMINAL_WIDTH];
    int current_line;
    int scroll_offset;
    char command_buffer[256];
    int command_pos;
    int edit_mode;          // Novo: indica se está em modo de edição
    int cursor_visible;     // Novo: controla a visibilidade do cursor
    int blink_counter;      // Novo: contador para piscar o cursor
    int ica_pressed;         // Flag para tecla ICA
    int combo_timer;         // Timer para combinações
} Terminal;

static Terminal terminal = {
    .current_line = 0,
    .scroll_offset = 0,
    .command_pos = 0,
    .edit_mode = 1,        // Começa em modo de edição
    .cursor_visible = 1,   // Cursor começa visível
    .blink_counter = 0,
    .ica_pressed = 0,
    .combo_timer = 0
};

typedef struct {
    int selected_index;    // Índice do arquivo selecionado
    int scroll_offset;     // Offset para scroll da lista
    int total_items;       // Total de itens na lista atual
} FileManager;

static FileManager file_manager = {
    .selected_index = 0,
    .scroll_offset = 0,
    .total_items = 0
};

typedef struct {
    char content[EDITOR_BUFFER_SIZE];
    int cursor_pos;
    int scroll_offset;
    char filename[32];
    int is_modified;
} TextEditor;

static TextEditor text_editor = {
    .cursor_pos = 0,
    .scroll_offset = 0,
    .is_modified = 0
};

// Forward declarations
static void render_taskbar(void);
static void render_start_menu(void);
static void render_mouse(void);
static void render_window(void);
static void terminal_window_render(void);
static void info_window_render(void);
static void ui_render(void);
static void files_window_render(void);
static void files_handle_key(char key);
static void text_editor_render(void);
static void text_editor_handle_key(char key);

void ui_init(void) {
    // Limpa a tela e posiciona o cursor no início
    clear_screen();
    set_cursor(0, 0);
    
    // Inicializa o mouse no centro da tela
    mouse.x = UI_WIDTH / 2;
    mouse.y = UI_HEIGHT / 2;
    ui_active = 1;
    
    // Limpa o buffer da UI
    memset(screen_buffer, ' ', sizeof(screen_buffer));
    
    // Faz a primeira renderização
    ui_render();
}

void ui_cleanup(void) {
    ui_active = 0;
    clear_screen();
}

static void ui_render(void) {
    // Limpa o buffer
    memset(screen_buffer, ' ', sizeof(screen_buffer));
    
    // Renderiza os componentes em memória
    render_taskbar();
    if (start_menu_open) {
        render_start_menu();
    }
    if (current_window) {
        render_window();
    }
    render_mouse();
    
    // Escreve diretamente na memória de vídeo
    char* video_memory = (char*)VIDEO_MEMORY;
    for (int y = 0; y < UI_HEIGHT && y < MAX_ROWS; y++) {
        for (int x = 0; x < UI_WIDTH && x < MAX_COLS; x++) {
            int offset = (y * MAX_COLS + x) * 2;
            video_memory[offset] = screen_buffer[y][x];
            video_memory[offset + 1] = 0x07;  // Atributo de cor padrão (branco sobre preto)
        }
    }
    
    // Atualiza a posição do cursor
    set_cursor(0, 0);
}

static void render_taskbar(void) {
    // Coloca a barra de tarefas na última linha da tela
    int taskbar_y = MAX_ROWS - 1;
    
    // Draw taskbar background
    for (int x = 0; x < UI_WIDTH && x < MAX_COLS; x++) {
        screen_buffer[taskbar_y][x] = '-';
    }
    
    // Draw start button
    const char* start_text = "[Start]";
    memcpy(&screen_buffer[taskbar_y][0], start_text, strlen(start_text));
    
    // Draw window title if active
    if (current_window) {
        int title_pos = MAX_COLS - strlen(current_window->title) - 2;
        if (title_pos >= 0) {
            memcpy(&screen_buffer[taskbar_y][title_pos],
                   current_window->title, strlen(current_window->title));
        }
    }
}

static void render_start_menu(void) {
    // Ajusta o menu para começar do fundo da tela
    int menu_y = MAX_ROWS - UI_MENU_HEIGHT - 2;  // -2 para deixar espaço para a barra de tarefas
    int menu_x = 0;
    
    // Draw border
    for (int y = menu_y; y < menu_y + UI_MENU_HEIGHT && y < MAX_ROWS - 1; y++) {
        for (int x = menu_x; x < UI_MENU_WIDTH && x < MAX_COLS; x++) {
            if (y == menu_y || y == menu_y + UI_MENU_HEIGHT - 1 ||
                x == menu_x || x == UI_MENU_WIDTH - 1) {
                screen_buffer[y][x] = '+';
            }
        }
    }
    
    // Draw menu items
    const char* terminal_text = "1. Terminal";
    const char* files_text = "2. ICA Files";
    const char* info_text = "3. Info";
    memcpy(&screen_buffer[menu_y + 2][2], terminal_text, strlen(terminal_text));
    memcpy(&screen_buffer[menu_y + 3][2], files_text, strlen(files_text));
    memcpy(&screen_buffer[menu_y + 4][2], info_text, strlen(info_text));
}

static void render_window(void) {
    if (!current_window) return;
    
    // Ajusta a posição e tamanho da janela para não sair da tela
    int window_y = current_window->y;
    int window_height = current_window->height;
    
    if (window_y + window_height > UI_HEIGHT) {
        window_height = UI_HEIGHT - window_y;
    }
    
    // Draw window border
    for (int y = window_y; y < window_y + window_height && y < UI_HEIGHT; y++) {
        for (int x = current_window->x; x < current_window->x + current_window->width && x < UI_WIDTH; x++) {
            if (y == window_y || y == window_y + window_height - 1 ||
                x == current_window->x || x == current_window->x + current_window->width - 1) {
                screen_buffer[y][x] = WINDOW_BORDER_CHAR;
            }
        }
    }
    
    // Draw window title
    if (window_y < UI_HEIGHT) {
        int title_x = current_window->x + (current_window->width - strlen(current_window->title)) / 2;
        memcpy(&screen_buffer[window_y][title_x], current_window->title, strlen(current_window->title));
    }
    
    // Render window content
    if (current_window->render) {
        current_window->render();
    }
}

static void render_mouse(void) {
    // Removemos esta função pois agora o mouse é renderizado diretamente
    // na memória de vídeo e não precisa ser armazenado no screen_buffer
}

void ui_move_mouse(int dx, int dy) {
    // Salva a posição antiga do mouse
    int old_x = mouse.x;
    int old_y = mouse.y;
    
    // Atualiza a posição
    mouse.x = (mouse.x + dx + UI_WIDTH) % UI_WIDTH;
    mouse.y = (mouse.y + dy + UI_HEIGHT) % UI_HEIGHT;
    
    char* video_memory = (char*)VIDEO_MEMORY;
    
    // Restaura o caractere original na posição antiga
    if (old_x < UI_WIDTH && old_y < UI_HEIGHT) {
        int old_offset = (old_y * MAX_COLS + old_x) * 2;
        video_memory[old_offset] = screen_buffer[old_y][old_x];
        video_memory[old_offset + 1] = 0x07;
    }
    
    // Salva o caractere atual e desenha o mouse
    if (mouse.x < UI_WIDTH && mouse.y < UI_HEIGHT) {
        int new_offset = (mouse.y * MAX_COLS + mouse.x) * 2;
        // Não precisamos atualizar o screen_buffer aqui, apenas a memória de vídeo
        video_memory[new_offset] = MOUSE_CHAR;
        video_memory[new_offset + 1] = 0x07;
    }
}

void ui_handle_click(int button) {
    // Check if clicking start button
    if (mouse.y == MAX_ROWS - 1 && mouse.x < 7) {
        start_menu_open = !start_menu_open;
        ui_render();
        return;
    }
    
    // Check start menu clicks
    int menu_y = MAX_ROWS - UI_MENU_HEIGHT - 2;
    if (start_menu_open && mouse.y >= menu_y && mouse.y < MAX_ROWS - 1 && 
        mouse.x < UI_MENU_WIDTH) {
        if (mouse.y == menu_y + 2) { // Terminal
            ui_open_window("Terminal", 60, 15, terminal_window_render);
        } else if (mouse.y == menu_y + 3) { // ICA Files
            ui_open_window("ICA Files", 60, 20, files_window_render);
        } else if (mouse.y == menu_y + 4) { // Info
            ui_open_window("System Info", 40, 10, info_window_render);
        }
        start_menu_open = 0;
        ui_render();
    }
}

void ui_toggle_start_menu(void) {
    start_menu_open = !start_menu_open;
    ui_render();
}

void ui_open_window(const char* title, int width, int height, void (*render)(void)) {
    if (current_window) {
        free(current_window);
    }
    
    current_window = malloc(sizeof(UIWindow));
    strcpy(current_window->title, title);
    current_window->x = (UI_WIDTH - width) / 2;
    current_window->y = (UI_HEIGHT - height) / 2;
    current_window->width = width;
    current_window->height = height;
    current_window->is_active = 1;
    current_window->render = render;
    
    ui_render();
}

void ui_close_current_window(void) {
    if (current_window) {
        free(current_window);
        current_window = NULL;
        ui_render();
    }
}

static void terminal_window_render(void) {
    int start_x = current_window->x + 1;
    int start_y = current_window->y + 1;
    int visible_start = terminal.current_line - TERMINAL_VISIBLE_LINES + terminal.scroll_offset;
    
    // Renderiza as linhas visíveis
    for (int i = 0; i < TERMINAL_VISIBLE_LINES && (visible_start + i) < terminal.current_line; i++) {
        int line_idx = visible_start + i;
        if (line_idx >= 0) {
            memcpy(&screen_buffer[start_y + i][start_x], 
                   terminal.lines[line_idx], 
                   strlen(terminal.lines[line_idx]));
        }
    }
    
    // Renderiza a linha de comando atual
    int prompt_len = strlen(TERMINAL_PROMPT);
    memcpy(&screen_buffer[start_y + TERMINAL_VISIBLE_LINES - 1][start_x],
           TERMINAL_PROMPT,
           prompt_len);
    memcpy(&screen_buffer[start_y + TERMINAL_VISIBLE_LINES - 1][start_x + prompt_len],
           terminal.command_buffer,
           terminal.command_pos);
           
    // Renderiza o cursor se estiver em modo de edição
    if (terminal.edit_mode && terminal.cursor_visible) {
        screen_buffer[start_y + TERMINAL_VISIBLE_LINES - 1]
                    [start_x + prompt_len + terminal.command_pos] = CURSOR_CHAR;
    }
    
    // Atualiza o contador de piscar do cursor
    terminal.blink_counter = (terminal.blink_counter + 1) % CURSOR_BLINK_RATE;
    if (terminal.blink_counter == 0) {
        terminal.cursor_visible = !terminal.cursor_visible;
    }
}

static void info_window_render(void) {
    const char* title = "icaOS textUI v1.0";
    const char* info1 = "A text-based UI system";
    const char* info2 = "Disk: ATA Primary Master";
    
    memcpy(&screen_buffer[current_window->y + 2][current_window->x + 2],
           title, strlen(title));
    memcpy(&screen_buffer[current_window->y + 4][current_window->x + 2],
           info1, strlen(info1));
    memcpy(&screen_buffer[current_window->y + 6][current_window->x + 2],
           info2, strlen(info2));
}

static void terminal_add_line(const char* text) {
    strncpy(terminal.lines[terminal.current_line], text, TERMINAL_WIDTH - 1);
    terminal.lines[terminal.current_line][TERMINAL_WIDTH - 1] = '\0';
    terminal.current_line++;
    
    // Ajusta o scroll se necessário
    if (terminal.current_line - terminal.scroll_offset > TERMINAL_VISIBLE_LINES) {
        terminal.scroll_offset = terminal.current_line - TERMINAL_VISIBLE_LINES;
    }
}

static void terminal_execute_command(void) {
    // Adiciona o comando ao histórico
    char full_command[TERMINAL_WIDTH];
    snprintf(full_command, TERMINAL_WIDTH, "%s%s", TERMINAL_PROMPT, terminal.command_buffer);
    terminal_add_line(full_command);
    
    // Configura redirecionamento de saída
    void (*old_print_char)(char) = print_char;
    char output_buffer[4096] = {0};
    int output_pos = 0;
    
    print_char = (void (*)(char))({
        void temp_print_char(char c) {
            if (output_pos < sizeof(output_buffer) - 1) {
                output_buffer[output_pos++] = c;
            }
        }
        temp_print_char;
    });
    
    // Executa o comando
    execute_command(terminal.command_buffer);
    
    // Restaura a função de impressão
    print_char = old_print_char;
    
    // Processa a saída linha por linha
    char* line = output_buffer;
    char* next_line;
    while ((next_line = strchr(line, '\n')) != NULL) {
        *next_line = '\0';
        terminal_add_line(line);
        line = next_line + 1;
    }
    if (*line) {
        terminal_add_line(line);
    }
    
    // Limpa o buffer de comando
    memset(terminal.command_buffer, 0, sizeof(terminal.command_buffer));
    terminal.command_pos = 0;
    
    ui_render();
}

static void terminal_handle_key(char key) {
    if (!current_window || current_window->render != terminal_window_render) {
        return;
    }
    
    // Verifica se é a tecla ICA
    if (key == KEY_ICA) {
        terminal.edit_mode = !terminal.edit_mode;
        if (!terminal.edit_mode) {
            ui_close_current_window();  // Fecha a janela do terminal
        }
        return;
    }
    
    // Verifica combinação ICA + Q para retornar à UI
    if (key == KEY_ICA) {
        terminal.ica_pressed = 1;
        terminal.combo_timer = KEY_COMBO_TIMEOUT;
        return;
    }
    
    if (terminal.ica_pressed && terminal.combo_timer > 0) {
        if (key == 'q' || key == 'Q') {
            terminal.edit_mode = 1;  // Reativa modo de edição
            terminal.cursor_visible = 1;
            ui_render();
            return;
        }
        terminal.combo_timer--;
    } else {
        terminal.ica_pressed = 0;
    }
    
    // Se não estiver em modo de edição, ignora entrada de texto
    if (!terminal.edit_mode) {
        if (key == KEY_UP || key == KEY_DOWN) {
            // Permite scroll mesmo fora do modo de edição
            if (key == KEY_UP && terminal.scroll_offset < terminal.current_line - TERMINAL_VISIBLE_LINES) {
                terminal.scroll_offset++;
                ui_render();
            }
            else if (key == KEY_DOWN && terminal.scroll_offset > 0) {
                terminal.scroll_offset--;
                ui_render();
            }
        }
        return;
    }
    
    // Processamento normal das teclas em modo de edição
    if (key == '\n') {
        terminal_execute_command();
    }
    else if (key == '\b') {
        if (terminal.command_pos > 0) {
            terminal.command_pos--;
            terminal.command_buffer[terminal.command_pos] = '\0';
            ui_render();
        }
    }
    else if (key == KEY_UP) {
        if (terminal.scroll_offset < terminal.current_line - TERMINAL_VISIBLE_LINES) {
            terminal.scroll_offset++;
            ui_render();
        }
    }
    else if (key == KEY_DOWN) {
        if (terminal.scroll_offset > 0) {
            terminal.scroll_offset--;
            ui_render();
        }
    }
    else if (terminal.command_pos < sizeof(terminal.command_buffer) - 1 && key >= ' ' && key <= '~') {
        terminal.command_buffer[terminal.command_pos++] = key;
        terminal.command_buffer[terminal.command_pos] = '\0';
        ui_render();
    }
}

static void files_window_render(void) {
    int start_x = current_window->x + 1;
    int start_y = current_window->y + 1;
    
    // Desenha o título
    const char* title = "++++++++++++++++++++ICA Files+++++++++++++++++++";
    memcpy(&screen_buffer[start_y][start_x], title, strlen(title));
    
    // Mostra o caminho atual
    char* current_path = fs_get_current_path();
    memcpy(&screen_buffer[start_y + 1][start_x], "Path: ", 6);
    memcpy(&screen_buffer[start_y + 1][start_x + 6], current_path, strlen(current_path));
    
    // Mostra as instruções
    const char* help_text[] = {
        "Commands:",
        "[N]ew [M]kdir [D]el [Enter]Open [B]ack [S]ave [L]oad [Q]uit",
        "Use arrow keys to navigate",
        "Files and Directories:"
    };
    
    for (int i = 0; i < 4; i++) {
        memcpy(&screen_buffer[start_y + 2 + i][start_x], 
               help_text[i], strlen(help_text[i]));
    }
    
    // Prepara a lista de arquivos e diretórios
    char* file_list[MAX_FILES];
    int list_size = 0;
    file_manager.total_items = 0;
    
    // Primeiro adiciona os diretórios
    for (int i = 0; i < fs_size; i++) {
        if (fs_nodes[i].parent_dir == current_dir && fs_nodes[i].is_directory) {
            char* entry = malloc(64);
            snprintf(entry, 64, "D: %s", fs_nodes[i].name);
            file_list[list_size++] = entry;
            file_manager.total_items++;
        }
    }
    
    // Depois adiciona os arquivos
    for (int i = 0; i < fs_size; i++) {
        if (fs_nodes[i].parent_dir == current_dir && !fs_nodes[i].is_directory) {
            char* entry = malloc(64);
            snprintf(entry, 64, "F: %s", fs_nodes[i].name);
            file_list[list_size++] = entry;
            file_manager.total_items++;
        }
    }
    
    // Ajusta a seleção se necessário
    if (file_manager.selected_index >= file_manager.total_items) {
        file_manager.selected_index = file_manager.total_items - 1;
    }
    if (file_manager.selected_index < 0) {
        file_manager.selected_index = 0;
    }
    
    // Renderiza a lista com scroll
    int visible_start = file_manager.scroll_offset;
    int visible_end = visible_start + MAX_FILES_VISIBLE;
    if (visible_end > list_size) visible_end = list_size;
    
    int list_y = start_y + FILE_LIST_START_Y;
    for (int i = visible_start; i < visible_end; i++) {
        char line[64];
        if (i == file_manager.selected_index) {
            snprintf(line, sizeof(line), "> %s", file_list[i]);
        } else {
            snprintf(line, sizeof(line), "  %s", file_list[i]);
        }
        memcpy(&screen_buffer[list_y++][start_x], line, strlen(line));
    }
    
    // Mostra indicadores de scroll se necessário
    if (file_manager.scroll_offset > 0) {
        memcpy(&screen_buffer[start_y + FILE_LIST_START_Y - 1][start_x + 25], "^ More ^", 8);
    }
    if (visible_end < list_size) {
        memcpy(&screen_buffer[list_y][start_x + 25], "v More v", 8);
    }
    
    // Limpa a memória alocada
    for (int i = 0; i < list_size; i++) {
        free(file_list[i]);
    }
    
    // Se a lista estiver vazia
    if (list_size == 0) {
        const char* empty_msg = "< Empty Directory >";
        memcpy(&screen_buffer[start_y + FILE_LIST_START_Y][start_x], 
               empty_msg, strlen(empty_msg));
    }
}

static void files_handle_key(char key) {
    if (!current_window || current_window->render != files_window_render) {
        return;
    }
    
    switch(key) {
        case KEY_UP:
            if (file_manager.selected_index > 0) {
                file_manager.selected_index--;
                if (file_manager.selected_index < file_manager.scroll_offset) {
                    file_manager.scroll_offset--;
                }
                ui_render();
            }
            break;
            
        case KEY_DOWN:
            if (file_manager.selected_index < file_manager.total_items - 1) {
                file_manager.selected_index++;
                if (file_manager.selected_index >= file_manager.scroll_offset + MAX_FILES_VISIBLE) {
                    file_manager.scroll_offset++;
                }
                ui_render();
            }
            break;
            
        case '\n': {
            int index = file_manager.selected_index;
            int current_item = 0;
            
            for (int i = 0; i < fs_size; i++) {
                if (fs_nodes[i].parent_dir == current_dir) {
                    if (current_item == index) {
                        if (fs_nodes[i].is_directory) {
                            fs_cd(fs_nodes[i].name);
                            file_manager.selected_index = 0;
                            file_manager.scroll_offset = 0;
                        } else {
                            // Abre o arquivo no editor
                            char* content = fs_read(fs_nodes[i].name);
                            if (content) {
                                strncpy(text_editor.content, content, EDITOR_BUFFER_SIZE - 1);
                                text_editor.content[EDITOR_BUFFER_SIZE - 1] = '\0';
                                text_editor.cursor_pos = strlen(text_editor.content); // Cursor no final
                                text_editor.scroll_offset = 0;
                                text_editor.is_modified = 0;
                                strcpy(text_editor.filename, fs_nodes[i].name);
                                ui_open_window("Text Editor", 62, 17, text_editor_render);
                            }
                        }
                        break;
                    }
                    current_item++;
                }
            }
            ui_render();
            break;
        }
            
        case 'n':
        case 'N': {
            // Interface para criar arquivo
            int input_y = current_window->y + current_window->height - 2;
            memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
            memcpy(&screen_buffer[input_y][current_window->x + 1], 
                   "New file name: ", 14);
            ui_render();
            
            char name[32] = {0};
            char* input = get_line();
            strncpy(name, input, 31);
            
            if (fs_mkfile(name) == 0) {
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "File created successfully!", 25);
            } else {
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "Error creating file!", 20);
            }
            ui_render();
            break;
        }
        
        case 'm':
        case 'M': {
            // Interface para criar diretório
            int input_y = current_window->y + current_window->height - 2;
            memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
            memcpy(&screen_buffer[input_y][current_window->x + 1], 
                   "New directory name: ", 19);
            ui_render();
            
            char name[32] = {0};
            char* input = get_line();
            strncpy(name, input, 31);
            
            if (fs_mkdir(name) == 0) {
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "Directory created successfully!", 30);
            } else {
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "Error creating directory!", 25);
            }
            ui_render();
            break;
        }
        
        case 'd':
        case 'D': {
            // Encontra o item selecionado
            int index = file_manager.selected_index;
            int current_item = 0;
            
            for (int i = 0; i < fs_size; i++) {
                if (fs_nodes[i].parent_dir == current_dir) {
                    if (current_item == index) {
                        int input_y = current_window->y + current_window->height - 2;
                        memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                        
                        if (fs_nodes[i].is_directory) {
                            if (fs_rmdir(fs_nodes[i].name) == 0) {
                                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                                       "Directory deleted successfully!", 30);
                            } else {
                                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                                       "Error deleting directory!", 25);
                            }
                        } else {
                            if (fs_rmfile(fs_nodes[i].name) == 0) {
                                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                                       "File deleted successfully!", 25);
                            } else {
                                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                                       "Error deleting file!", 20);
                            }
                        }
                        
                        if (file_manager.selected_index >= file_manager.total_items - 1) {
                            file_manager.selected_index--;
                            if (file_manager.selected_index < 0) file_manager.selected_index = 0;
                        }
                        break;
                    }
                    current_item++;
                }
            }
            ui_render();
            break;
        }
        
        case 'b':
        case 'B':
            fs_cd("..");
            file_manager.selected_index = 0;
            file_manager.scroll_offset = 0;
            ui_render();
            break;
            
        case 'q':
        case 'Q':
            ui_close_current_window();
            break;
        
        case 's':
        case 'S':
            fs_save(NULL);  // Salva o sistema de arquivos
            {
                int input_y = current_window->y + current_window->height - 2;
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "Filesystem saved successfully!", 30);
                ui_render();
            }
            break;
            
        case 'l':
        case 'L':
            fs_load(NULL);  // Carrega o sistema de arquivos
            file_manager.selected_index = 0;
            file_manager.scroll_offset = 0;
            {
                int input_y = current_window->y + current_window->height - 2;
                memset(&screen_buffer[input_y][current_window->x + 1], ' ', current_window->width - 2);
                memcpy(&screen_buffer[input_y][current_window->x + 1], 
                       "Filesystem loaded successfully!", 31);
                ui_render();
            }
            break;
    }
}

static void text_editor_render(void) {
    int start_x = current_window->x + 1;
    int start_y = current_window->y + 1;
    
    // Mostra o nome do arquivo e status
    char title[64];
    snprintf(title, sizeof(title), "File: %s %s", 
             text_editor.filename,
             text_editor.is_modified ? "(modified)" : "");
    memcpy(&screen_buffer[start_y][start_x], title, strlen(title));
    
    // Calcula a posição do cursor em linhas e colunas
    int cursor_line = 0;
    int cursor_col = 0;
    int pos = 0;
    
    // Conta linhas até o cursor
    while (pos < text_editor.cursor_pos) {
        if (text_editor.content[pos] == '\n') {
            cursor_line++;
            cursor_col = 0;
        } else {
            cursor_col++;
        }
        pos++;
    }
    
    // Ajusta o scroll para manter o cursor visível
    if (cursor_line < text_editor.scroll_offset) {
        text_editor.scroll_offset = cursor_line;
    }
    else if (cursor_line >= text_editor.scroll_offset + EDITOR_HEIGHT - 3) {
        text_editor.scroll_offset = cursor_line - (EDITOR_HEIGHT - 4);
    }
    
    // Mostra o conteúdo
    int content_y = start_y + 1;
    pos = 0;
    int current_line = 0;
    
    // Pula linhas até o scroll_offset
    while (pos < strlen(text_editor.content) && current_line < text_editor.scroll_offset) {
        if (text_editor.content[pos] == '\n') {
            current_line++;
        }
        pos++;
    }
    
    // Mostra as linhas visíveis
    for (int y = 0; y < EDITOR_HEIGHT - 3 && pos < strlen(text_editor.content); y++) {
        int line_start = pos;
        int line_length = 0;
        
        // Calcula o comprimento da linha atual
        while (pos < strlen(text_editor.content) && text_editor.content[pos] != '\n') {
            line_length++;
            pos++;
        }
        
        // Copia a linha para o buffer
        if (line_length > EDITOR_WIDTH - 2) {
            line_length = EDITOR_WIDTH - 2;
        }
        memcpy(&screen_buffer[content_y + y][start_x],
               &text_editor.content[line_start],
               line_length);
        
        // Se estamos na linha do cursor, mostra o cursor
        if (current_line == cursor_line && cursor_col < EDITOR_WIDTH - 2) {
            screen_buffer[content_y + y][start_x + cursor_col] = '_';
        }
        
        if (pos < strlen(text_editor.content)) {
            pos++;  // Pula o \n
            current_line++;
        }
    }
    
    // Mostra ajuda
    const char* help = "ICA: Save and Exit | Shift+E: Exit without saving";
    memcpy(&screen_buffer[start_y + EDITOR_HEIGHT - 1][start_x],
           help, strlen(help));
}

static void text_editor_handle_key(char key) {
    if (key == KEY_ICA) { // ICA para salvar e sair
        if (text_editor.is_modified) {
            fs_write(text_editor.filename, text_editor.content);
        }
        ui_close_current_window();
        return;
    }
    
    if (key == 'E' && shift_pressed) { // Shift+E para sair sem salvar
        ui_close_current_window();
        return;
    }
    
    if (key == '\b') {
        if (text_editor.cursor_pos > 0) {
            memmove(&text_editor.content[text_editor.cursor_pos - 1],
                    &text_editor.content[text_editor.cursor_pos],
                    strlen(&text_editor.content[text_editor.cursor_pos]) + 1);
            text_editor.cursor_pos--;
            text_editor.is_modified = 1;
        }
    }
    else if ((key >= ' ' && key <= '~') || key == '\n') {
        if (strlen(text_editor.content) < EDITOR_BUFFER_SIZE - 1) {
            memmove(&text_editor.content[text_editor.cursor_pos + 1],
                    &text_editor.content[text_editor.cursor_pos],
                    strlen(&text_editor.content[text_editor.cursor_pos]) + 1);
            text_editor.content[text_editor.cursor_pos] = key;
            text_editor.cursor_pos++;
            text_editor.is_modified = 1;
        }
    }
    ui_render();
}

void ui_main_loop(void) {
    while (ui_active) {
        char key = get_key();
        if (key != 0) {
            if (current_window && current_window->render == text_editor_render) {
                text_editor_handle_key(key);
            }
            else if (current_window && current_window->render == files_window_render) {
                files_handle_key(key);
            }
            else {
                if (key == KEY_ICA) {  // Usando o novo código
                    terminal_handle_key(key);
                }
                else if (key == 'w' && shift_pressed) {
                    terminal_handle_key(KEY_UP);
                } else if (key == 's' && shift_pressed) {
                    terminal_handle_key(KEY_DOWN);
                } else if (key == KEY_LEFT) {
                    ui_move_mouse(-1, 0);
                } else if (key == KEY_RIGHT) {
                    ui_move_mouse(1, 0);
                } else if (key == KEY_UP) {
                    ui_move_mouse(0, -1);
                } else if (key == KEY_DOWN) {
                    ui_move_mouse(0, 1);
                } else if (key == '\n' && shift_pressed) {
                    ui_handle_click(1);
                } else if (key == '\b' && shift_pressed) {
                    ui_handle_click(2);
                } else if (key == 'q' && shift_pressed) {
                    ui_close_current_window();
                } else if (key == 'Q') {
                    break;
                } else {
                    terminal_handle_key(key);
                }
            }
        }
    }
} 