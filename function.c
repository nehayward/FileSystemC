//Nick Hayward
//Functions

#include "util.c"

void menu()
{
	printf("ls: list directories\n");
	printf("pwd: current pathname\n");
	printf("cd:  goes into directory\n");

	/*
	case 0 : menu();		break;
			case 1 : //pwd();			break;
			case 2 : ls();			break;
			case 3 : cd();			break;
			case 4 : make_dir();		break;
			case 5 : //rmdir();               break;
			case 6 : //creat_file();          break;
			case 7 : //link();                break;
			case 8 : //unlink();              break;
			case 9 : //symlink();             break;
			case 10: //rm_file();             break;
			case 11: //chmod_file();          break;
			case 12: //chown_file();          break;
			case 13: //stat_file();           break;
			case 14: //touch_file();          break;
			case 20: //open_file();           break;		//LEVEL 2
			case 21: //close_file();          break;
			case 22: //pfd();                 break;
			case 23: //lseek_file();          break;
			case 24: //access_file();         break;
			case 25: //read_file();           break;
			case 26: //write_file();          break;
			case 27: //cat_file();            break;
			case 28: //cp_file();             break;
			case 29: //mv_file();             break;
			case 30: //mount();               break;		//LEVLEL 3
			case 31: //umount();              break;
			case 32: //cs();                  break;
			case 33: //do_fork();             break;
			case 34: //do_ps();               break;
			case 40: //sync();                break; 
			case 41: //quit();                break;
			case 42 : system("clear");	break; 
			default: printf("invalid command\n");
			break;

	*/
}

void mount_root(char* device, int *fd)
{	
	int i = 0;
	MINODE *mip;

	*fd = open(device, O_RDWR);
	if (fd < 0)
	{
		printf("open %s failed\n", device);
		exit(1);
	}

	//Intializations
	FD = *fd;
	init();

	if(sp->s_magic == 48)
	{
		mip =  iget(*fd,2);
		(minode[0]) = *mip;

		root = mip; //get root inode

		mip->refCount++;

		P0.cwd = mip;
		running.cwd = mip;

	}
	else
	{
		printf("Error not the Superblock for an EXT2 file system\n");
		exit(1);
	}

	
}

