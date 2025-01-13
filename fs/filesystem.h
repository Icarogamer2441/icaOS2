#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../drivers/ata.h"

#define MAX_FILENAME 32
#define MAX_FILES 100
#define MAX_DIRS 100
#define MAX_PATH 256
#define MAX_FILE_SIZE 4096

typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_FILE_SIZE];
    int size;
    int is_directory;
    int parent_dir;
} fs_node_t;

// Sistema de arquivos
extern fs_node_t fs_nodes[MAX_FILES];
extern int current_dir;
extern int fs_size;

// Funções do sistema de arquivos
void fs_init();
int fs_mkdir(const char* name);
int fs_rmdir(const char* name);
int fs_rmdirf(const char* name);
int fs_mkfile(const char* name);
int fs_rmfile(const char* name);
int fs_cd(const char* path);
void fs_list();
int fs_write(const char* name, const char* content);
char* fs_read(const char* name);
void fs_save(const char* filename);
void fs_load(const char* filename);
char* fs_get_current_path();

#endif 