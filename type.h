//Nick Hayward
//Lab 6

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <linux/fs.h>	//Fixed and Added
#include <linux/ext2_fs.h>
#include <stdbool.h>

// define shorter TYPES
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

// define pointer variables
GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777 
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define READY             1
#define RUNNING           2

// Table sizes
#define NMINODES         100
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT             100

// Open File Table
typedef struct oft{
	int   mode;
	int   refCount;
	struct minode *inodeptr;
	int   offset;
}OFT;

// PROC structure
typedef struct proc{
	int   uid;
	int   pid;
	int   gid;
	int   ppid;
	struct proc *parent;
	int   status;
	struct minode *cwd;
	OFT   *fd[NFD];
}PROC;
      
// In-memory inodes structure
typedef struct minode{		
	INODE INODE;               // disk inode
	int   dev, ino;
	int   refCount;
	int   dirty;
	int   mounted;
	struct mount *mountptr;
}MINODE;

// Mount Table structure
typedef struct mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        MINODE *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
}MOUNT;

//Global Variables
int ROOTBLOCK = 0, INODETABLE = 0, INODESIZE = 0, FD = 0,
    IMAP = 0, BMAP = 0, CWD = 0;

char PATHNAME[124] = "NULL", PARAMETER[124] = "NULL", mounted[12][30];

PROC P0, P1, running;

MINODE minode[100];

OFT *oftp;

struct minode *root;