void pwd(MINODE *cwd)
{
	char buf[BLOCK_SIZE], tempbuf[BLOCK_SIZE], name[128] = "/", temp[10][1024], *tempcp, *cp;
	int currentDirectory = -1, parentDirectory = -2, i = 0;
	DIR *tempdp;

	//Intialize Temp
	while(i < 10)
	{
		strcpy(temp[i], "NULL");
		i++;
	}

	i = 0;
	mailman(FD, cwd->ino, buf);
	get_block(FD, ip->i_block[0], buf);

	dp = (DIR *)buf;
	cp = buf;

	while(currentDirectory != parentDirectory) 
	{
		while (cp < &buf[BLOCK_SIZE])
		{	
			if(strcmp(dp->name, ".") == 0)
			{
				currentDirectory = dp->inode;
			}
			if(strcmp(dp->name, "..") == 0)
				parentDirectory = dp->inode;
			
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		if(currentDirectory != parentDirectory)
		{
			mailman(FD, parentDirectory, buf);
			mailman(FD, parentDirectory, tempbuf);

			//Changes to new i_block
			get_block(FD, ip->i_block[0], buf);
			get_block(FD, ip->i_block[0], tempbuf);
	
			//ASSIGNS NEW DP
			dp = (DIR *)buf;
			cp = buf;
			
			//ASSIGNS NEW TempDP
			tempdp = (DIR *)tempbuf;
			tempcp = tempbuf;

			while (tempcp < &tempbuf[BLOCK_SIZE])
			{	
				if(currentDirectory == tempdp->inode)
				{	
					strcpy(temp[i],tempdp->name);
				}
				tempcp += tempdp->rec_len;
				tempdp = (DIR *)tempcp;
			}
			
		}
		i++;
	}

	i = 9;
	while(i > -1)
	{
		if(strcmp(temp[i], "NULL") != 0)
		{
			strcat(name, temp[i]);
			if(i != 0)
				strcat(name, "/");
		}
		i--;
	}

	printf("%s", name);
}

void ls()
{
	char buf[BLOCK_SIZE], *cp;
	int ino = 0, temp = 0;
	MINODE *mip;

	if(strcmp(PATHNAME, "NULL") == 0)
	{

		mailman(FD, P0.cwd->ino, buf);
		temp = ip->i_block[0];

		if(temp < 0)
		{
			printf("ERROR: Not a directory\n");
			return;
		}
		get_block(FD, ip->i_block[0], buf);

		dp = (DIR *)buf;
		cp = buf;
	
		while (cp < &buf[BLOCK_SIZE])
		{	
			printf("Inode = %-2d Rec_Len = %-3d Name Length = %-2d Name = %s\n",
						dp->inode, dp->rec_len, dp->name_len, dp->name);
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
	}
	else
	{	
		ino = get_ino(FD,PATHNAME);
		if(ino < 1)
		{
			printf("ERROR: Not a directory\n");
			return;
		}

		mip = iget(FD, ino);
		if(!S_ISDIR(mip->INODE.i_mode))
		{
			printf("%s is not a directory\n", PATHNAME);
			return;
		}

		mailman(FD, mip->ino, buf);

		get_block(FD, ip->i_block[0], buf);
		dp = (DIR *)buf;
		cp = buf;
		
		while (cp < &buf[BLOCK_SIZE])
		{	
			printf("Inode = %-2d Rec_Len = %-3d Name Length = %-2d Name = %s\n",
						dp->inode, dp->rec_len, dp->name_len, dp->name);

			cp += dp->rec_len;
			dp = (DIR *)cp;
		}

	}



}

void cd()
{
	char buf[1024];
	int ino = 0, temp = 0;
	MINODE *mip;

	if(strcmp(PATHNAME, "NULL") ==  0)
	{
		//printf("%d\n, root %d", root->ino, rootBlock);
		mailman(FD,root->ino, buf);

		//CHANGES CWD
		P0.cwd = iget(FD, root->ino);

	}
	else
	{
		mailman(FD, P0.cwd->ino, buf);
		ino = get_ino(FD,PATHNAME);
		if(ino == 0)
		{
			printf("ERROR: Not a directory\n");
			return;
		}

		mip = iget(FD, ino);

		if(S_ISDIR(mip->INODE.i_mode))
		{
			mailman(FD, mip->ino, buf);
			P0.cwd = iget(FD, mip->ino);
		}
		else
		{
			printf("%s is not a directory\n", PATHNAME);
			return;
		}
		
	}


}

void make_dir()
{
	char parent[128] = "NULL", child[128] = "NULL", temp_pathname[128] = "NULL", buf[BLOCK_SIZE];
	int ino = 0, r = 0, inParent = -1;
	MINODE *pip;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") != 0)
	{	
		strcpy(temp_pathname, PATHNAME); 
		get_dirname(temp_pathname, parent); 	 //Get Dirname

		strcpy(temp_pathname, PATHNAME); 
		get_basename(temp_pathname, child); //Get Basename
		
	}
	else
	{
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, parent);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}

	pip = iget(FD, ino);
	if(S_ISDIR(pip->INODE.i_mode))
	{
		inParent = search(pip, child);
		if(inParent != -1)
		{
			printf("ERROR: Child is in parent\n");
			return;
		}
		r = my_mkdir(pip, child);
	}
	
}

int my_mkdir(MINODE *pip, char *name)
{
	int inumber = 0, bnumber = 0, i = 0, need_len = 0, name_len = 0, new_len = 0, ideal_len = 0;
	char buf[1024], str[64], *prev, *cp;
	time_t now = time(0);
	MINODE *mip; 

	pip->dirty = 1;
	FD = pip->dev;

	//Allocate inode and a disk block for new directory
	inumber = ialloc(FD);
	bnumber = balloc(FD);

	mip = iget(FD,inumber);
	
	memcpy ((void*)&(mip->INODE), (void*) &(root->INODE), sizeof(INODE));
	mip->INODE.i_mode = DIR_MODE;		// DIR and permissions 
	mip->INODE.i_uid  = running.uid;	// Owner Uid 
	mip->INODE.i_gid =  running.gid;	// Group Id 
	mip->INODE.i_size = 1024 ;		// Size in bytes 

	mip->INODE.i_links_count = 2;	// Links count 

	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L); 

	mip->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	mip->dirty = 1;               // mark dirty 

	for (i=0; i < 15; i++)
  	  mip->INODE.i_block[i] = 0;
  
	mip->INODE.i_block[0] = bnumber; 
	iput(mip);

	//SET CUR AND PARENT DIR
	dp = (DIR *)buf;

	dp->inode = inumber;		// Inode number 
	strncpy(dp->name, ".", 1);    // File name 
	dp->name_len = 1;		// Name length 
	dp->rec_len = 12;		// Directory entry length 

	//New Entry
	cp = buf;
	cp += dp->rec_len;             
	dp = (DIR *)cp;

	dp->inode = pip->ino;	   // Inode number of parent DIR
	dp->name_len = 2;		// Name length 
	strncpy(dp->name, "..", 2);   // .. File name 
	dp->rec_len = BLOCK_SIZE - 12;// last DIR entry length to end of block 

	put_block(FD, bnumber, buf);


	//INSERT BLOCK
	//GO TO LAST ENTRY
	mailman(FD, pip->ino,buf);	//ADDED and FIXED
	pip->INODE = *ip;
	get_block(FD, pip->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE || dp->inode == 0)
	{    
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}      

	dp = (DIR *)prev; 	//DP NOW LAST ENTRY
  	cp = prev;

	//Gets length of child
	name_len = strlen(name);
	need_len = 4*((8 + name_len+3)/4);
	
	name_len = strlen(dp->name);
	ideal_len = 4*((8+name_len+3)/4);

	if((dp->rec_len - ideal_len) >= need_len)
	{	
		//TRIM
		new_len = dp->rec_len - ideal_len;	
		dp->rec_len = ideal_len;
	
		//NEW
		cp += dp->rec_len;               
		dp = (DIR *)cp;
	
		dp->inode = inumber;			// Inode number 
		strncpy(dp->name, name, strlen(name));   // .. File name
		dp->name_len = strlen(name);		// Name length
		dp->rec_len = new_len;	// Directory entry length 

	}
	else
	{
		printf("ERROR\n");
		return;
	}
	
	//INSERTS DIRECTORY
	dp = (DIR *)buf;
	cp = buf; 

	while(cp < buf + BLOCK_SIZE  && dp->rec_len != 0)
	{
		prev = cp;
		strncpy(str, dp->name, dp->name_len);
		str[dp->name_len] = '\0';
		
		//printf("Inode = %d Rec_Len = %d Name Length = %d Name = %s\n", 
		//		dp->inode, dp->rec_len, dp->name_len, dp->name);
		//getchar();
		cp += dp->rec_len;            // advance by rec_len 
		dp = (DIR *)cp;
	}

	put_block(FD, pip->INODE.i_block[0], buf);

	now = time(0);
	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0);
	iput(pip);

	//REAFFIRMS VALUES
	mip = iget(FD, inumber);

	mip->INODE.i_mode = DIR_MODE;		// DIR and permissions 
	mip->INODE.i_uid  = running.uid;	// Owner Uid 
	mip->INODE.i_gid =  running.gid;	// Group Id 
	mip->INODE.i_size = 1024 ;		// Size in bytes 

	mip->INODE.i_links_count = 2;	// Links count 

	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L); 

	mip->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	mip->dirty = 1;               // mark dirty 

	return 1;
}

