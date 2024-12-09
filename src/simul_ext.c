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

void PrintByteMaps(EXT_BYTE_MAPS *ext_bytemaps) {
    // Function body
}
void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup) {
    // Function body
}
int FindFile(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *name) {
    // Function body
    return 0; // Default return
}
void ListDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes) {
    // Function body
}
int Rename(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *oldName, char *newName) {
    // Function body
    return 0; // Default return
}
int Print(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, EXT_DATOS *memData, char *name) {
    // Function body
  




    return 0; // Default return
}
int Delete(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *name, FILE *file) {
  // Function body
  return 0; // Default return
}
int Copy(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
         EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
         EXT_DATOS *memData, char *srcName, char *destName, FILE *file) {
  // Function body
  return 0; // Default return
}
void SaveInodesAndDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, FILE *file) {
    // Function body
}
void SaveByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *file) {
    // Function body
}
void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *file) {
    // Function body
}
void SaveData(EXT_DATOS *memData, FILE *file) {
    // Function body
}
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2) {
  // Check if input is empty or just whitespace
  if (commandStr == NULL || strlen(commandStr) == 0 || commandStr[0] == '\n') {
    printf("ERROR: Illegal command [info, bytemaps, dir, rename, print, remove, copy, exit]\n");
      return 1; // Return 1 for invalid input
  }
  // Remove newline from the input string if present
  size_t len = strlen(commandStr);

  if (len > 0 && commandStr[len - 1] == '\n') {
    commandStr[len - 1] = '\0';
  }

  // Initialize the output strings
  command[0] = '\0';
  arg1[0] = '\0';
  arg2[0] = '\0';

  // Parse the command string
  char *token = strtok(commandStr, " ");
  if (token == NULL) {
    return 1; // Empty input
  }

  // Copy the command
  strcpy(command, token);

  // Parse the first argument
  token = strtok(NULL, " ");
  if (token != NULL) {
    strcpy(arg1, token);
  }

  // Parse the second argument
  token = strtok(NULL, " ");
  if (token != NULL) {
    strcpy(arg2, token);
  }
  
  // Validate the command
  if (
    strcmp(command, "info")     == 0 ||
    strcmp(command, "bytemaps") == 0 ||
    strcmp(command, "dir")      == 0 ||
    strcmp(command, "rename")   == 0 ||
    strcmp(command, "print")    == 0 ||
    strcmp(command, "remove")   == 0 ||
    strcmp(command, "copy")     == 0 ||
    strcmp(command, "exit")     == 0
  ) {
    printf("DEBUG: Valid\n");
    return 0; 
  } else {
    printf("ERROR: Illegal command [info, bytemaps, dir, rename, print, remove, copy, exit]\n");
    return 1; 
  }
}

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
      perror("Error opening partition file.");
      return 1;
    } else {
      printf("Partition file opened successfully.\n");
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
      if (strcmp(cmd, "info" == 0))
      {
        
      }
      
      //USED FOR CAT: Print(directory, &ext_blq_inodos, memData, );


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
