//Nick Hayward
//Lab 6
//Main

#include "function.c"

void main(int argc, char *argv[])
{
	int i = 0, cmd = -1, fd = 0; 
	char line[128], cname[64], temp_pathname[124], pathname[124] = "NULL", basename[124] = "NULL",
	     dirname[124] = "NULL", parameter[124] = "NULL", *device;

	//GETS DISK NAME TO MOUNT
	/*
	printf("Enter the name of your disk image\n");
	fgets(device, 128, stdin);
	device[strlen(device)-1] = 0;  // kill the \n char at end	
	*/
	device = "mydisk";
	
	mount_root(device, &fd);

	while(1)
	{
		//printf("P%d running: ", running.uid);
		printf("nick's@linux:");
		pwd(P0.cwd);
		printf("$ ");
		fgets(line, 128, stdin);
		line[strlen(line)-1] = 0;  // kill the \n char at end
		if (line[0]==0) continue;

		sscanf(line, "%s %s %s", cname, pathname, parameter);

		if(strcmp(pathname,"NULL") != 0)		
			strcpy(PATHNAME, pathname);
		
		if(strcmp(parameter,"NULL") != 0)		
			strcpy(PARAMETER, parameter);

		cmd = findCmd(cname); // map cname to an index
		switch(cmd)
		{
			case 0 : menu();		break;
			case 1 : pwd(P0.cwd);printf("\n");			break;
			case 2 : ls();			break;
			case 3 : cd();			break;
			case 4 : make_dir();		break;
			case 5 : rmdir();               break;
			case 6 : creat_file();          break;
			case 7 : link();                break;
			case 8 : unlink();              break;
			case 9 : symlink();             break;
			case 10: rm_file();             break;
			case 11: chmod_file();          break;
			case 12: chown_file();          break;
			case 13: stat_file();           break;
			case 14: touch_file();          break;
			case 20: open_file();           break;		//LEVEL 2
			case 21: close_file((int)PATHNAME);          break;
			case 22: pfd();                 break;
			case 23: lseek_file();          break;
			case 24: //access_file();         break;
				break;
			case 25: //read_file();           
				break;
			case 26: write_file();          break;
			case 27: cat_file();            break;
			case 28: cp_file();             break;
			case 29: mv_file();             break;
			case 30: mount();               break;		//LEVLEL 3
			case 31: //umount();              break;
			case 32: //cs();                  break;
			case 33: //do_fork();             break;
			case 34: //do_ps();               break;
			case 40: //sync();                break; 
			case 41: quit();              	 break;
			case 42 : system("clear");	break; 
			default: printf("invalid command\n");
			break;
			
		}

		//RESET NAMES
		strcpy(parameter, "NULL");
		strcpy(pathname, "NULL");
		strcpy(PATHNAME, "NULL");
	}
	
  }