int rmdir()
{
	char temp[128] = "NULL", child[128], parent[128], buf[BLOCK_SIZE], *cp;
	int ino = 0, pino = 0, r = 0, i = 0;
	MINODE *mip, *pip;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing operand\n");
		return;
	}
		
	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Directory doesn't exist\n");
		return;
	}
	

	//CHECKS IF IT'S NOT DIR OR BUSY OR NOT EMPTY
	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{
		printf("ERROR: Not a Directory\n");
		return;
	}
	if(S_ISDIR(mip->INODE.i_mode) == 1 || mip->INODE.i_links_count > 2)
	{
		mailman(FD, mip->ino, buf);
		get_block(FD, ip->i_block[0], buf);
		dp = (DIR *)buf;
		cp = buf;
		
		while (cp < &buf[BLOCK_SIZE])
		{	
			cp += dp->rec_len;
			dp = (DIR *)cp;
			i++;
		}
	}
	if(i > 2)
	{
		printf("ERROR: Not an empty DIR\n");
		return 0;
	}
	
	for (i=0; i<12; i++)
	{
		if (mip->INODE.i_block[i]==0)
		continue;
		bdalloc(mip->dev, mip->INODE.i_block[i]);
	}

	idalloc(mip->dev, mip->ino);
	iput(mip); //(which clears mip->refCount = 0);
     
	
	//GET PARENTS DIR'S INO
	strcpy(temp, PATHNAME); 
	get_dirname(temp, parent); //Get Basename
	pino = get_ino(FD, parent);

	pip = iget(mip->dev, pino); 

	//GETS BASENAME
	strcpy(temp, PATHNAME); 
	get_basename(temp, child); //Get Basename
		
	r = rm_child(pip, child);

	return r;
}


int rm_child(MINODE *parent, char *my_name)
{
	char buf[1024], buf2[1024], *cp, *prev, *next;	
	int child_exist = 0, saved_rec_len = 0, total_rec_len = 0, prev_r = 0, bytes_remain = 0;
	time_t curTime;

	child_exist = search(parent, my_name);
	if(child_exist == -1)
	{
		printf("ERROR: Child not in parent\n");
		return;
	}

	mailman(FD, parent->ino, buf);	//ADDED and FIXED
	parent->INODE = *ip;
	get_block(FD, parent->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE)
	{    
		total_rec_len += dp->rec_len;
		if(strcmp(my_name,dp->name) == 0)
			break;
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	} 
	
	//SAVES REC_LEN TO BE REMOVED   
	saved_rec_len = dp->rec_len;

	cp += dp->rec_len;
	next = cp;
	cp -= saved_rec_len;

	//ASSIGNS PREV AND NEXT
	dp =(DIR*)prev;
	dp =(DIR*)next;

	if(dp->inode == 0) //LAST ENTRY
	{
		dp =(DIR*)cp;//HOLDS ITEM TO DELETE
		total_rec_len -= saved_rec_len;
		bytes_remain = BLOCK_SIZE - total_rec_len;
		memcpy ((void*)(buf2), (void*) (next), bytes_remain);
		memcpy ((void*)(cp), (void*) (buf2), bytes_remain);

		cp = buf; 
		dp = (DIR *)buf;
	
		//GOES TO END OF BUF
		while(cp < buf + BLOCK_SIZE  && dp->rec_len != 0)
		{
			prev_r = dp->rec_len;
			cp += dp->rec_len;            // advance by rec_len 
			dp = (DIR *)cp;
		}
		cp -= prev_r;            // advance by rec_len 
		dp = (DIR *)cp;

		dp->rec_len += saved_rec_len;
	
	}
	else //NOT LAST
	{
		dp =(DIR*)cp;//HOLDS ITEM TO DELETE

		bytes_remain = BLOCK_SIZE - total_rec_len;
		memcpy ((void*)(buf2), (void*) (next), bytes_remain);
		memcpy ((void*)(cp), (void*) (buf2), bytes_remain);

		cp = buf; 
		dp = (DIR *)buf;
	
		//GOES TO END OF BUF
		while(cp < buf + BLOCK_SIZE  && dp->rec_len != 0)
		{
			prev_r = dp->rec_len;
			cp += dp->rec_len;            // advance by rec_len 
			dp = (DIR *)cp;
		}
		cp -= prev_r;            // advance by rec_len 
		dp = (DIR *)cp;

		dp->rec_len += saved_rec_len;
	}

	put_block(FD, parent->INODE.i_block[0], buf);

	time(&curTime);
	parent->INODE.i_links_count--;
	parent->INODE.i_atime = curTime;
	parent->INODE.i_mtime = curTime;
	parent->dirty = 1;
	iput(parent);
}


creat_file()
{
	char parent[128] = "NULL", child[128] = "NULL", temp_pathname[128] = "NULL", buf[BLOCK_SIZE];
	int ino = 0, r = 0, inParent = -1;
	MINODE *pip;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") != 0)
	{	
		strcpy(temp_pathname, PATHNAME); 
		get_dirname(temp_pathname, parent); 	 //Get Dirname

		strcpy(temp_pathname, PATHNAME); 
		get_basename(temp_pathname, child); //Get Basename
		
	}
	else
	{
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, parent);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}

	pip = iget(FD, ino);
	if(S_ISDIR(pip->INODE.i_mode))
	{
		inParent = search(pip, child);
		if(inParent != -1)
		{
			printf("ERROR: Child is in parent\n");
			return;
		}
		r = my_creat(pip,child);
	}

}


