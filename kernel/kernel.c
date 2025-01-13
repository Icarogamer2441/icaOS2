#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../drivers/ata.h"
#include "../drivers/system.h"
#include "../fs/filesystem.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../editor/editor.h"
#include "../compiler/compiler.h"

#define COMMAND_BUFFER_SIZE 256

static char cmd_output_buffer[4096];
static int cmd_output_pos = 0;
static void (*original_print_char)(char);

static void buffer_print_char(char c) {
    if (cmd_output_pos < (int)sizeof(cmd_output_buffer) - 1) {
        cmd_output_buffer[cmd_output_pos++] = c;
    }
}

void execute_command(char* command) {
    char* args = strchr(command, ' ');
    if (args) {
        *args = '\0';
        args++;
    }

    if (strcmp(command, "help") == 0) {
        print_string("Available commands:\n");
        print_string("  clear                             - Clear screen\n");
        print_string("  ls/list                           - List files and directories\n");
        print_string("  cd <dir>                          - Change directory\n");
        print_string("  mkdir <dir>                       - Create directory\n");
        print_string("  mkfile <file>                     - Create file\n");
        print_string("  rmdir <dir>                       - Remove empty directory\n");
        print_string("  rmdirf <dir>                      - Force remove directory and contents\n");
        print_string("  rmfile <file>                     - Remove file\n");
        print_string("  write <file> <txt>                - Write text to file\n");
        print_string("  read <file>                       - Read file content\n");
        print_string("  save                              - Save filesystem to disk\n");
        print_string("  load                              - Load filesystem from disk\n");
        print_string("  pwd                               - Print working directory\n");
        print_string("  reboot                            - Restart the system\n");
        print_string("  shutdown                          - Power off the system\n");
        print_string("  editmode <file>                   - Edit file in multiline mode\n");
        print_string("  run <file.icl>                    - Run ICA program\n");
        print_string("  helpicl                           - ICA programming language help\n");
        print_string("  help2                             - Show detailed command help\n");
        print_string("  filecmd <cmd> [args....] > <file> - Run command and save output to file\n");
        print_string("\nType 'help2' for detailed help and examples.\n");
    }
    else if (strcmp(command, "clear") == 0) {
        clear_screen();
    }
    else if (strcmp(command, "ls") == 0 || strcmp(command, "list") == 0) {
        fs_list();
    }
    else if (strcmp(command, "cd") == 0 && args) {
        if (fs_cd(args) < 0) {
            print_string("Directory not found\n");
        }
    }
    else if (strcmp(command, "mkdir") == 0 && args) {
        if (fs_mkdir(args) < 0) {
            print_string("Error creating directory\n");
        }
    }
    else if (strcmp(command, "mkfile") == 0 && args) {
        if (fs_mkfile(args) < 0) {
            print_string("Error creating file\n");
        }
    }
    else if (strcmp(command, "rmdir") == 0 && args) {
        if (fs_rmdir(args) < 0) {
            print_string("Error removing directory\n");
        }
    }
    else if (strcmp(command, "rmfile") == 0 && args) {
        if (fs_rmfile(args) < 0) {
            print_string("Error removing file\n");
        }
    }
    else if (strcmp(command, "rmdirf") == 0 && args) {
        print_string("Are you sure you want to remove '");
        print_string(args);
        print_string("' and all its contents? (y/n): ");
        
        while(1) {
            char confirm = get_key();
            if (confirm == 'y' || confirm == 'Y' || confirm == 'n' || confirm == 'N') {
                print_char(confirm);
                print_string("\n");
                if (confirm == 'y' || confirm == 'Y') {
                    if (fs_rmdirf(args) < 0) {
                        print_string("Error removing directory\n");
                    }
                }
                break;
            }
        }
    }
    else if (strcmp(command, "pwd") == 0) {
        char* path = fs_get_current_path();
        print_string(path);
        print_string("\n");
    }
    else if (strcmp(command, "write") == 0 && args) {
        char* filename = args;
        char* content = strchr(args, ' ');
        if (content) {
            *content = '\0';
            content++;
            if (fs_write(filename, content) < 0) {
                print_string("Error writing to file\n");
            }
        } else {
            print_string("Usage: write <filename> <content>\n");
        }
    }
    else if (strcmp(command, "read") == 0 && args) {
        char* content = fs_read(args);
        if (content) {
            print_string(content);
            print_string("\n");
        } else {
            print_string("File not found\n");
        }
    }
    else if (strcmp(command, "save") == 0) {
        fs_save(NULL);
    }
    else if (strcmp(command, "load") == 0) {
        fs_load(NULL);
    }
    else if (strcmp(command, "reboot") == 0) {
        print_string("Are you sure you want to reboot? (y/n): ");
        while(1) {
            char confirm = get_key();
            if (confirm == 'y' || confirm == 'Y' || confirm == 'n' || confirm == 'N') {
                print_char(confirm);
                print_string("\n");
                if (confirm == 'y' || confirm == 'Y') {
                    print_string("Rebooting system...\n");
                    system_reboot();
                }
                break;
            }
        }
    }
    else if (strcmp(command, "shutdown") == 0) {
        print_string("Are you sure you want to shutdown? (y/n): ");
        while(1) {
            char confirm = get_key();
            if (confirm == 'y' || confirm == 'Y' || confirm == 'n' || confirm == 'N') {
                print_char(confirm);
                print_string("\n");
                if (confirm == 'y' || confirm == 'Y') {
                    print_string("Shutting down system...\n");
                    system_shutdown();
                }
                break;
            }
        }
    }
    else if (strcmp(command, "run") == 0 && args) {
        // Verifica se o arquivo existe
        char* content = fs_read(args);
        if (!content) {
            print_string("Error: Could not read file\n");
            return;
        }
        
        if (run_program(args) == 0) {
            print_string("\nProgram finished successfully\n");
        }
    }
    else if (strcmp(command, "editmode") == 0 && args) {
        // Lê o conteúdo atual do arquivo se ele existir
        char* current_content = fs_read(args);
        Editor editor;
        
        if (current_content) {
            print_string("Current file content:\n");
            print_string(current_content);
            print_string("\nDo you want to (c)lear the file or (a)ppend to it? ");
            
            while(1) {
                char choice = get_key();
                if (choice == 'c' || choice == 'C') {
                    editor_init(&editor, NULL);
                    break;
                }
                else if (choice == 'a' || choice == 'A') {
                    editor_init(&editor, current_content);
                    break;
                }
            }
        } else {
            editor_init(&editor, NULL);
        }
        
        print_string("\nEntering edit mode (press ESC to finish):\n");
        editor_display(&editor);
        
        while(1) {
            char input = get_key();
            if (input != 0) {
                if (editor_handle_key(&editor, input)) {
                    // ESC ou ICA foi pressionado
                    if (editor.is_modified) {
                        if (input == EDITOR_KEY_ICA) {
                            print_string("\nICA key pressed. ");
                            print_string("(s)ave and exit, (e)xit without saving, or (c)ontinue editing? ");
                            
                            while(1) {
                                char choice = get_key();
                                if (choice == 's' || choice == 'S') {
                                    // Salva o arquivo
                                    if (fs_write(args, editor_get_content(&editor)) < 0) {
                                        print_string("\nError saving file!\n");
                                    } else {
                                        print_string("\nFile saved successfully!\n");
                                    }
                                    editor_free(&editor);
                                    return;
                                }
                                else if (choice == 'e' || choice == 'E') {
                                    print_string("\nExiting without saving...\n");
                                    editor_free(&editor);
                                    return;
                                }
                                else if (choice == 'c' || choice == 'C') {
                                    editor_display(&editor);
                                    break;
                                }
                            }
                        } else {
                            // ESC foi pressionado
                            print_string("\nDo you want to (s)ave and exit or (c)ontinue editing? ");
                            
                            while(1) {
                                char choice = get_key();
                                if (choice == 's' || choice == 'S') {
                                    // Salva o arquivo
                                    if (fs_write(args, editor_get_content(&editor)) < 0) {
                                        print_string("\nError saving file!\n");
                                    } else {
                                        print_string("\nFile saved successfully!\n");
                                    }
                                    editor_free(&editor);
                                    return;
                                }
                                else if (choice == 'c' || choice == 'C') {
                                    editor_display(&editor);
                                    break;
                                }
                            }
                        }
                    } else {
                        editor_free(&editor);
                        return;
                    }
                } else {
                    // Atualiza a tela
                    editor_display(&editor);
                }
            }
        }
    }
    else if (strcmp(command, "helpicl") == 0) {
        print_string("ICA Language (ICL) Help:\n");
        print_string("======================\n\n");
        
        print_string("1. Printing:\n");
        print_string("   print \"Your text here\"    - Print text\n");
        print_string("   newline                  - Print a line break\n\n");
        
        print_string("2. Variables:\n");
        print_string("   var name = \"John\"        - Create string variable\n");
        print_string("   var age = 25            - Create number variable\n");
        print_string("   printvar name           - Print variable value\n\n");
        
        print_string("Example Program:\n");
        print_string("---------------\n");
        print_string("print \"What's your name? \"\n");
        print_string("var name = \"John\"\n");
        print_string("print \"Hello, \"\n");
        print_string("printvar name\n");
        print_string("newline\n");
        print_string("var age = 25\n");
        print_string("print \"Age: \"\n");
        print_string("printvar age\n\n");
        
        print_string("How to Run:\n");
        print_string("-----------\n");
        print_string("1. Create a file:     mkfile program.icl\n");
        print_string("2. Edit the file:     editmode program.icl\n");
        print_string("3. Run the program:   run program.icl\n");
    }
    else if (strcmp(command, "filecmd") == 0 && args) {
        // Extrai o comando e o nome do arquivo
        char* cmd = args;
        char* filename = strchr(args, ' ');
        
        if (!filename) {
            print_string("Usage: filecmd <command> [args...] > output.txt\n");
            return;
        }
        
        // Procura pelo '>'
        char* redirect = strchr(filename, '>');
        if (!redirect) {
            print_string("Usage: filecmd <command> [args...] > output.txt\n");
            return;
        }
        
        // Separa o comando do arquivo de saída
        *redirect = '\0';
        redirect++;
        while (*redirect == ' ') redirect++;
        
        // Cria o arquivo de saída
        if (fs_mkfile(redirect) < 0) {
            // Se já existe, não é erro
        }
        
        // Salva a posição atual do cursor
        int old_cursor_x = cursor_x;
        int old_cursor_y = cursor_y;
        
        // Reseta o buffer de saída
        cmd_output_pos = 0;
        memset(cmd_output_buffer, 0, sizeof(cmd_output_buffer));
        
        // Redireciona a saída para o buffer
        original_print_char = print_char;
        print_char = buffer_print_char;
        
        // Executa o comando
        execute_command(cmd);
        
        // Restaura a função original de print
        print_char = original_print_char;
        
        // Adiciona terminador nulo
        cmd_output_buffer[cmd_output_pos] = '\0';
        
        // Salva a saída no arquivo
        fs_write(redirect, cmd_output_buffer);
        
        // Restaura o cursor
        cursor_x = old_cursor_x;
        cursor_y = old_cursor_y;
        set_cursor(cursor_x, cursor_y);
    }
    else if (strcmp(command, "help2") == 0) {
        print_string("Detailed Command Help:\n");
        print_string("===================\n\n");
        
        print_string("File Operations Guide:\n");
        print_string("-------------------\n");
        print_string("  - Use .. to go to parent directory: cd ..\n");
        print_string("  - Paths can be relative or absolute\n");
        print_string("  - rmdirf will remove directory and all contents\n");
        print_string("  - write command will overwrite existing content\n");
        print_string("  - save/load commands persist filesystem to disk\n\n");
        
        print_string("File Management:\n");
        print_string("-----------------\n");
        print_string("mkdir <dir>        - Create a new directory\n");
        print_string("rmdir <dir>        - Remove an empty directory\n");
        print_string("rmdirf <dir>       - Force remove directory and all contents\n");
        print_string("mkfile <file>      - Create a new empty file\n");
        print_string("rmfile <file>      - Remove a file\n");
        print_string("write <file> <txt> - Write text to a file\n");
        print_string("read <file>        - Display file contents\n\n");
        
        print_string("Navigation:\n");
        print_string("-----------\n");
        print_string("cd <dir>           - Change current directory\n");
        print_string("pwd                - Show current directory path\n");
        print_string("ls/list            - List files and directories\n\n");
        
        print_string("System:\n");
        print_string("--------\n");
        print_string("clear              - Clear the screen\n");
        print_string("reboot             - Restart the system\n");
        print_string("shutdown           - Power off the system\n");
        print_string("save               - Save filesystem to disk\n");
        print_string("load               - Load filesystem from disk\n\n");
        
        print_string("Editor & Programming:\n");
        print_string("--------------------\n");
        print_string("editmode <file>    - Open file in text editor\n");
        print_string("run <file.icl>     - Run an ICA program\n");
        print_string("helpicl            - Show ICA language help\n\n");
        
        print_string("Utilities:\n");
        print_string("----------\n");
        print_string("help               - Show basic command list\n");
        print_string("help2              - Show this detailed help\n");
        print_string("filecmd <cmd> > <file> - Run command and save output to file\n\n");
        
        print_string("Examples:\n");
        print_string("---------\n");
        print_string("mkdir Documents\n");
        print_string("cd Documents\n");
        print_string("mkfile notes.txt\n");
        print_string("write notes.txt Hello World!\n");
        print_string("filecmd help > help.txt\n");
        print_string("editmode program.icl\n");
        
        print_string("Editor Commands:\n");
        print_string("--------------\n");
        print_string("  ESC              - Open save/exit prompt\n");
        print_string("  ICA key (Win)    - Open extended options menu\n");
        print_string("  Backspace        - Delete previous character\n");
        print_string("  Enter            - Insert new line\n");
        print_string("  Up/Down          - Scroll through content\n\n");
        
        print_string("Editor Options:\n");
        print_string("  - ESC shows basic save/continue options\n");
        print_string("  - ICA key shows save/exit/continue options\n");
        print_string("  - Content is auto-saved when choosing save\n");
        print_string("  - Exit without save discards changes\n\n");
    }
    else {
        print_string("Unknown command. Type 'help' for available commands.\n");
    }
}

void kernel_main() {
    char command_buffer[COMMAND_BUFFER_SIZE];
    int buffer_pos = 0;
    
    clear_screen();
    print_string("Welcome to icaOS2!\n");
    
    // Initialize drivers
    ata_init();
    fs_init();
    
    while(1) {
        print_string("icaOS2$ ");
        buffer_pos = 0;
        
        while(1) {
            char input = get_key();
            if (input != 0) {
                if (input == KEY_UP) {
                    scroll_up();
                    continue;
                }
                else if (input == KEY_DOWN) {
                    scroll_down();
                    continue;
                }
                
                if (input == '\n') {
                    command_buffer[buffer_pos] = '\0';
                    print_char('\n');
                    execute_command(command_buffer);
                    break;
                }
                else if (input == '\b') {
                    if (buffer_pos > 0) {
                        buffer_pos--;
                        cursor_x--;
                        print_char(' ');
                        cursor_x--;
                        set_cursor(cursor_x, cursor_y);
                    }
                }
                else if (buffer_pos < COMMAND_BUFFER_SIZE - 1) {
                    command_buffer[buffer_pos++] = input;
                    print_char(input);
                }
            }
        }
    }
} 