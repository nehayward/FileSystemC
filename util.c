//Util.c 
#include "type.h"

int super(void)
{
	char buf[BLOCK_SIZE];

	//READS SUPER block at byte offset 1024
	lseek(FD, (long)BLOCK_SIZE, 0);
	read(FD, buf, BLOCK_SIZE);
	sp = (SUPER *)buf;    
	
	//SAVES INODE TABLE INFO FOR INODE FUNCTION
	INODESIZE = sp->s_inode_size;

	/*	
	printf("SUPER BLOCK\n");
	printf("s_inodes_count = %d\n", sp->s_inodes_count);
	printf("s_blocks_count = %d\n", sp->s_blocks_count);	
	printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);	
	printf("sp->s_free_blocks_count = %d\n", sp->s_free_blocks_count);	
	printf("sp->s_log_block_size = %d\n", sp->s_log_block_size);	// 0 Means 1024
	printf("sp->s_blocks_per_group = %d\n", sp->s_blocks_per_group);       
	printf("sp->s_inodes_per_group = %d\n", sp->s_inodes_per_group);         
	printf("sp->s_mnt_count = %d\n", sp->s_mnt_count);               
	printf("sp->s_max_mnt_count = %d\n", sp->s_max_mnt_count);          
	printf("sp->s_magic = %d\n", sp->s_magic);                       
	printf("sp->s_mtime = %d\n", sp->s_mtime);
	printf("sp->s_inode_size = %d\n", sp->s_inode_size);  
	*/  
}

int gd(void)
{
	char buf[BLOCK_SIZE];

	//READ GROUP DESCRIPTOR BLOCK OFFSET BLOCKSIZE * 2
	lseek(FD, (long)(BLOCK_SIZE*GDBLOCK), 0);
	read(FD, buf, BLOCK_SIZE);
	gp = (GD *)buf;

	//SAVES INODE TABLE INFO FOR INODE FUNCTION
	INODETABLE = gp->bg_inode_table;
	IMAP = gp->bg_inode_bitmap;
	BMAP = gp->bg_block_bitmap;

	/*
	printf("GD BLOCK\n");
	printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
	printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);
	printf("bg_inode_table = %d\n", gp->bg_inode_table);
	printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
	printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count);
	printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count);
	printf("bg_pad = %d\n", gp->bg_pad);
	printf("bg_reserved[3] = %d\n", gp->bg_reserved[3]);
	*/

}

int inode(void)
{
	char buf[BLOCK_SIZE];
	
	//GOES TO WHERE INODE IS AT
	lseek(FD, (long)((BLOCK_SIZE*INODETABLE)+INODESIZE), 0);
	read(FD, buf, BLOCK_SIZE);
	ip = (INODE *)buf;

	//STORES INODE
	ROOTBLOCK = ip->i_block[0];

	/*
	printf("INODE BLOCK\n");
	printf("inode_block = %d\n", ip->i_blocks);
	printf("i_mode = %d\n", ip->i_mode);
	printf("i_size = %d\n", ip->i_size);
	printf("i_block[0] = %d\n", ip->i_block[0]);
	*/
}

int dir(void)
{
   	char buf[BLOCK_SIZE];

	lseek(FD, (long)(BLOCK_SIZE*ROOTBLOCK), 0);
	read(FD, buf, BLOCK_SIZE);
	dp = (DIR *)buf;
	
	/*	
	printf("DIR BLOCK\n");
	printf("inode = %d\n", dp->inode);
	printf("rec_len = %d\n", dp->rec_len);
	printf("name_len = %d\n", dp->name_len);
	printf("file_type = %d\n", dp->file_type);
	printf("name = %s\n\n", dp->name);
	*/
}


void init()
{
	int i = 0;	
	P0.uid = 0;
	P1.uid = 1;
	
	//Intializes Structs
	super();
	gd();
	inode();
	dir();

	running = P0;
	P0.cwd = root;
	P1.cwd = root;
	
	while(i < 100)
	{
		minode[i].refCount = 0;
		i++;
	}

	root = 0;
	
}

void get_block(int dev, int blk, char buf[])
{

	lseek(dev, (long)(BLOCK_SIZE*blk), 0);
	read(dev, buf, BLOCK_SIZE);
 	
}