int my_creat(MINODE *parent, char *name)
{
	int inumber = 0, bnumber = 0, i = 0, need_len = 0, name_len = 0, new_len = 0, ideal_len = 0;
	char buf[1024], str[64], *prev, *cp;
	time_t now = time(0);
	MINODE *mip;

	parent->dirty = 1;
	FD = parent->dev;

	//Allocate inode
	inumber = ialloc(FD);
	bnumber = balloc(FD);

	mip = iget(FD,inumber);
	
	//memcpy ((void*)&(mip->INODE), (void*) &(root->INODE), sizeof(INODE));
	mip->INODE.i_mode = FILE_MODE;		// DIR and permissions 
	mip->INODE.i_uid  = running.uid;	// Owner Uid 
	mip->INODE.i_gid =  running.gid;	// Group Id 
	mip->INODE.i_size = BLOCK_SIZE ;		// Size in bytes 
	mip->INODE.i_links_count = 0;	// Links count 
	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
	mip->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	mip->dirty = 1;               // mark dirty 

	for (i=0; i < 15; i++)
    	mip->INODE.i_block[i] = 0;
  
	mip->INODE.i_block[0] = bnumber; 

	iput(mip);

	//SET CUR AND PARENT DIR
	dp = (DIR *)buf;


	//INSERT BLOCK
	//GO TO LAST ENTRY
	mailman(FD, parent->ino,buf);	//ADDED and FIXED
	parent->INODE = *ip;
	get_block(FD, parent->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE)
	{    
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}      

	dp = (DIR *)prev; 	//DP NOW LAST ENTRY
  	cp = prev;

	//Gets length of child
	name_len = strlen(name);
	need_len = 4*((8 + name_len+3)/4);
	
	name_len = strlen(dp->name);
	ideal_len = 4*((8+name_len+3)/4);

	if((dp->rec_len - ideal_len) >= need_len)
	{	
		//TRIM
		new_len = dp->rec_len - ideal_len;	
		dp->rec_len = ideal_len;
	
		//NEW
		cp += dp->rec_len;               
		dp = (DIR *)cp;
	
		dp->inode = inumber;			// Inode number 
		strncpy(dp->name, name, strlen(name));   // .. File name
		dp->name_len = strlen(name);		// Name length
		dp->rec_len = new_len;	// Directory entry length 

	}
	else
	{
		printf("ERROR\n");
		return;
	}
	
	//INSERTS DIRECTORY
	put_block(FD, parent->INODE.i_block[0], buf);
	time(&now);
	parent->INODE.i_atime = now;
	iput(parent);

	mip = iget(FD, inumber);

	mip->INODE.i_mode = FILE_MODE;		// DIR and permissions 
	mip->INODE.i_uid  = running.uid;	// Owner Uid 
	mip->INODE.i_gid =  running.gid;	// Group Id 
	mip->INODE.i_size = 1024 ;		// Size in bytes 

	mip->INODE.i_links_count = 0;	// Links count 

	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L); 

	mip->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	mip->dirty = 1;               // mark dirty 

	return 1;


}  


int rm_file()
{
	char temp[128] = "NULL", child[128], parent[128], buf[BLOCK_SIZE], *cp;
	int ino = 0, pino = 0, r = 0, i = 0;
	MINODE *mip, *pip;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing operand\n");
		return;
	}
		
	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: FILE doesn't exist\n");
		return;
	}

	//CHECKS IF IT'S FILE OR BUSY OR NOT EMPTY
	mip = iget(FD, ino);
	if(!S_ISREG(mip->INODE.i_mode))
	{
		printf("ERROR: Not a file\n");
		return;
	}

	iput(mip); //(which clears mip->refCount = 0)

	//GET PARENTS DIR'S INO
	strcpy(temp, PATHNAME); 
	get_dirname(temp, parent); //Get Basename
	pino = get_ino(FD, parent);

	pip = iget(mip->dev, pino); 

	//GETS BASENAME
	strcpy(temp, PATHNAME); 
	get_basename(temp, child); //Get Basename
		
	r = rm_child(pip, child);

	return r;
}


int my_remove(MINODE *parent, char *my_name)
{
	char buf[1024], buf2[1024], *cp, *prev, *next;	
	int child_exist = 0, saved_rec_len = 0, 
	    total_rec_len = 0, prev_r = 0, bytes_remain = 0, i = 0;
	
	time_t curTime;

	child_exist = search(parent, my_name);
	if(child_exist == -1)
	{
		printf("ERROR: Child not in parent\n");
		return;
	}

	mailman(FD, parent->ino, buf);	//ADDED and FIXED
	parent->INODE = *ip;
	get_block(FD, parent->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE)
	{    
		i++;
		total_rec_len += dp->rec_len;
		if(strcmp(my_name,dp->name) == 0)
			break;
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	} 
	//SAVES REC_LEN TO BE REMOVED   
	saved_rec_len = dp->rec_len;
	
	cp += dp->rec_len;
	next = cp;
	

	if(dp->inode == 0) //LAST ENTRY
	{
		dp =(DIR*)cp;//HOLDS ITEM TO DELETE
		total_rec_len -= saved_rec_len;
		bytes_remain = BLOCK_SIZE - total_rec_len;
		memcpy ((void*)(buf2), (void*) (next), bytes_remain);
		memcpy ((void*)(cp), (void*) (buf2), bytes_remain);

		cp = buf; 
		dp = (DIR *)buf;
	
		//GOES TO END OF BUF
		while(cp < buf + BLOCK_SIZE  && dp->rec_len != 0)
		{
			prev_r = dp->rec_len;
			cp += dp->rec_len;            // advance by rec_len 
			dp = (DIR *)cp;
		}
		cp -= prev_r;            // advance by rec_len 
		dp = (DIR *)cp;

		dp->rec_len += saved_rec_len;
	}
	else //NOT LAST
	{
		dp =(DIR*)cp;//HOLDS ITEM TO DELETE

		bytes_remain = BLOCK_SIZE - total_rec_len;
		memcpy ((void*)(buf2), (void*) (next), bytes_remain);
		memcpy ((void*)(cp), (void*) (buf2), bytes_remain);

		cp = buf; 
		dp = (DIR *)buf;
	
		//GOES TO END OF BUF
		while(cp < buf + BLOCK_SIZE  && dp->rec_len != 0)
		{
			prev_r = dp->rec_len;
			cp += dp->rec_len;            // advance by rec_len 
			dp = (DIR *)cp;
		}
		cp -= prev_r;            // advance by rec_len 
		dp = (DIR *)cp;

		dp->rec_len += saved_rec_len;
	}


	put_block(FD, parent->INODE.i_block[0], buf);
	parent->INODE.i_atime = time(0l);
	parent->INODE.i_mtime = time(0l);
	parent->dirty = 1;
	iput(parent);
}


