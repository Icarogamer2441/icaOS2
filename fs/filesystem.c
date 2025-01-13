#include "filesystem.h"
#include "../drivers/screen.h"
#include "../lib/string.h"

fs_node_t fs_nodes[MAX_FILES];
int current_dir = 0;
int fs_size = 0;

#define FS_MAGIC 0x1CA052  // Magic number para identificar o sistema de arquivos (0x1CA052 = icaOS2)
#define FS_SECTOR_START 1   // Setor onde começa o sistema de arquivos

void fs_init() {
    // Cria diretório root
    strcpy(fs_nodes[0].name, "/");
    fs_nodes[0].is_directory = 1;
    fs_nodes[0].parent_dir = 0;
    fs_size = 1;
}

int fs_mkdir(const char* name) {
    if (fs_size >= MAX_FILES) return -1;
    
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir) {
            return -1;
        }
    }
    
    strcpy(fs_nodes[fs_size].name, name);
    fs_nodes[fs_size].is_directory = 1;
    fs_nodes[fs_size].parent_dir = current_dir;
    fs_size++;
    return 0;
}

int fs_mkfile(const char* name) {
    if (fs_size >= MAX_FILES) return -1;
    
    strcpy(fs_nodes[fs_size].name, name);
    fs_nodes[fs_size].is_directory = 0;
    fs_nodes[fs_size].parent_dir = current_dir;
    fs_nodes[fs_size].size = 0;
    fs_size++;
    return 0;
}

void fs_list() {
    print_string("Contents of ");
    print_string(fs_get_current_path());
    print_string(":\n");
    
    int count = 0;
    for (int i = 0; i < fs_size; i++) {
        if (fs_nodes[i].parent_dir == current_dir) {
            if (fs_nodes[i].is_directory) {
                print_string("[DIR]  ");
            } else {
                print_string("[FILE] ");
            }
            print_string(fs_nodes[i].name);
            if (!fs_nodes[i].is_directory) {
                print_string(" (");
                // TODO: Adicionar função para converter int para string
                // print_int(fs_nodes[i].size);
                print_string(" bytes)");
            }
            print_string("\n");
            count++;
        }
    }
    
    if (count == 0) {
        print_string("Directory is empty\n");
    } else {
        print_string("\nTotal: ");
        // TODO: Adicionar função para converter int para string
        // print_int(count);
        print_string(" items\n");
    }
}

int fs_cd(const char* path) {
    if (strcmp(path, "..") == 0) {
        if (current_dir != 0) {
            current_dir = fs_nodes[current_dir].parent_dir;
        }
        return 0;
    }
    
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, path) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            fs_nodes[i].is_directory) {
            current_dir = i;
            return 0;
        }
    }
    return -1;
}

int fs_write(const char* name, const char* content) {
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            !fs_nodes[i].is_directory) {
            
            size_t content_len = strlen(content);
            if (content_len >= MAX_FILE_SIZE) {
                print_string("Error: Content too large for file\n");
                return -1;
            }
            
            memcpy(fs_nodes[i].content, content, content_len + 1); // +1 para incluir o \0
            fs_nodes[i].size = content_len;
            return 0;
        }
    }
    return -1;
}

char* fs_read(const char* name) {
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            !fs_nodes[i].is_directory) {
            return fs_nodes[i].content;
        }
    }
    return 0;
}

int fs_rmdir(const char* name) {
    // Não permite remover o diretório raiz
    if (current_dir == 0 && strcmp(name, "/") == 0) {
        return -1;
    }
    
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            fs_nodes[i].is_directory) {
            
            // Verifica se o diretório está vazio
            int is_empty = 1;
            for (int j = 0; j < fs_size; j++) {
                if (fs_nodes[j].parent_dir == i) {
                    is_empty = 0;
                    break;
                }
            }
            
            if (!is_empty) {
                print_string("Error: Directory not empty\n");
                return -1;
            }
            
            // Remove o diretório
            for (int j = i; j < fs_size - 1; j++) {
                fs_nodes[j] = fs_nodes[j + 1];
            }
            
            // Atualiza os parent_dir que são maiores que o índice removido
            for (int j = 0; j < fs_size; j++) {
                if (fs_nodes[j].parent_dir > i) {
                    fs_nodes[j].parent_dir--;
                }
            }
            
            fs_size--;
            return 0;
        }
    }
    
    print_string("Error: Directory not found\n");
    return -1;
}

int fs_rmfile(const char* name) {
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            !fs_nodes[i].is_directory) {
            
            // Remove movendo todos os nós seguintes uma posição para trás
            for (int j = i; j < fs_size - 1; j++) {
                fs_nodes[j] = fs_nodes[j + 1];
                // Atualiza os parent_dir que apontavam para nós que foram movidos
                for (int k = 0; k < fs_size; k++) {
                    if (fs_nodes[k].parent_dir > j) {
                        fs_nodes[k].parent_dir--;
                    }
                }
            }
            fs_size--;
            return 0;
        }
    }
    return -1;
}

