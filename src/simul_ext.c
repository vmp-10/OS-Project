#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Used for clearing the terminal
#include <ctype.h>
#include "headers.h"

#define COMMAND_LENGTH 100

// Function prototypes
void PrintByteMaps(EXT_BYTE_MAPS *ext_bytemaps); // BYTEMAPS command
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2);
void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup); // INFO command
int FindFile(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *name);
void ListDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes);                       // DIR command
int Rename(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *oldName, char *newName); // RENAME command
int Print(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, EXT_DATA *memData, char *name); // CAT command
int Delete(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,                                // DELETE command
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *name, FILE *file);
int Copy(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, // COPY command
         EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
         EXT_DATA *memData, char *srcName, char *destName, FILE *file);

void SaveInodesAndDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, FILE *file);
void SaveByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *file);
void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *file);
void SaveData(EXT_DATA *memData, FILE *file);

void PrintByteMaps(EXT_BYTE_MAPS *ext_bytemaps)
{
    printf("I-nodes: ");
    for (int i = 0; i < MAX_INODES; i++)
    {
        printf("%d ", ext_bytemaps->bmap_inodes[i] != 0); // Prints 1 if byte is non-zero, 0 if zero
    }
    printf("\n");

    printf("Blocks [0-25]: ");
    for (int i = 0; i < 25; i++)
    {                                                     // 25 blocks as requested
        printf("%d ", ext_bytemaps->bmap_blocks[i] != 0); // Prints 1 if byte is non-zero, 0 if zero
    }
    printf("\n");
}

void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup)
{
    printf("Superblock Information:\n");
    printf("-> Block size: %u bytes\n", psup->s_block_size);
    printf("-> Inodes: %u\n", psup->s_inodes_count);
    printf("-> Free inodes: %u\n", psup->s_free_inodes_count);
    printf("-> Blocks count: %u\n", psup->s_blocks_count);
    printf("-> Free blocks: %u\n", psup->s_free_blocks_count);
    printf("-> First data block: %u\n", psup->s_first_data_block);
}

void ListDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes)
{
    // i = 1 to avoid root folder
    for (int i = 1; i < MAX_FILES; i++)
    {
        if (directory[i].dir_inode != NULL_INODE)
        {
            // Looks up directory[i].dir_inode in inodes array
            EXT_SIMPLE_INODE *inode = &inodes->inode_blocks[directory[i].dir_inode];

            printf("%-14s\tSize: %d\t I-node: %d, Blocks: ",
                   directory[i].file_name,
                   inode->file_size,
                   directory[i].dir_inode);

            // Get all block numbers associated with this inode
            for (int j = 0; j < MAX_BLOCKS_PER_INODE; j++)
            {
                if (inode->i_nblock[j] != NULL_BLOCK)
                {
                    printf("%d ", inode->i_nblock[j]);
                }
            }
            printf("\n");
        }
    }
}

int FindFile(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *name)
{
    // Iterate through the directory to find the file
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (strcmp(directory[i].file_name, name) == 0)
        {
            return i; // Return the index of the filename);
        }
    }

    return -1;
}

int Rename(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, char *oldName, char *newName)
{
    // Check if the new file name is too long
    if (strlen(newName) >= FILE_NAME_LENGTH)
    {
        printf("Error: New file name too long.\n");
        return -1;
    }

    // Check if the new name already exists
    int newFileIndex = FindFile(directory, inodes, newName);
    if (newFileIndex != -1)
    {
        printf("ERROR: File '%s' already exists.\n", newName);
        return -1;
    }

    // Find the "oldName" file
    int oldFileIndex = FindFile(directory, inodes, oldName);
    if (oldFileIndex == -1)
    {
        printf("Error: File '%s' not found.\n", oldName);
        return -1;
    }

    // Rename the file
    strncpy(directory[oldFileIndex].file_name, newName, FILE_NAME_LENGTH - 1);
    directory[oldFileIndex].file_name[FILE_NAME_LENGTH - 1] = '\0'; // Ensure null termination

    return 0;
}

int Print(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, EXT_DATA *memData, char *name)
{
    printf("Print called with name: %s\n", name);
    return 0; // Temporary default
}

int Delete(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *name, FILE *file)
{

    // Find the file
    int inodeIndex = FindFile(directory, inodes, name);
    if (inodeIndex == -1)
    {
        printf("Error: File '%s' not found.\n", name);
        return -1;
    }

    int inodeNum = directory[inodeIndex].dir_inode;            // Get i-node number
    EXT_SIMPLE_INODE *inode = &inodes->inode_blocks[inodeNum]; // Get i-node

    // Free the blocks associated with the inode
    for (int i = 0; i < MAX_BLOCKS_PER_INODE; i++)
    {
        if (inode->i_nblock[i] != NULL_BLOCK)
        {
            int blockNum = inode->i_nblock[i];
            ext_bytemaps->bmap_blocks[blockNum] = 0; // Set bytemap as unused
            ext_superblock->s_free_blocks_count++;   // Update superblock info
            inode->i_nblock[i] = NULL_BLOCK;         // Clear the inode block
        }
    }

    // Free the inode
    ext_bytemaps->bmap_inodes[inodeNum] = 0; // Set bytemap as unused
    ext_superblock->s_free_inodes_count++;   // Update superblock info
    inode->file_size = 0;                    // Reset the file size

    // Remove the directory entry
    int dirCount = 0;
    while (directory[dirCount].dir_inode != NULL_INODE && dirCount < MAX_FILES)
    {
        dirCount++;
    }

    // Shift directory entries to remove the gap
    for (int i = inodeIndex; i < dirCount - 1; i++)
    {
        directory[i] = directory[i + 1];
    }
    // Clear the last directory entry
    directory[dirCount - 1].dir_inode = NULL_INODE;
    return 0;
}