void link()
{
	int ino = 0, inParent = 0, need_len = 0, ideal_len = 0, new_len = 0;
	char parent[128], child[128], temp[124], buf[1024], *cp, *prev;
	MINODE *mip, *pip, *new;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing first operand\n");
		return;
	}
	if(strcmp(PARAMETER,"NULL") == 0)
	{	
		printf("ERROR: missing second operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	mip = iget(FD, ino);

	if(S_ISDIR(mip->INODE.i_mode) == 1)
	{
		printf("ERROR: Directory\n");
		return;
	}
	
	strcpy(temp, PARAMETER);
	get_dirname(temp,parent);		
	ino = get_ino(FD, parent);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}
	mip = iget(FD, ino);
	if(S_ISDIR(mip->INODE.i_mode))
	{	
		strcpy(temp, PARAMETER);
		get_basename(temp, child); //Get Basename
		inParent = search(mip, child);
		if(inParent != -1)
		{
			printf("ERROR: Child is in parent\n");
			return;
		}
	}

	//INSERT BLOCK
	ino = get_ino(FD, parent);		//ASSIGNS PIP
	pip = iget(FD, ino);

	ino = get_ino(FD, PATHNAME);

	mip = iget(FD, ino);
	mip->INODE.i_links_count++;	// Links count
	new = mip;
	iput(new);

	//INSERT BLOCK
	//GO TO LAST ENTRY
	mailman(FD, pip->ino,buf);	//ADDED and FIXED
	pip->INODE = *ip;

	get_block(FD, pip->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE)
	{    
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}      

	dp = (DIR *)prev; 	//DP NOW LAST ENTRY
  	cp = prev;

	//Gets length of child
	need_len = 4*((8 + strlen(child)+3)/4);
	ideal_len = 4*((8 + strlen(dp->name)+3)/4);

	if((dp->rec_len - ideal_len) >= need_len)
	{	
		//TRIM
		new_len = dp->rec_len - ideal_len;	
		dp->rec_len = ideal_len;
	
		//NEW
		cp += dp->rec_len;               
		dp = (DIR *)cp;
	
		dp->inode = ino;			// Inode number 
		strncpy(dp->name, child, strlen(child));   // .. File name
		dp->name_len = strlen(child);		// Name length
		dp->rec_len = new_len;	// Directory entry length 

	}
	else
	{
		printf("ERROR\n");
		return;
	}
	
	//INSERTS DIRECTORY
	put_block(FD, pip->INODE.i_block[0], buf);
	pip->INODE.i_atime = time(0);
	iput(pip);

	/*new = iget(FD, ino);
	new->INODE.i_mode = 0xA000;		// DIR and permissions 
	new->INODE.i_uid  = running.uid;	// Owner Uid 
	new->INODE.i_gid =  running.gid;	// Group Id 
	new->INODE.i_size = 1024 ;		// Size in bytes 

	new->INODE.i_links_count++;	// Links count 

	new->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L); 

	new->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	new->dirty = 1;               // mark dirty 
	*/
}


void unlink()
{
	int ino = 0;
	char parent[128], child[128], temp[124];
	MINODE *mip, *pip;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing first operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	mip = iget(FD, ino);

	if(S_ISDIR(mip->INODE.i_mode) == 1)
	{
		printf("ERROR: Directory\n");
		return;
	}

	//UNLINk
	mip->INODE.i_links_count--;	// Links count
	if(mip->INODE.i_links_count == 0)
	{
		bdalloc(mip->dev, mip->INODE.i_block[0]);
		idalloc(mip->dev, mip->ino);
		iput(mip); //(which clears mip->refCount = 0);

		strcpy(temp, PATHNAME);
		get_dirname(temp, parent);

		strcpy(temp, PATHNAME);
		get_basename(temp, child);

		ino = get_ino(FD, parent);
		pip = iget(FD, ino);
		
		rm_child(pip, child);        // mark dirty 
	
	}

}

void symlink()
{
	int ino = 0, inParent = 0, need_len = 0, ideal_len = 0, new_len = 0;
	char parent[128], child[128], temp[124], buf[1024], *cp, *prev;
	MINODE *mip, *pip, *new;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing first operand\n");
		return;
	}
	if(strcmp(PARAMETER,"NULL") == 0)
	{	
		printf("ERROR: missing second operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	mip = iget(FD, ino);

	if(S_ISDIR(mip->INODE.i_mode) == 1)
	{
		printf("ERROR: Directory\n");
		return;
	}
	
	strcpy(temp, PARAMETER);
	get_dirname(temp,parent);		
	ino = get_ino(FD, parent);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}
	mip = iget(FD, ino);
	if(S_ISDIR(mip->INODE.i_mode))
	{	
		strcpy(temp, PARAMETER);
		get_basename(temp, child); //Get Basename
		inParent = search(mip, child);
		if(inParent != -1)
		{
			printf("ERROR: Child is in parent\n");
			return;
		}
	}

	//INSERT BLOCK
	ino = get_ino(FD, parent);		//ASSIGNS PIP
	pip = iget(FD, ino);

	ino = get_ino(FD, PATHNAME);

	mip = iget(FD, ino);
	mip->INODE.i_links_count++;	// Links count 
	new = mip;
	iput(new);

	//INSERT BLOCK
	//GO TO LAST ENTRY
	mailman(FD, pip->ino,buf);	//ADDED and FIXED
	pip->INODE = *ip;

	get_block(FD, pip->INODE.i_block[0], buf);
	dp = (DIR *)buf;
  	cp = buf; 
	
	while (cp < buf+BLOCK_SIZE)
	{    
		prev = cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}      

	dp = (DIR *)prev; 	//DP NOW LAST ENTRY
  	cp = prev;

	//Gets length of child
	need_len = 4*((8 + strlen(child)+3)/4);
	ideal_len = 4*((8 + strlen(dp->name)+3)/4);

	if((dp->rec_len - ideal_len) >= need_len)
	{	
		//TRIM
		new_len = dp->rec_len - ideal_len;	
		dp->rec_len = ideal_len;
	
		//NEW
		cp += dp->rec_len;               
		dp = (DIR *)cp;
	
		dp->inode = ino;			// Inode number 
		strncpy(dp->name, child, strlen(child));   // .. File name
		dp->name_len = strlen(child);		// Name length
		dp->rec_len = new_len;	// Directory entry length 

	}
	else
	{
		printf("ERROR\n");
		return;
	}
	
	//INSERTS DIRECTORY
	put_block(FD, pip->INODE.i_block[0], buf);
	pip->INODE.i_atime = time(0);
	iput(pip);

	new = iget(FD, ino);
	new->INODE.i_mode = 0xA000;		// DIR and permissions 
	new->INODE.i_uid  = running.uid;	// Owner Uid 
	new->INODE.i_gid =  running.gid;	// Group Id 
	new->INODE.i_size = 1024 ;		// Size in bytes 

	new->INODE.i_links_count++;	// Links count 

	new->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L); 

	new->INODE.i_blocks = 2;     	// Blocks count in 512-byte blocks 
	new->dirty = 1;               // mark dirty 
	
}

