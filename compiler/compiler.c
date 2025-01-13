#include "compiler.h"
#include "../drivers/screen.h"
#include "../fs/filesystem.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../lib/ctype.h"

#define MAX_VARS 100
#define MAX_INPUT_SIZE 256

typedef struct {
    char name[32];
    enum { VAR_STRING, VAR_INT } type;
    union {
        char* str_value;
        int int_value;
    };
} Variable;

static Variable variables[MAX_VARS];
static int var_count = 0;

// Função para encontrar uma variável pelo nome
static Variable* find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Função para criar ou atualizar uma variável
static void set_variable(const char* name, const char* value, int is_number) {
    Variable* var = find_variable(name);
    if (!var && var_count < MAX_VARS) {
        var = &variables[var_count++];
        strcpy(var->name, name);
    }
    
    if (var) {
        if (is_number) {
            var->type = VAR_INT;
            var->int_value = atoi(value);
        } else {
            var->type = VAR_STRING;
            var->str_value = malloc(strlen(value) + 1);
            strcpy(var->str_value, value);
        }
    }
}

// Adicione função para limpar variáveis
static void clear_variables() {
    for (int i = 0; i < var_count; i++) {
        if (variables[i].type == VAR_STRING && variables[i].str_value) {
            free(variables[i].str_value);
            variables[i].str_value = NULL;  // Evita double-free
        }
    }
    var_count = 0;
    memset(variables, 0, sizeof(variables));  // Limpa completamente o array
}

// Modifique read_user_input para ter duas versões
static char* read_user_input_str() {
    static char input_buffer[MAX_INPUT_SIZE];
    int pos = 0;
    
    while (1) {
        char c = get_key();
        if (c != 0) {  // Verifica se uma tecla foi realmente pressionada
            if (c == '\n') {
                input_buffer[pos] = '\0';
                print_char('\n');
                break;
            } else if (c == '\b' || c == 127) {  // Backspace ou Delete
                if (pos > 0) {
                    pos--;
                    // Move o cursor para trás e apaga o caractere anterior
                    cursor_x--;  // Move o cursor para trás
                    print_char(' ');  // Apaga o caractere
                    cursor_x--;  // Move o cursor de volta
                    set_cursor(cursor_x, cursor_y);  // Atualiza a posição do cursor
                }
            } else if (c >= ' ' && pos < MAX_INPUT_SIZE - 1) {
                input_buffer[pos++] = c;
                print_char(c);
            }
        }
    }
    return input_buffer;
}

