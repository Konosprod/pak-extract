#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <string.h>

#define PATH_SIZE 0x40
#define SIZE_SIZE 0x04
#define OFFSET_SIZE 0x04

typedef struct
{
    int size;
    int offset;
    char path[40];
}file;

static file init_files;

int nombre_dossier(char chaine[])
{
    int i = 0;
    int nb = 0;
    
    while(chaine[i] != '\0')
    {
        if(chaine[i] == '/')
        {
            nb++;
        }
        i++;
    }
    return nb;
}

void make_dirs(file* files, char* out)
{
    char temp[250] = {0};
    int i = 0;
    int nb = 0;
    char* dir = NULL;
    char path[250] = {0};
    DIR* directory = NULL;
    
    sprintf(temp, "%s", files->path);
    strcat(path, out);
    
    dir = strtok(temp, "/");
    
    if(dir != NULL)
    {
    
        nb = nombre_dossier(files->path);
    
        for(i = 0; i < nb; i++)
        {
            strcat(path, dir);
            directory = opendir(path);
            if(!directory)
            {
                mkdir(path, 0777);
                closedir(directory);
            }
            else
            {
                closedir(directory);
            }
            
            dir = strtok(NULL, "/");
        }
    }
}


void dump_file(FILE* pak, file* files, char* dirout)
{
    FILE* out = NULL;
    int i = 0;
    int a = 0;
    long pos = 0;
    char dir[250] = {0};
    
    strcat(dir, dirout);
    strcat(dir, files->path);
    out = fopen(dir, "wb+");
    
    if(!out)
    {
        printf("Unable to create %s\n", dir);
        exit(-1);
    }
    
    pos = ftell(pak);
    fseek(pak, files->offset, SEEK_SET);
    
    while(a != files->size)
    {
        fread(&i, sizeof(char), 1, pak);
        fwrite(&i, sizeof(char), 1, out);
        a++;
    }
    
    fclose(out);
    fseek(pak, pos, SEEK_SET);
}

void get_index(FILE* pak, int token, char* out)
{
    int i = 0;
    file* files = NULL;
    
    files = calloc(token+1, sizeof(file));
    
    if(!files)
    {
        printf("Unable to alloc files !\n");
        exit(-1);
    }
    
    fseek(pak, 4, SEEK_SET);
    
    for(i = 0; i < token; i++) 
    {
        files[i] = init_files;
        fread(&files[i].path, sizeof(char)*PATH_SIZE, 1, pak);
        fread(&files[i].offset, sizeof(char)*OFFSET_SIZE,  1, pak);
        fread(&files[i].size, sizeof(char)*SIZE_SIZE, 1, pak);
        printf("Path[%d]: %s Offset: 0x%.8X Size: %d octets\n", i+1, files[i].path, files[i].offset, files[i].size);
        make_dirs(&files[i], out);
        dump_file(pak, &files[i], out);
    }
    
    free(files);
}

void print_help(void)
{
    printf("\nUtilisation:\n\n");
    printf("\t./extract-pak [PAK FILE] [DIRECTORY]\n");
    printf("\n\tExemple: ./extract-pak data.pak Out/\n");
    printf("\tThe directroy must finish by a \"/\" caractere !\n");
}

int main(int argc, char* argv[])
{
    FILE* pak = NULL;
    int token = 0;
    DIR* dir = NULL;
    
    if(argc != 3)
    {
        print_help();
        exit(-1);
    }
    else
    {
        pak = fopen(argv[1], "rb+");
        fread(&token, sizeof(int), 1, pak);
        printf("File number:\t%d\n", token);
        dir = opendir(argv[2]);
        if(!dir)
        {
            mkdir(argv[2], 0777);
        }
        if(argv[2][strlen(argv[2])-1] != '/')
        {
            printf("You must specifie a directory ! Directory_out/ !\n\n");
            exit(-1);
        }
        get_index(pak, token, argv[2]);
        closedir(dir);
    }
    
    return 0;
}
        
