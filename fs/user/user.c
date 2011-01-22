#include <stdio.h>
#include "includes.h"
#include <string.h>
#include "user.h"
#include "fs_api.h"
#include "ide_x_hw.h"

#define MEM_BLOCK_BEGIN 0x30200000

//char ptest[512]={1,2,3,4,5,6,7,8,9,10,9,8,7,6,5,4,3,2,1};
char ptest[512]="read from the file finished\n";
FS_FILE *myfile;
char mybuffer[0x100];

static void _error(const char *msg) 
{

  	printk("%s",msg);

}


static void _log(const char *msg) 
{

 	 printk("%s",msg);

}

/*********************************************************************
*
*             _write_file
*
  This routine demonstrates, how to create and write to a file
  using the file system.
*/

static void _write_file(const char *name, const char *txt) 
{
    int x;
  
  	/* create file */
  	myfile = FS_FOpen(name,"w");
  	if (myfile)
  	{
    	/* write to file */
    	x = FS_FWrite(txt,1,strlen(txt),myfile);
    	/* all data written ? */
    	if (x!=(int)strlen(txt)) 
    	{
     		/* check, why not all data was written */
      		x = FS_FError(myfile);
      		sprintk(mybuffer,"Not all bytes written because of error %d.\n",x);
      		_error(mybuffer);
    	}
    	/* close file */
    	FS_FClose(myfile);
  	}
  	else 
  	{
    	sprintk(mybuffer,"Unable to create file %s\n",name);
    	_error(mybuffer);
  	}
}

/*********************************************************************
*
*             _dump_file
*
  This routine demonstrates, how to open and read from a file using 
  the file system.
*/

static void _dump_file(const char *name) 
{
    int x;

    /* open file */
    myfile = FS_FOpen(name,"r");
    if (myfile) 
    {
   		 /* read until EOF has been reached */
    	do 
    	{
      		x = FS_FRead(mybuffer,1,sizeof(mybuffer)-1,myfile);
      		mybuffer[x]=0;
      		if (x) 
      		{
        		_log(mybuffer);
      		}
    	} while (x);
   	 	/* check, if there is no more data, because of EOF */
    	x = FS_FError(myfile);
    	if (x!=FS_ERR_EOF) 
    	{
     	 /* there was a problem during read operation */
      		sprintk(mybuffer,"Error %d during read operation.\n",x);
      		_error(mybuffer);
    	}
    	/* close file */
    	FS_FClose(myfile);
    }
    else 
    {
    	sprintk(mybuffer,"Unable to open file %s.\n",name);
    	_error(mybuffer);
  	}
}


/*********************************************************************
*
*             _show_directory
*
  This routine demonstrates, how to read a directory.
*/
/*
#if FS_POSIX_DIR_SUPPORT

static void _show_directory(const char *name) 
{
    FS_DIR *dirp;
    struct FS_DIRENT *direntp;

    _log("Directory of ");
    _log(name);
    _log("\n");
    dirp = FS_OpenDir(name);
    if (dirp) 
    {
        do 
        {
      		direntp = FS_ReadDir(dirp);
      		if (direntp) 
      		{
        		sprintk(mybuffer,"%s\n",direntp->d_name);
        		_log(mybuffer);
      		}
    	} while (direntp);
    	FS_CloseDir(dirp);
    }
    else 
    {
    	_error("Unable to open directory\n");
    }
}
#endif *//* FS_POSIX_DIR_SUPPORT */


/*********************************************************************
*
*             _show_free
*
  This routine demonstrates, how to read disk space information.
*/
/*
static void _show_free(const char *device) 
{
    FS_DISKFREE_T disk_data;
    int x;

    _log("Disk information of ");
    _log(device);
    _log("\n");
    x = FS_IoCtl(device,FS_CMD_GET_DISKFREE,0,(void*) &disk_data);
    if (x==0) 
    {
    	sprintk(mybuffer,"total clusters     : %lu\navailable clusters : %lu\nsectors/cluster    : %u\nbytes per sector   : %u\n",
          disk_data.total_clusters, disk_data.avail_clusters, disk_data.sectors_per_cluster, disk_data.bytes_per_sector);
    	_log(mybuffer);
	} 
    else 
    {
    	_error("Invalid drive specified\n");
    }
}

*/

void MainTask()
{
    U32 x;

    FS__IDE_Init(0);

    x = FS_IoCtl("ide:",FS_CMD_FORMAT_MEDIA,FS_MEDIA_NAND_64MB,0);
    if (x!=0) 
    {
    	_error("Cannot format Nandflash.\n");
    }

    _write_file("ide:\\default.txt",ptest);
 
    _write_file("123.txt",ptest);
    
    _dump_file("ide:\\default.txt");
    
    _dump_file("123.txt");

}     


int main(void)
{
                               
    FS_Init(); 
    MainTask();
	FS_Exit();

}