void chmod_file()
{
	char temp[124];
	int ino = 0;
	MINODE *mip;

	//MAKES PATHNAME ACTUALLY PATH
	strcpy(temp, PATHNAME);
	strcpy(PATHNAME,PARAMETER);
	strcpy(PARAMETER, temp);

	if(strcmp(PATHNAME,"NULL") == 0 || strcmp(PARAMETER,"NULL") == 0)
	{	
		printf("ERROR: missing operands\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Directory or File doesn't exist\n");
		return;
	}
	
	mip = iget(FD, ino);
	mip->INODE.i_atime = time(0);
	mip->INODE.i_mtime = time(0);

	if(PARAMETER[0] = 'u');
	{
		if(PARAMETER[2] == 'x')
			mip->INODE.i_uid = 'x';
		if(PARAMETER[2] == 'r')
			mip->INODE.i_uid = 'r';
		if(PARAMETER[2] == 'w')
			mip->INODE.i_uid = 'w';
		if(PARAMETER[3] == 'x')
			mip->INODE.i_uid = 'x';
		if(PARAMETER[3] == 'r')
			mip->INODE.i_uid = 'r';
		if(PARAMETER[3] == 'w')
			mip->INODE.i_uid = 'w';
		if(PARAMETER[4] == 'x')
			mip->INODE.i_uid = 'x';
		if(PARAMETER[4] == 'r')
			mip->INODE.i_uid = 'r';
		if(PARAMETER[4] == 'w')
			mip->INODE.i_uid = 'w';
	}
	if(PARAMETER[0] = 'g');
	{
		if(PARAMETER[2] == 'x')
			mip->INODE.i_gid = 'x';
		if(PARAMETER[2] == 'r')
			mip->INODE.i_gid = 'r';
		if(PARAMETER[2] == 'w')
			mip->INODE.i_gid = 'w';
		if(PARAMETER[3] == 'x')
			mip->INODE.i_gid = 'x';
		if(PARAMETER[3] == 'r')
			mip->INODE.i_gid = 'r';
		if(PARAMETER[3] == 'w')
			mip->INODE.i_gid = 'w';
		if(PARAMETER[4] == 'x')
			mip->INODE.i_gid = 'x';
		if(PARAMETER[4] == 'r')
			mip->INODE.i_gid = 'r';
		if(PARAMETER[4] == 'w')
			mip->INODE.i_gid = 'w';
	}
}

void chown_file()
{
	char temp[124];
	int ino = 0;
	MINODE *mip;

	//MAKES PATHNAME ACTUALLY PATH
	strcpy(temp, PATHNAME);
	strcpy(PATHNAME,PARAMETER);
	strcpy(PARAMETER, temp);

	if(strcmp(PATHNAME,"NULL") == 0 || strcmp(PARAMETER,"NULL") == 0)
	{	
		printf("ERROR: missing operands\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Directory or File doesn't exist\n");
		return;
	}
	
	mip = iget(FD, ino);
	mip->INODE.i_atime = time(0);
	mip->INODE.i_mtime = time(0);

	if(strcmp(PARAMETER, "root") == 0);
	{
		
	}
}



void stat_file()
{
	struct stat sb;
	int ino = 0;
	MINODE *mip;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Directory doesn't exist\n");
		return;
	}

	
	mip = iget(FD, ino);
	
	printf("File type:\t");
	switch (mip->INODE.i_mode & S_IFMT) {
	case S_IFBLK:  printf("block device\n");            break;
	case S_IFCHR:  printf("character device\n");        break;
	case S_IFDIR:  printf("directory\n");               break;
	case S_IFIFO:  printf("FIFO/pipe\n");               break;
	case S_IFLNK:  printf("symlink\n");                 break;
	case S_IFREG:  printf("regular file\n");            break;
	case S_IFSOCK: printf("socket\n");                  break;
	default:       printf("unknown?\n");                break;
	}
	
	printf("I-node number:            %d\n", ino);
	printf("Mode:                     %lo (octal)\n",
	   (unsigned long) mip->INODE.i_mode);
	printf("Link count:               %ld\n", (long)  mip->INODE.i_links_count);
	printf("Ownership:                UID=%ld   GID=%ld\n",
	   (long) mip->INODE.i_uid, (long) mip->INODE.i_gid);
	printf("File size:                %lld bytes\n",
	   (long long) mip->INODE.i_size);
	printf("Blocks allocated:         %lld\n",
	   (long long) mip->INODE.i_blocks);
	
	
	printf("Last status change:       %s", ctime(&mip->INODE.i_ctime));
	printf("Last file access:         %s", ctime(&mip->INODE.i_atime));
	printf("Last file modification:   %s", ctime(&mip->INODE.i_mtime));
	
}


void touch_file()
{
	struct stat sb;
	int ino = 0;
	MINODE *mip;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Directory doesn't exist\n");
		return;
	}

	
	mip = iget(FD, ino);

	mip->INODE.i_atime = time(0);
	mip->INODE.i_mtime = time(0);

}

