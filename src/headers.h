#define BLOCK_SIZE 512
#define MAX_INODES 24
#define MAX_FILES 20
#define MAX_DATA_BLOCKS 96
#define FIRST_DATA_BLOCK 4
#define MAX_PARTITION_BLOCKS MAX_DATA_BLOCKS + FIRST_DATA_BLOCK // Superblock + inode bitmap and block bitmap + inodes + directory
#define MAX_BLOCKS_PER_INODE 7
#define FILE_NAME_LENGTH 17
#define NULL_INODE 0xFFFF
#define NULL_BLOCK 0xFFFF

/* Superblock structure */
typedef struct
{
  unsigned int s_inodes_count;                                    /* Total inodes in the partition */
  unsigned int s_blocks_count;                                    /* Total blocks in the partition */
  unsigned int s_free_blocks_count;                               /* Free blocks available */
  unsigned int s_free_inodes_count;                               /* Free inodes available */
  unsigned int s_first_data_block;                                /* First data block */
  unsigned int s_block_size;                                      /* Block size in bytes */
  unsigned char s_padding[BLOCK_SIZE - 6 * sizeof(unsigned int)]; /* Zero-filled padding */
} EXT_SIMPLE_SUPERBLOCK;

/* Bytemaps, fit within a single block */
typedef struct
{
  unsigned char bmap_blocks[MAX_PARTITION_BLOCKS]; /* Bitmap for blocks */
  unsigned char bmap_inodes[MAX_INODES];           /* Bitmap for inodes: inodes 0 and 1 reserved, inode 2 for directory */
  unsigned char bmap_padding[BLOCK_SIZE - (MAX_PARTITION_BLOCKS + MAX_INODES) * sizeof(char)];
} EXT_BYTE_MAPS;

/* Inode structure */
typedef struct
{
  unsigned int file_size;                            /* Size of the file in bytes */
  unsigned short int i_nblock[MAX_BLOCKS_PER_INODE]; /* Block numbers associated with the inode */
} EXT_SIMPLE_INODE;

/* List of inodes, fit within a single block */
typedef struct
{
  EXT_SIMPLE_INODE inode_blocks[MAX_INODES]; /* Array of inodes */
  unsigned char inode_padding[BLOCK_SIZE - MAX_INODES * sizeof(EXT_SIMPLE_INODE)];
} EXT_BLQ_INODES;

/* Individual directory entry */
typedef struct
{
  char file_name[FILE_NAME_LENGTH]; /* Name of the file */
  unsigned short int dir_inode;     /* Inode number for the file */
} EXT_ENTRY_DIR;

/* Data block */
typedef struct
{
  unsigned char data[BLOCK_SIZE]; /* Data inside the block */
} EXT_DATA;