void CopyInode(EXT_SIMPLE_INODE *src, EXT_SIMPLE_INODE *dest)
{
    dest->file_size = src->file_size;
    for (int i = 0; i < MAX_BLOCKS_PER_INODE; i++)
    {
        dest->i_nblock[i] = NULL_BLOCK;
    }
}

void updateFirstDataBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_BYTE_MAPS *ext_bytemaps)
{
    // Finds first empty data block in bytemaps
    for (int i = 0; i < MAX_DATA_BLOCKS; i++)
    {
        if (ext_bytemaps->bmap_blocks[i] == 0)
        {
            ext_superblock->s_first_data_block = i;
            return;
        }
    }
}

int Copy(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes,
         EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
         EXT_DATA *memData, char *srcName, char *destName, FILE *file)
{

    // Check if the original file exists
    int srcInodeIndex = FindFile(directory, inodes, srcName);
    if (srcInodeIndex == -1)
    {
        printf("Error: File '%s' not found.\n", srcName);
        return -1;
    }

    // Check if there's already a file with the desired name
    int destNameIndex = FindFile(directory, inodes, destName);
    if (destNameIndex != -1)
    {
        printf("ERROR: File '%s' already exists.\n", destName);
        return -1;
    }

    // Get the original i-node
    int srcInodeNum = directory[srcInodeIndex].dir_inode;            // Get i-node number
    EXT_SIMPLE_INODE *srcInode = &inodes->inode_blocks[srcInodeNum]; // Get i-node

    // Find first available inode
    int newInodeNum = -1;
    for (int i = 1; i < MAX_INODES; i++)
    {
        if (ext_bytemaps->bmap_inodes[i] == 0)
        { // Checks if bytemap is free
            newInodeNum = i;
            ext_bytemaps->bmap_inodes[i] = 1;
            ext_superblock->s_free_inodes_count--;
            break;
        }
    }
    if (newInodeNum == -1)
    {
        printf("ERROR: No free i-nodes available.\n");
        return -1;
    }

    // Find first available directory entry
    int dirIndex = -1;
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (directory[i].dir_inode == NULL_INODE)
        {
            dirIndex = i;
            directory[i].dir_inode = newInodeNum;
            strcpy(directory[i].file_name, destName);
            break;
        }
    }
    if (dirIndex == -1)
    {
        printf("ERROR: No space in the directory.\n");
        return -1;
    }

    // Initialize the new inode, use copyInode so it doesn't modify the srcInode
    EXT_SIMPLE_INODE *destInode = &inodes->inode_blocks[newInodeNum];
    CopyInode(srcInode, destInode);

    // Copy data blocks
    for (int i = 0; i < MAX_BLOCKS_PER_INODE; i++)
    { // Iterate over original i-node blocks
        if (srcInode->i_nblock[i] != NULL_BLOCK)
        { // Check if block is occupied with data
            int success = 0;
            for (int j = 0; j < MAX_DATA_BLOCKS; j++)
            {
                if (ext_bytemaps->bmap_blocks[j] == 0)
                {

                    // Update superblock data
                    ext_bytemaps->bmap_blocks[j] = 1; // Mark block as used
                    ext_superblock->s_free_blocks_count--;

                    memcpy(&memData[j].data, &memData[srcInode->i_nblock[i]].data, BLOCK_SIZE);
                    destInode->i_nblock[i] = j;
                    success = 1;
                    break;
                }
            }

            if (!success)
            {
                for (int k = 0; k < i; k++)
                {
                    ext_bytemaps->bmap_blocks[destInode->i_nblock[k]] = 0;
                    ext_superblock->s_free_blocks_count++;
                }
                printf("ERROR: No free data blocks available.\n");
                return -1;
            }
        }
    }

    updateFirstDataBlock(ext_superblock, ext_bytemaps); // Updates the first data block in the superblock
    return 0;
}

void SaveInodesAndDirectory(EXT_ENTRY_DIR *directory, EXT_BLQ_INODES *inodes, FILE *file)
{
}

void SaveByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *file)
{
}

void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *file)
{
}

void SaveData(EXT_DATA *memData, FILE *file)
{
}