int open_file()
{
	int ino = 0, file = 0, i = 0;
	char parent[128], child[128], temp[124];
	MINODE *mip;
	
	if(strcmp(PATHNAME,"NULL") == 0 || strcmp(PARAMETER,"NULL") == 0)
	{	
		printf("ERROR: missing operands\n");
		return;
	}
	
	ino = get_ino(FD, PATHNAME);
	mip = iget(FD, ino);

	if(S_ISDIR(mip->INODE.i_mode) == 1)
	{
		printf("ERROR: Directory\n");
		return;
	}
	
	strcpy(temp, PARAMETER);
	get_dirname(temp,parent);		
	ino = get_ino(FD, parent);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}
	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{	
		//CHECK IF OPENED HERE

		//ASSIGNS OFT ENTRY
		oftp = falloc();       // get a FREE OFT
		oftp->mode = (int)PARAMETER;     // open mode 
		oftp->refCount = 1;
		oftp->inodeptr = mip;  // point at the file's minode[]

		switch(oftp->mode)
		{
		case 0 : oftp->offset = 0; 
			break;
		case 1 : truncate(mip);        // W : truncate file to 0 size
			oftp->offset = 0;
			mip->dirty = 1;
			break;
		case 2 : oftp->offset = 0;    // RW does NOT truncate file
			mip->dirty = 1;
			break;
		case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
			break;
		default: printf("invalid mode\n");
			return(-1);
		}

		i = 0;
		while(running.fd[i] != NULL)
		{
			i++;
		}
		mip->INODE.i_atime = time(0);
		mip->INODE.i_mtime = time(0);
		
		return i;
	}
}

 
int close_file(int fd)
{
	MINODE *mip;

	oftp = running.fd[fd];
	running.fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) 
		return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	mip = oftp->inodeptr;
	iput(mip);

	fdalloc(oftp);
	return 0; 
}

int pfd()
{
	int i = 0;

	printf("filename  fd  mode  offset\n");
	printf("--------  --  ----  ------ \n");
  	
	//running.fd[i]->inodeptr->INODE.i_block[0]
	while(running.fd[i]->refCount != 0)
	{
		printf("Refcount: ");
	
	}
}

void lseek_file()
{








}

long lseek_f(int fd, long position)
{
	/*
  From fd, find the OFT entry. 

  change OFT entry's offset to position but make sure NOT to over run
  either end of the file.

  return originalPosition
	*/
}



void access_file()
{








}

/*

int myread(int fd, char *buf, int nbytes)
{
 1. avil = fileSize - OFT's offset // number of bytes still available in file.
    int count = 0;
    char *cq = buf;   // cq points at buf[ ]

 2. while (nbytes && avil){
       compute LOGICAL BLOCK number lbk and startByte in that block from offset;
             lbk       = oftp->offset / BLOCK_SIZE;
             startByte = oftp->offset % BLOCK_SIZE;
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12){              // direct block
           blk = mip->INODE.i_block[lbk];
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks 
       }
       else{ 
            //  double indirect blocks
       } 

       // get the data block into readbuf[BLKSIZE] 
       get_block(mip->dev, blk, readbuf);

       // copy from startByte to buf[ ], at most remain bytes in this block 
       char *cp = readbuf + startByte;   
       remain = BLOCK_SIZE - startByte;  // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte into buf[ ]       
             oftp->offset++; 
             count++;                  // count=0 for counting
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
       }
 
       // if one data block is not enough, loop back to OUTER while for more ...

   }
   printf("myread: read %d char from file %d\n", count, fd);  
   return count;   // count is the actual number of bytes read
}

                  OPTMIAZATION OF THE READ CODE:

Instead of reading one byte at a time and updating the counters on each byte 
read, TRY to calculate the maximum number of bytes available is a data block and
the number of bytes still needed to read. Take the minimum of the two, and read
that many bytes in one operation. Then adjust the counters accordingly. This 
would make the read loops more efficient. EXTRA CREDITs if you optimize the
read algorithm in your project.

*/

void write_file()
{
	char src[128] = "NULL", temp_pathname[128] = "NULL", buf[BLOCK_SIZE],
	     basename[124], dirname[124];
	int ino = 0, ino_dest = 0;
	MINODE *mip, *mip_dest;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") != 0)
	{	
		strcpy(src, PATHNAME);  
	}
	else
	{
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, src);
	if(ino == 0)
	{
		printf("ERROR: File doesn't exist\n");
		return;
	}

	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{
		get_block(FD, mip->INODE.i_block[0], buf);
		mywrite(FD, buf, mip->INODE.i_size);
	}





}

int mywrite(int fd, char *buf, int nbytes) 
{
	char wbuf[1024], *cp, *cq;
	int blk = 0, lbk = 0, startByte = 0, remain = 0, i = 0;
	MINODE *mip;

	while (nbytes > 0 )
	{
		//compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
		lbk       = oftp->offset / BLOCK_SIZE;
		startByte = oftp->offset % BLOCK_SIZE;


		if (lbk < 12)
		{                         
			// direct block
			if (mip->INODE.i_block[lbk] == 0)
			{   
				mip->INODE.i_block[lbk] = balloc(mip->dev);
			}

			blk = mip->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < 256 + 12)
		{ 
		    	// indirect blocks
			i = 0;
			while(i < 256)
			{
				if (mip->INODE.i_block[lbk] == 0)
				{   
					mip->INODE.i_block[lbk] = balloc(mip->dev);
				}

				blk = mip->INODE.i_block[lbk];
				i++;
			}
		}
		else
		{
		    // double indirect blocks 
		}

		// all cases come to here : write to the data block 
		get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
		cp = wbuf + startByte;      // cp points at startByte in wbuf[]
		remain = BLOCK_SIZE - startByte;  // number of bytes remain in this block

		while (remain > 0)
		{               // write as much as remain allows  
			*cp++ = *cq++;              // cq points at buf[ ]
			nbytes--; remain--;         // dec counts
			oftp->offset++;             // advance offset
			if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
				mip->INODE.i_size++;    // inc file size (if offset>filesize)
			if (nbytes <= 0) break;     // if already nbytes, break
		}
		put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk

	// loop back to while to write more .... until nbytes are written
  	}

	mip->dirty = 1;       // mark mip dirty for iput() 
	printf("wrote %d char into file fd=%d\n", nbytes, fd);           
	return nbytes;
}