void put_block(int dev, int blk, char buf[])
{
	lseek(dev, (long)(BLOCK_SIZE*blk),0);
   	write(dev, buf, BLOCK_SIZE);

}

int parse(char pathname[128])
{
	int i = 0;
	char* temp;

	temp = strtok(pathname, "/");
	while(temp != NULL)
	{
		temp = strtok(NULL, "/");
		i++;
	}
	
	return i;
}

void get_dirname(char pathname[128], char dirname[])
{
	int i = 0, length = 0, baseLength = 0;
	char *temp, tempBasename[128], tempPath[128];

	strcpy(dirname, "");

	length = strlen(pathname);
	strcpy(tempPath, pathname);

	temp = strtok(pathname, "/");
	while(temp != NULL)
	{
		strcpy(tempBasename, "");
		strcpy(tempBasename, temp);
		temp = strtok(NULL, "/");
		i++;
	}

	baseLength = strlen(tempBasename);
	baseLength += 1; //Removes backslash

	strncat(dirname, tempPath, (length-baseLength));
	if(i == 1)
	{
		strcpy(dirname, ".");
	}
}

void get_basename(char pathname[128], char basename[])
{
	int i = 0;
	char *temp, name[10][64];
	char * pch;
 
	strcpy(basename, "");	//Set Dirname to Null
	
	temp = strtok(pathname, "/");
	while(temp != NULL)
	{
		strcpy(name[i], temp);
		temp = strtok(NULL, "/");
		i++;
	}

	strcpy(basename, name[i-1]);
	
}

//Returns IP
void mailman(int dev, int inodeValue, char buf[])
{
	int blockNumber = 0, inodeNumber = 0;

	blockNumber = (inodeValue - 1) / 8 + INODETABLE;
	inodeNumber = (inodeValue - 1) % 8;

	get_block(dev, blockNumber, buf);
	ip = (INODE *)buf + inodeNumber;
}