int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2)
{
    // Check if input is empty or just whitespace
    if (commandStr == NULL || strlen(commandStr) == 0 || commandStr[0] == '\n')
    {
        printf("ERROR: Illegal command [info, bytemaps, dir, rename, print, remove, copy, exit]\n");
        return 1; // Return 1 for invalid input
    }
    // Remove newline from the input string if present
    size_t len = strlen(commandStr);

    if (len > 0 && commandStr[len - 1] == '\n')
    {
        commandStr[len - 1] = '\0';
    }

    // Initialize the output strings
    command[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';

    // Parse the command string
    char *token = strtok(commandStr, " ");
    if (token == NULL)
    {
        return 1; // Empty input
    }

    // Copy the command
    strcpy(command, token);

    // Parse the first argument
    token = strtok(NULL, " ");
    if (token != NULL)
    {
        strcpy(arg1, token);
    }

    // Parse the second argument
    token = strtok(NULL, " ");
    if (token != NULL)
    {
        strcpy(arg2, token);
    }

    // Validate the command
    if (
        strcmp(command, "info") == 0 ||
        strcmp(command, "bytemaps") == 0 ||
        strcmp(command, "dir") == 0 ||
        strcmp(command, "rename") == 0 ||
        strcmp(command, "print") == 0 ||
        strcmp(command, "remove") == 0 ||
        strcmp(command, "copy") == 0 ||
        strcmp(command, "exit") == 0)
    {
        // Additional validation for 'remove' and 'print'
        if ((strcmp(command, "remove") == 0 || strcmp(command, "print") == 0) &&
            (strlen(arg1) == 0))
        {
            printf("ERROR: Command '%s' requires an argument.\n", command);
            return 1; // Return 1 for missing arguments
        }

        // Additional validation for 'copy' and 'rename'
        if ((strcmp(command, "copy") == 0 || strcmp(command, "rename") == 0) &&
            (strlen(arg1) == 0 || strlen(arg2) == 0))
        {
            printf("ERROR: Command '%s' requires two arguments.\n", command);
            return 1; // Return 1 for missing arguments
        }
        return 0;
    }
    else
    {
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
    EXT_BLQ_INODES ext_inode_blocks;
    EXT_ENTRY_DIR directory[MAX_FILES];
    EXT_DATA memData[MAX_DATA_BLOCKS];
    EXT_DATA fileData[MAX_PARTITION_BLOCKS];
    int dirEntry;
    int saveData;
    FILE *file;

    // Clear the terminal
    system("clear");

    // Open the binary partition file
    file = fopen("partition.bin", "r+b");
    if (!file)
    {
        perror("Error opening partition file.");
        return 1;
    }
    else
    {
        printf("Partition file opened successfully.\n");
    }

    // Read the entire file in one go
    fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);

    // Map binary file sections into respective structures
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
    memcpy(&directory, (EXT_ENTRY_DIR *)&fileData[3], BLOCK_SIZE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODES *)&fileData[1], BLOCK_SIZE);
    memcpy(&ext_inode_blocks, (EXT_BLQ_INODES *)&fileData[2], BLOCK_SIZE);
    memcpy(&memData, (EXT_DATA *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);

    // Command processing loop
    while (1)
    {
        do
        {
            printf(">> ");
            fflush(stdin);
            fgets(command, COMMAND_LENGTH, stdin);
        } while (CheckCommand(command, cmd, arg1, arg2) != 0);

        // Basically a switch case for all commands
        if (strcmp(cmd, "dir") == 0)
        {
            ListDirectory(directory, &ext_inode_blocks);
        }
        else if (strcmp(command, "info") == 0)
        {
            ReadSuperBlock(&ext_superblock);
        }
        else if (strcmp(command, "bytemaps") == 0)
        {
            PrintByteMaps(&ext_bytemaps);
        }
        else if (strcmp(command, "rename") == 0)
        {
            Rename(directory, &ext_inode_blocks, arg1, arg2);
        }
        else if (strcmp(command, "print") == 0)
        {
            Print(directory, &ext_inode_blocks, memData, arg1);
        }
        else if (strcmp(command, "remove") == 0)
        {
            Delete(directory, &ext_inode_blocks, &ext_bytemaps, &ext_superblock, arg1, file);
        }
        else if (strcmp(command, "copy") == 0)
        {
            Copy(directory, &ext_inode_blocks, &ext_bytemaps, &ext_superblock, memData, arg1, arg2, file);
        }

        // Save metadata after modifying the filesystem
        SaveInodesAndDirectory(directory, &ext_inode_blocks, file);
        SaveByteMaps(&ext_bytemaps, file);
        SaveSuperBlock(&ext_superblock, file);

        // Handle the "exit" command
        if (strcmp(cmd, "exit") == 0)
        {
            while (1)
            {
                printf(">> Save data [0/1]: ");
                fflush(stdin);
                fgets(saveData, sizeof(saveData), stdin); // read the input

                if (strcmp(saveData, "0") == 0)
                {
                    break;
                }
                else if (strcmp(saveData, "1") == 0)
                {
                    SaveData(memData, file);
                    break;
                }
                else
                {
                    printf("Invalid input. Please enter either '0' or '1'.\n");
                }
            }
            fclose(file);
            return 0;
        }
    }
    return 0;
}