void cat_file()
{
	int 	blk = 0, ino = 0;
	char 	buf[BLOCK_SIZE], *cp;
	MINODE *mip;

	if(strcmp(PATHNAME,"NULL") == 0)
	{	
		printf("ERROR: missing operand\n");
		return;
	}

	ino = get_ino(FD, PATHNAME);
	if(ino == 0)
	{
		printf("ERROR: Path not in directory\n");
		return;
	}

	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{
		blk = mip->INODE.i_block[0];
	
		lseek(FD, (long)(BLOCK_SIZE*(ROOTBLOCK+blk)), 0);
		read(FD, buf, BLOCK_SIZE);

		get_block(FD, blk, buf);
		dp = (DIR *)buf;
		cp = buf;

		printf("%s\n", buf);	
	}
		
	printf("ERROR: Directory\n");	
}

void cp_file()
{
	char src[128] = "NULL", dest[128] = "NULL", temp_pathname[128] = "NULL", buf[BLOCK_SIZE],
	     basename[124], dirname[124];
	int ino = 0, ino_dest = 0;
	MINODE *mip, *mip_dest;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") != 0 && strcmp(PARAMETER,"NULL") != 0)
	{	
		strcpy(src, PATHNAME); 
		strcpy(dest, PARAMETER); 
	}
	else
	{
		printf("ERROR: missing operands\n");
		return;
	}

	ino = get_ino(FD, src);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}

	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{
		strcpy(temp_pathname, dest);
		get_dirname(temp_pathname, dirname);
		
		strcpy(temp_pathname, src);
		get_basename(temp_pathname, basename);

		ino_dest = get_ino(FD, dirname);
		if(ino_dest == 0)
		{
			printf("ERROR: Parent not a directory\n");
			return;
		}

		mip_dest = iget(FD, ino_dest);

		my_creat(mip_dest, basename);
	
	}

}

void mv_file()
{
	char src[128] = "NULL", dest[128] = "NULL", temp_pathname[128] = "NULL", buf[BLOCK_SIZE],
	     basename[124], dirname[124];
	int ino = 0, ino_dest = 0;
	MINODE *mip, *mip_dest;

	if(PATHNAME[0] == '/')
		FD = root->dev;

	if(strcmp(PATHNAME,"NULL") != 0 && strcmp(PARAMETER,"NULL") != 0)
	{	
		strcpy(src, PATHNAME); 
		strcpy(dest, PARAMETER); 
	}
	else
	{
		printf("ERROR: missing operands\n");
		return;
	}

	ino = get_ino(FD, src);
	if(ino == 0)
	{
		printf("ERROR: Parent not a directory\n");
		return;
	}

	mip = iget(FD, ino);
	if(S_ISREG(mip->INODE.i_mode))
	{
		strcpy(temp_pathname, dest);
		get_dirname(temp_pathname, dirname);
		
		strcpy(temp_pathname, dest);
		get_basename(temp_pathname, basename);

		ino_dest = get_ino(FD, dirname);
		if(ino_dest == 0)
		{
			printf("ERROR: Parent not a directory\n");
			return;
		}
		
		link();
		unlink();
	
		/*
		mip_dest = iget(FD, ino_dest);
		my_creat(mip_dest, basename);

		strcpy(temp_pathname, src);
		get_dirname(temp_pathname, dirname);
		
		strcpy(temp_pathname, src);
		get_basename(temp_pathname, basename);
		
		ino = get_ino(FD, dirname);
		mip = iget(FD, ino);

		my_remove(mip, basename);
		*/
	
	}
}


void mount()    /*  Usage: mount filesys mount_point OR mount */
{
	int f = 0;
	
	strcpy(mounted[0], PATHNAME);
	mount_root(mounted[0], &f);
}
  

void umount(char* filesys)
{

/*
1. Search the MOUNT table to check filesys is indeed mounted.

2. Check whether any file is still active in the mounted filesys;
      e.g. someone's CWD or opened files are still there,
   if so, the mounted filesys is BUSY ==> cannot be umounted yet.
   HOW to check?      ANS: by checking all minode[].dev

3. Find the mount_point's inode (which should be in memory while it's mounted 
   on).  Reset it to "not mounted"; then 
         iput()   the minode.  (because it was iget()ed during mounting)

4. return(0);
*/
}  
  
void quit()
{
	/*
	MINODE *mip;	
	
	mailman(FD, root->ino, buf);
	ino = get_ino(FD,PATHNAME);
	mip = iget(FD, ino);
	iput(mip);
	*/

	close(FD);
	exit(1);

}
	

/*

void printDirBlock(int inodeValue)
{
	char 	buf[BLOCK_SIZES], buf2[BLOCK_SIZES], buf3[BLOCK_SIZES], buf4[BLOCK_SIZES];
	char	namebuf[256];
	int i = 0, j = 0;
	int *pointer, *pointer2, *pointer3;

	mailmanAlg(inodeValue, buf);			

	while(i < 12)
	{
		if(ip->i_block[i] != 0 )
			printf("Block Number is %d\n", ip->i_block[i]);
			i++;
		
	}
	printf("\n");
	//getchar();
	if(ip->i_block[12] != 0)
	{
		printf("Single Indirect\nBlock Number is %u\n\n", ip->i_block[12]);
		
		getBlock(ip->i_block[12], buf2);
		pointer = (int*) buf2;
		
		i = 0;
		while(i < 256)
		{	
			printf("Block Number is %d\n",(*pointer+i));
			i++;
		}
		
	}
	mailmanAlg(inodeValue, buf);
	//getchar();
	if(ip->i_block[13] != 0)
	{
		printf("\nDouble Indirect\nBlock Number is %u\n\n", ip->i_block[13]);



		getBlock(ip->i_block[13], buf2);
		pointer = (int*) buf2;
		
		i = 0;
		while(i < 256)
		{	
			//printf("\n\nBlock Number is %d\n\n",(*pointer+i));
			getBlock((*pointer+i), buf3);
			pointer2 = (int*) buf3;
			j = 0;
			
			while(j<256)
			{
				
				printf("Block Number is %d\n",(*pointer2+j));
				j++;
			}
			strcpy(buf3, "");
			i++;
		}
		
	}

}


*/
