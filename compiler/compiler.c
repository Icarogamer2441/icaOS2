#include "compiler.h"
#include "../drivers/screen.h"
#include "../fs/filesystem.h"
#include "../lib/string.h"
#include "../lib/memory.h"
#include "../lib/ctype.h"

#define MAX_VARS 100

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

// Função auxiliar para converter string para inteiro
static int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

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
}

// Função principal que interpreta o arquivo
int run_program(const char* program_file) {
    char* source = fs_read(program_file);
    if (!source) {
        print_string("Error: Could not read source file\n");
        return -1;
    }
    
    var_count = 0; // Reset variables
    
    // Interpreta linha por linha
    char* line = source;
    char* next_line;
    
    while (line && *line) {
        // Encontra o fim da linha
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }
        
        interpret_line(line);
        line = next_line;
    }
    
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