unsigned long get_ino(int dev, char pathname[64])
{
	int i = 0;
	char tempPath[128], buf[1024], namebuf[256], *cp, *temp;
	int returnvalue, elementsOfName, tempBlock;
	
	//CHECKS IF IT'S IN ROOT DIRECTORY
	mailman(FD, P0.cwd->ino, buf);
	tempBlock = ip->i_block[0];

	//SCANS TO ROOTBLOCK OR CURRENTBLOCK
	if(tempBlock == ROOTBLOCK)
		get_block(FD, ROOTBLOCK, buf);
	else
		get_block(FD, tempBlock, buf);

	//ASSIGNS DP
	dp = (DIR *)buf;
        cp = buf;
	
	//SPLITS PATHNAME
	strcpy(tempPath, pathname);
	elementsOfName = parse(tempPath);		//ELEMENTSOFNAME HOLDS NUMBER OF NAMES
	
	//SPLITS NAME BY '/'
	strcpy(tempPath, pathname);
	temp = strtok(tempPath, "/");
	while(i < elementsOfName)
	{
		returnvalue = 0;
		while (cp < &buf[BLOCK_SIZE])
		{	
			//COPIES NAME TO NAMEBUF
			strncpy(namebuf, dp->name, dp->name_len);
			namebuf[dp->name_len] = 0;
			if(strcmp(namebuf,temp) == 0)
			{
				//HOLDS ALL DIRECTORY CONTENTS
				returnvalue = dp->inode;	
			}
			
			//GOES TO NEXT DIRECTORY 
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		//GOES TO NEXT DIRECTORY
		mailman(FD, returnvalue, buf);
	
		//Exits if file doesn't exist
		if(ip->i_block[0] == 0)
			return 0;		

		//Changes to new i_block
		get_block(FD, ip->i_block[0], buf);
	
		//GET NEXT NAME
		temp = strtok(NULL, "/");

		//ASSIGNS NEW DP
		dp = (DIR *)buf;
		cp = buf;
	
		i++;
	}
	return returnvalue;
}

unsigned long search(MINODE *mip, char *name)
{
	int i = 0, j = 0;
	char buf[1024], newbuf[1024], namebuf[256], *cp;
	int newBlock0, inodeValue[20], returnvalue = -1, temp = 0;
	
	mailman(FD,mip->ino, buf);
	temp = ip->i_block[0];

	if(temp < 0)
	{
		printf("Path doesn't exist\n...Exit\n");
		return;
	}

	get_block(FD, ip->i_block[0], buf);

	//ASSIGNS DP
	dp = (DIR *)buf;
	cp = buf;

	while (cp < &buf[BLOCK_SIZE])
	{	
		//COPIES NAME TO NAMEBUF
		strncpy(namebuf, dp->name, dp->name_len);
		namebuf[dp->name_len] = 0;

		if(strcmp(namebuf,name) == 0)
		{
			//HOLDS ALL DIRECTORY CONTENTS
			returnvalue = dp->inode;	
		}

		//GOES TO NEXT DIRECTORY 
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
	
	return returnvalue;
}

MINODE *iget(int dev, unsigned long ino)
{
	char buf[BLOCK_SIZE];
	int i = 0;
	MINODE *mip;
	
	i = 0;
	while( i < NMINODES)
	{
		mip = &minode[i];
		if(mip->ino == ino && mip->refCount > 0)
		{
			mip->refCount++;
			break;
		}
		else
		{
			if(mip->ino == 0)
			{
				//GOES TO DEVICE INODE
				lseek(FD, (long)((BLOCK_SIZE*INODETABLE)+INODESIZE), 0);
				read(FD, buf, BLOCK_SIZE);
				ip = (INODE *)buf;
	
				//STORES INODE
				mip->INODE = *ip;
		
				//SETS OVER VALUES	
				mip->dev = dev;
				mip->ino = ino;
				mip->refCount = 1;
				break;
			}
		}
		i++;
	}	
	
	return &minode[i];
}

void iput(MINODE *mip)
{
	char buf[1024];
	int blk = 0;
	
	blk = (mip->ino - 1) / 8 + INODETABLE;

	mip->refCount--;
	if (mip->refCount)
		return;
	if (mip->dirty==0)
		return;

	mailman(FD,mip->ino,buf);
	memcpy(ip, &mip->INODE, sizeof(INODE));
	put_block(FD, blk, buf); // write blk back to disk
}

int TST_bit(char buf[ ], int BIT)
{
    return buf[BIT/8] & (1 << (BIT%8));
}

int SET_bit(char buf[ ], int BIT)
{
    return buf[BIT/8] |= (1 << (BIT%8));
}

int CLR_bit(char buf[ ], int BIT)
{
    return buf[BIT/8] &= ~(1 << (BIT%8));
}

int ialloc(int dev)
{	
	int i;
	char buf[1024];
	
	get_block(FD, IMAP, buf);       // read in IMAP block from disk
	
	for (i=0; i < NMINODES; i++)
	{
		if (TST_bit(buf, i)==0)
		{
			SET_bit(buf, i);
			gp->bg_free_inodes_count--; // ASSUME: gp->group_descrptor0 in memory
			put_block(FD, IMAP, buf);   // write IMP block back to disk
			return (i+1);  // ino = bit position + 1
		}
   	}
   	printf("FS PANIC: out of INODES\n");
	return 0;

}

void idalloc(dev, ino) // deallocate an ino
{
	char buf[1024];

	get_block(FD, IMAP, buf);
	CLR_bit(buf, ino-1);                // bit position = ino - 1
	gp->bg_free_inodes_count++;         // inc *gp's free_inodes_count
	put_block(FD, IMAP, buf);
}

int balloc(int dev)
{
	int i;
	char buf[1024];
	
	get_block(FD, BMAP, buf);       // read in IMAP block from disk
	for (i=0; i < NMINODES; i++)
	{
		if (TST_bit(buf, i)==0)
		{
			SET_bit(buf, i);
			gp->bg_free_blocks_count--; // ASSUME: gp->group_descrptor0 in memory
			put_block(FD, BMAP, buf);   // write IMP block back to disk
			return i+1;  // ino = bit position + 1
		}
   	}
   	printf("FS PANIC: out of BLOCKS\n");
	return 0;

}

void bdalloc(dev, ino) // deallocate an ino
{
	char buf[1024];

	get_block(FD, BMAP, buf);
	CLR_bit(buf, ino-1);                // bit position = ino - 1
	gp->bg_free_blocks_count++;         // inc *gp's free_inodes_count
	put_block(FD, BMAP, buf);
}

OFT* falloc()
{
	int i = 0;
	char buf[1024];

	get_block(FD, IMAP, buf);       // read in IMAP block from disk
	
	for (i=0; i < NMINODES; i++)
	{
		if (TST_bit(buf, i)==0)
		{
			SET_bit(buf, i);
			gp->bg_free_inodes_count--; // ASSUME: gp->group_descrptor0 in memory
			put_block(FD, IMAP, buf);   // write IMP block back to disk
			//return (i+1);  // ino = bit position + 1
		}
   	}
   	printf("FS PANIC: out of INODES\n");

}

void fdalloc(dev, ino) // deallocate an ino
{
	char buf[1024];

	get_block(FD, IMAP, buf);
	CLR_bit(buf, ino-1);                // bit position = ino - 1
	gp->bg_free_inodes_count++;         // inc *gp's free_inodes_count
	put_block(FD, IMAP, buf);
}


truncate(MINODE *mip)
{
	int i = 0;
	
	while(i < 12)
		bdalloc(FD,mip->INODE.i_block[i]);

	mip->INODE.i_atime = mip->INODE.i_mtime = time(0l);
 
	mip->INODE.i_size = 0;
	mip->dirty = 0;
}


int findmyname(MINODE *parent, unsigned long myino, char *myname) 
{
	/*
   Given the parent DIR (MINODE pointer) and my inumber, this function finds 
   the name string of myino in the parent's data block. This is similar to 
   SERACH() mentioned above.
	*/
}

int findino(MINODE *mip, unsigned long *myino, MINODE *parentino)
{
	/*
  For a DIR Minode, extract the inumbers of . and .. 
  Read in 0th data block. The inumbers are in the first two dir entries.
	*/
}


int findCmd(char* command)
{
	if(strcmp("menu", command) == 0)
		return 0;
	if(strcmp("pwd", command) == 0)
		return 1;
	if(strcmp("ls", command) == 0)
		return 2;
	if(strcmp("cd", command) == 0)
		return 3;
	if(strcmp("mkdir", command) == 0)
		return 4;
	if(strcmp("rmdir", command) == 0)
		return 5;
	if(strcmp("creat", command) == 0)
		return 6;
	if(strcmp("link", command) == 0)
		return 7;
	if(strcmp("unlink", command) == 0)
		return 8;
	if(strcmp("symlink", command) == 0)
		return 9;
	if(strcmp("rm", command) == 0)
		return 10;
	if(strcmp("chmod", command) == 0)
		return 11;
	if(strcmp("chown", command) == 0)
		return 12;
	if(strcmp("stat", command) == 0)
		return 13;
	if(strcmp("touch", command) == 0)
		return 14;
	if(strcmp("open", command) == 0)
		return 20;
	if(strcmp("close", command) == 0)
		return 21;
	if(strcmp("pfd", command) == 0)		//?
		return 22;
	if(strcmp("lseek", command) == 0)
		return 23;
	if(strcmp("access", command) == 0)
		return 24;
	if(strcmp("read", command) == 0)		
		return 25;
	if(strcmp("write", command) == 0)
		return 26;
	if(strcmp("cat", command) == 0)
		return 27;
	if(strcmp("cp", command) == 0)		
		return 28;
	if(strcmp("mv", command) == 0)
		return 29;
	if(strcmp("mount", command) == 0)
		return 30;
	if(strcmp("unmount", command) == 0)		
		return 31;
	if(strcmp("cs", command) == 0)
		return 32;
	if(strcmp("fork", command) == 0)		
		return 33;
	if(strcmp("ps", command) == 0)
		return 34;
	if(strcmp("sync", command) == 0)		
		return 40;
	if(strcmp("quit", command) == 0)		
		return 41;
	if(strcmp("clear", command) == 0)		
		return 42;
	else
		return -1;

}

