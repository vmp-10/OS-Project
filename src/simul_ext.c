#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "headers.h"

#define COMMAND_LENGTH 100

// Function prototypes
void PrintByteMaps(EXT_BYTE_MAPS *ext_bytemaps);
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2);
void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup);
int FindFile(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *name);
void ListDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes);
int Rename(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *oldName, char *newName);
int Print(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, EXT_DATOS *memData, char *name);
int Delete(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
               EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
               char *name, FILE *file);
int Copy(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
             EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
             EXT_DATOS *memData, char *srcName, char *destName, FILE *file);
void SaveInodesAndDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, FILE *file);
void SaveByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *file);
void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *file);
void SaveData(EXT_DATOS *memData, FILE *file);

int main()
{
    char *command[COMMAND_LENGTH];
    char *cmd[COMMAND_LENGTH];
    char *arg1[COMMAND_LENGTH];
    char *arg2[COMMAND_LENGTH];
    
    int i, j;
    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODES ext_blq_inodos;
    EXT_ENTRY_DIR directory[MAX_FILES];
    EXT_DATOS memData[MAX_DATA_BLOCKS];
    EXT_DATOS fileData[MAX_PARTITION_BLOCKS];
    int dirEntry;
    int saveData;
    FILE *file;
        
    // Open the binary partition file
    file = fopen("partition.bin", "r+b");
    if (!file) {
      perror("Error opening partition file");
      return 1;
    }    
    
    // Read the entire file in one go
    fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);    
    
    // Map binary file sections into respective structures
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
    memcpy(&directory, (EXT_ENTRY_DIR *)&fileData[3], BLOCK_SIZE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODES *)&fileData[1], BLOCK_SIZE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODES *)&fileData[2], BLOCK_SIZE);
    memcpy(&memData, (EXT_DATOS *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);
    
    // Command processing loop
    while (1) {
      do {
        printf(">> ");
        fflush(stdin);
        fgets(command, COMMAND_LENGTH, stdin);
      } while (CheckCommand(command, cmd, arg1, arg2) != 0);

      // Process the "dir" command
      if (strcmp(cmd, "dir") == 0) {
        ListDirectory(directory, &ext_blq_inodos);
        continue;
      }

      // Handle other commands like rename, delete, copy, etc.


      // Save metadata after modifying the filesystem
      SaveInodesAndDirectory(directory, &ext_blq_inodos, file);
      SaveByteMaps(&ext_bytemaps, file);
      SaveSuperBlock(&ext_superblock, file);

      if (saveData) {
        SaveData(memData, file);
      }

      saveData = 0; // Reset saveData flag after saving

      // Handle the "exit" command
      if (strcmp(cmd, "exit") == 0) {
        SaveData(memData, file);
        fclose(file);
        return 0;
      }
    }
  return 0;
}