// Interpreta uma linha do programa
static void interpret_line(const char* line) {
    // Pula espaços em branco
    while (*line == ' ' || *line == '\t') line++;
    
    // Linha vazia ou comentário
    if (*line == '\0' || *line == '#') return;
    
    // Comando print
    if (strncmp(line, "print", 5) == 0 && (!isalnum(line[5]))) {
        line += 5;
        while (*line == ' ' || *line == '\t') line++;
        
        if (*line == '"') {
            line++; // Pula a primeira aspas
            const char* end = strchr(line, '"');
            if (end) {
                int len = end - line;
                char buffer[256];
                strncpy(buffer, line, len);
                buffer[len] = '\0';
                print_string(buffer);
            }
        }
        return;
    }
    
    // Comando newline
    if (strncmp(line, "newline", 7) == 0 && (!isalnum(line[7]))) {
        print_string("\n");
        return;
    }
    
    // Comando var
    if (strncmp(line, "var", 3) == 0 && (!isalnum(line[3]))) {
        line += 3;
        while (*line == ' ' || *line == '\t') line++;
        
        // Pega o nome da variável
        const char* name_start = line;
        while (isalnum(*line)) line++;
        int name_len = line - name_start;
        
        // Pula espaços até o =
        while (*line == ' ' || *line == '\t') line++;
        if (*line++ != '=') return;
        while (*line == ' ' || *line == '\t') line++;
        
        // Pega o valor
        char name[32];
        strncpy(name, name_start, name_len);
        name[name_len] = '\0';
        
        if (*line == '"') { // String
            line++;
            const char* end = strchr(line, '"');
            if (end) {
                int len = end - line;
                char value[256];
                strncpy(value, line, len);
                value[len] = '\0';
                set_variable(name, value, 0);
            }
        } else { // Número
            set_variable(name, line, 1);
        }
        return;
    }
    
    // Comando printvar
    if (strncmp(line, "printvar", 8) == 0 && (!isalnum(line[8]))) {
        line += 8;
        while (*line == ' ' || *line == '\t') line++;
        
        char var_name[32];
        int i = 0;
        while (isalnum(*line)) {
            var_name[i++] = *line++;
        }
        var_name[i] = '\0';
        
        Variable* var = find_variable(var_name);
        if (var) {
            if (var->type == VAR_STRING) {
                print_string(var->str_value);
            } else {
                char num_str[12];
                int_to_string(var->int_value, num_str);
                print_string(num_str);
            }
        }
        return;
    }
    
    // Comando input (para strings)
    if (strncmp(line, "input", 5) == 0 && (!isalnum(line[5]))) {
        line += 5;
        while (*line == ' ' || *line == '\t') line++;
        
        // Verifica se tem mensagem de prompt
        if (*line == '"') {
            line++; // Pula a primeira aspas
            const char* end = strchr(line, '"');
            if (end) {
                int len = end - line;
                char buffer[256];
                strncpy(buffer, line, len);
                buffer[len] = '\0';
                print_string(buffer);
            }
            line = end + 1;
        }
        
        // Lê o input do usuário
        char* user_input = read_user_input_str();
        
        // Verifica se tem variável para armazenar
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '>') {
            line++;
            while (*line == ' ' || *line == '\t') line++;
            
            // Pega o nome da variável
            const char* var_start = line;
            while (isalnum(*line)) line++;
            int var_len = line - var_start;
            
            if (var_len > 0) {
                char var_name[32];
                strncpy(var_name, var_start, var_len);
                var_name[var_len] = '\0';
                
                // Verifica se é número ou string
                char* p = user_input;
                int is_number = 1;
                
                // Pula espaços iniciais
                while (*p == ' ') p++;
                
                // Verifica sinal
                if (*p == '-' || *p == '+') p++;
                
                // Verifica se tem apenas dígitos
                if (*p == '\0') is_number = 0;  // String vazia
                while (*p && is_number) {
                    if (*p < '0' || *p > '9') {
                        is_number = 0;
                    }
                    p++;
                }
                
                set_variable(var_name, user_input, is_number);
            }
        }
        return;
    }
}

// Função principal que interpreta o arquivo
int run_program(const char* program_file) {
    char* source = fs_read(program_file);
    if (!source) {
        print_string("Error: Could not read source file\n");
        return -1;
    }
    
    // Faz uma cópia do código fonte para não modificar o original
    char* program = malloc(strlen(source) + 1);
    strcpy(program, source);
    
    // Limpa variáveis antes de executar
    clear_variables();
    
    // Interpreta linha por linha
    char* line = program;
    char* next_line;
    
    while (line && *line) {
        // Encontra o fim da linha
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';  // Temporariamente marca o fim da linha
            next_line++;        // Avança para a próxima linha
        }
        
        interpret_line(line);
        
        line = next_line;
    }
    
    // Libera a memória
    free(program);  // Libera a cópia do programa
    free(source);   // Libera o código fonte original
    
    return 0;
}

// Função mantida para compatibilidade, agora apenas copia o arquivo
int compile_file(const char* input_file, const char* output_file) {
    char* source = fs_read(input_file);
    if (!source) {
        print_string("Error: Could not read source file\n");
        return -1;
    }
    
    if (fs_write(output_file, source) < 0) {
        print_string("Error: Could not write output file\n");
        return -1;
    }
    
    return 0;
} 