static char path_buffer[MAX_PATH];

char* fs_get_current_path() {
    int path_stack[MAX_DIRS];
    int stack_size = 0;
    int current = current_dir;
    
    // Constrói o caminho do atual até a raiz
    while (current != 0) {
        path_stack[stack_size++] = current;
        current = fs_nodes[current].parent_dir;
    }
    
    // Monta o caminho completo
    strcpy(path_buffer, "/");
    
    // Adiciona os diretórios do caminho na ordem correta
    for (int i = stack_size - 1; i >= 0; i--) {
        strcat(path_buffer, fs_nodes[path_stack[i]].name);
        if (i > 0) strcat(path_buffer, "/");
    }
    
    return path_buffer;
}

void fs_save(const char* filename) {
    (void)filename; // Evita warning de parâmetro não usado
    
    // Salva o magic number
    unsigned int magic = FS_MAGIC;
    
    // Buffer para um setor (512 bytes)
    unsigned char sector[512];
    
    // Primeiro setor: magic number e número de nós
    memcpy(sector, &magic, sizeof(magic));
    memcpy(sector + sizeof(magic), &fs_size, sizeof(fs_size));
    
    // Escreve o primeiro setor
    if(ata_write_sectors(FS_SECTOR_START, 1, sector) < 0) {
        print_string("Error saving filesystem header\n");
        return;
    }
    
    // Calcula quantos setores precisamos para os nós
    int sectors_needed = (fs_size * sizeof(fs_node_t) + 511) / 512;
    
    // Escreve os nós
    if(ata_write_sectors(FS_SECTOR_START + 1, sectors_needed, fs_nodes) < 0) {
        print_string("Error saving filesystem nodes\n");
        return;
    }
    
    print_string("Filesystem saved successfully\n");
}

void fs_load(const char* filename) {
    (void)filename; // Evita warning de parâmetro não usado
    
    // Buffer para um setor (512 bytes)
    unsigned char sector[512];
    unsigned int magic;
    
    // Lê o primeiro setor
    if(ata_read_sectors(FS_SECTOR_START, 1, sector) < 0) {
        print_string("Error reading filesystem header\n");
        return;
    }
    
    // Verifica o magic number
    memcpy(&magic, sector, sizeof(magic));
    if(magic != FS_MAGIC) {
        print_string("Invalid or not found filesystem\n");
        return;
    }
    
    // Lê o número de nós
    memcpy(&fs_size, sector + sizeof(magic), sizeof(fs_size));
    
    // Calcula quantos setores precisamos ler
    int sectors_needed = (fs_size * sizeof(fs_node_t) + 511) / 512;
    
    // Lê os nós
    if(ata_read_sectors(FS_SECTOR_START + 1, sectors_needed, fs_nodes) < 0) {
        print_string("Error reading filesystem nodes\n");
        return;
    }
    
    print_string("Filesystem loaded successfully\n");
}

// Função auxiliar para remover recursivamente
static void remove_recursive(int dir_index) {
    // Primeiro remove todos os arquivos e subdiretórios
    for (int i = 0; i < fs_size; i++) {
        if (fs_nodes[i].parent_dir == dir_index) {
            if (fs_nodes[i].is_directory) {
                remove_recursive(i);
                i--; // Ajusta o índice pois o array foi modificado
            } else {
                // Remove o arquivo
                for (int j = i; j < fs_size - 1; j++) {
                    fs_nodes[j] = fs_nodes[j + 1];
                }
                fs_size--;
                i--; // Ajusta o índice pois o array foi modificado
            }
        }
    }
    
    // Agora remove o próprio diretório
    for (int i = dir_index; i < fs_size - 1; i++) {
        fs_nodes[i] = fs_nodes[i + 1];
    }
    
    // Atualiza os parent_dir que são maiores que o índice removido
    for (int j = 0; j < fs_size; j++) {
        if (fs_nodes[j].parent_dir > dir_index) {
            fs_nodes[j].parent_dir--;
        }
    }
    
    fs_size--;
}

// Função para forçar a remoção de um diretório e todo seu conteúdo
int fs_rmdirf(const char* name) {
    // Não permite remover o diretório raiz
    if (current_dir == 0 && strcmp(name, "/") == 0) {
        print_string("Error: Cannot remove root directory\n");
        return -1;
    }
    
    for (int i = 0; i < fs_size; i++) {
        if (strcmp(fs_nodes[i].name, name) == 0 && 
            fs_nodes[i].parent_dir == current_dir &&
            fs_nodes[i].is_directory) {
            
            remove_recursive(i);
            print_string("Directory and all contents removed successfully\n");
            return 0;
        }
    }
    
    print_string("Error: Directory not found\n");
    return -1;
} 