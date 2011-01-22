/*
**********************************************************************
*                          Micrium, Inc.
*                      949 Crestview Circle
*                     Weston,  FL 33327-1848
*
*                            uC/FS
*
*             (c) Copyright 2001 - 2003, Micrium, Inc.
*                      All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File        : fat_dir.c
Purpose     : POSIX 1003.1 like directory support
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "fs_conf.h"
#include "fs_port.h"
#include "fs_dev.h"
#include "fs_api.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_lbl.h"
#include "fs_fat.h"
#include "fs_clib.h"

#if FS_POSIX_DIR_SUPPORT


/*********************************************************************
*
*             _FS_fat_create_directory
*
  Description:
  FS internal function. Create a directory in the directory specified
  with DirStart. Do not call, if you have not checked before for 
  existing directory with name pDirName.

  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pDirName    - Directory name. 
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Size of the directory starting at DirStart.
  
  Return value:
  >=0         - Directory has been created.
  <0          - An error has occured.
*/

static int _FS_fat_create_directory(int Idx, FS_u32 Unit, const char *pDirName,
                                    FS_u32 DirStart, FS_u32 DirSize) {
  char *buffer;
  FS__fat_dentry_type *s;
  FS_u32 dirindex;
  FS_u32 dsec;
  FS_i32 cluster;
  FS_u16 val_time;
  FS_u16 val_date;
  int err;
  int len;
  int j;

  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return -1;
  }
  len = FS__CLIB_strlen(pDirName);
  if (len > 11) {
    len = 11;
  }
  /* Read directory */
  for (dirindex = 0; dirindex < DirSize; dirindex++) {
    dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, dirindex);
    if (dsec == 0) {
      /* Translation of relativ directory sector to an absolute sector failed */
      FS__fat_free(buffer);
      return -1;
    }
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Read directory sector */
    if (err < 0) {
      /* Read error */
      FS__fat_free(buffer);
      return -1;
    }
    /* Scan the directory sector for a free or deleted entry */
    s = (FS__fat_dentry_type*)buffer;
    while (1) {
      if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
        break;  /* End of sector reached */
      }
      if (s->data[0] == 0x00) {
        break;  /* Found a free entry */
      }
      if (s->data[0] == (unsigned char)0xe5) {
        break;  /* Found a deleted entry */
      }
      s++;
    }
    if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
      /* Free entry found. Make entry and return 1st block of the file. */
      FS__CLIB_strncpy((char*)s->data, pDirName, len);
       s->data[11] = FS_FAT_ATTR_DIRECTORY;
      cluster = FS__fat_FAT_alloc(Idx, Unit, -1);              /* Alloc block in FAT */
      if (cluster >= 0) {
        s->data[12]     = 0x00;                                /* Res */
        s->data[13]     = 0x00;                                /* CrtTimeTenth (optional, not supported) */
        s->data[14]     = 0x00;                                /* CrtTime (optional, not supported) */
        s->data[15]     = 0x00;
        s->data[16]     = 0x00;                                /* CrtDate (optional, not supported) */
        s->data[17]     = 0x00;
        s->data[18]     = 0x00;                                /* LstAccDate (optional, not supported) */
        s->data[19]     = 0x00;
        val_time        = FS_X_OS_GetTime();
        s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
        s->data[23]     = (unsigned char)(val_time / 256);
        val_date        = FS_X_OS_GetDate();
        s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
        s->data[25]     = (unsigned char)(val_date / 256);
        s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */ 
        s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
        s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
        s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
        s->data[28]     = 0x00;                                /* FileSize */
        s->data[29]     = 0x00;
        s->data[30]     = 0x00;
        s->data[31]     = 0x00;
        err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Write the modified directory sector */
        if (err < 0) {
          FS__fat_free(buffer);
          return -1;
        }
        /* Clear new directory and make '.' and '..' entries */
        /* Make "." entry */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        s = (FS__fat_dentry_type*)buffer;
        FS__CLIB_strncpy((char*)s->data, ".          ", 11);
        s->data[11]     = FS_FAT_ATTR_DIRECTORY;
        s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
        s->data[23]     = (unsigned char)(val_time / 256);
        s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
        s->data[25]     = (unsigned char)(val_date / 256);
        s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */ 
        s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
        s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
        s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
        /* Make entry ".." */
        s++;
        FS__CLIB_strncpy((char*)s->data, "..         ", 11);
        s->data[11]     = FS_FAT_ATTR_DIRECTORY;
        s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
        s->data[23]     = (unsigned char)(val_time / 256);
        s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
        s->data[25]     = (unsigned char)(val_date / 256);
        s->data[26]     = (unsigned char)(DirStart & 0xff);    /* FstClusLo / FstClusHi */ 
        s->data[27]     = (unsigned char)((DirStart / 256) & 0xff);
        s->data[20]     = (unsigned char)((DirStart / 0x10000L) & 0xff);
        s->data[21]     = (unsigned char)((DirStart / 0x1000000L) & 0xff);
        dsec = FS__fat_dir_realsec(Idx, Unit, cluster, 0); /* Find 1st absolute sector of the new directory */
        if (dsec == 0) {
          FS__fat_free(buffer);
          return -1;
        }
        /* Write "." & ".." entries into the new directory */
        err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0) {
          FS__fat_free(buffer);
          return -1;
        }
        /* Clear rest of the directory cluster */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        for (j = 1; j < FS__FAT_aBPBUnit[Idx][Unit].SecPerClus; j++) {
          dsec = FS__fat_dir_realsec(Idx, Unit, cluster, j);
          err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
          if (err < 0) {
            FS__fat_free(buffer);
            return -1;
          }
        }
        FS__fat_free(buffer);
        return 1;

      }
      FS__fat_free(buffer);
      return -1;
    }
  }
  FS__fat_free(buffer);
  return -2;  /* Directory is full */
}


/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_opendir
*
  Description:
  FS internal function. Open an existing directory for reading.

  Parameters:
  pDirName    - Directory name. 
  pDir        - Pointer to a FS_DIR data structure. 
  
  Return value:
  ==0         - Unable to open the directory.
  !=0         - Address of an FS_DIR data structure.
*/

FS_DIR *FS__fat_opendir(const char *pDirName, FS_DIR *pDir) {
  FS_size_t len;
  FS_u32 unit;
  FS_u32 dstart;
  FS_u32 dsize;
  FS_i32 i;
  char realname[12];
  char *filename;

  if (!pDir) {
    return 0;  /* No valid pointer to a FS_DIR structure */
  }
  /* Find path on the media and return file name part of the complete path */
  dsize = FS__fat_findpath(pDir->dev_index, pDirName, &filename, &unit, &dstart); 
  if (dsize == 0) {
    return 0;  /* Directory not found */
  }
  FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0); /* Turn on busy signal */
  len = FS__CLIB_strlen(filename);
  if (len != 0) {
    /* There is a name in the complete path (it does not end with a '\') */
    FS__fat_make_realname(realname, filename);  /* Convert name to FAT real name */
    i =  FS__fat_find_dir(pDir->dev_index, unit, realname, dstart, dsize);  /* Search name in the directory */
    if (i == 0) {
      /* Directory not found */
      FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
      return 0;
    }
  }
  else {
    /* 
       There is no name in the complete path (it does end with a '\'). In that
       case, FS__fat_findpath returns already start of the directory.
    */
    i = dstart;  /* Use 'current' path */
  }
  if (i) {
    dsize  =  FS__fat_dir_size(pDir->dev_index, unit, i);  /* Get size of the directory */
  }
  if (dsize == 0) {
    /* Directory not found */
    FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    return 0;
  }
  pDir->dirid_lo  = unit;
  pDir->dirid_hi  = i;
  pDir->dirid_ex  = dstart;
  pDir->error     = 0;
  pDir->size      = dsize;
  pDir->dirpos    = 0;
  pDir->inuse     = 1;
  return pDir;
}


/*********************************************************************
*
*             FS__fat_closedir
*
  Description:
  FS internal function. Close a directory referred by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure. 
  
  Return value:
  ==0         - Directory has been closed.
  ==-1        - Unable to close directory.
*/

int FS__fat_closedir(FS_DIR *pDir) {
  if (!pDir) {
    return -1;  /* No valid pointer to a FS_DIR structure */
  }
  FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
  pDir->inuse = 0;
  return 0;
}


/*********************************************************************
*
*             FS__fat_readdir
*
  Description:
  FS internal function. Read next directory entry in directory 
  specified by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure. 
  
  Return value:
  ==0         - No more directory entries or error.
  !=0         - Pointer to a directory entry.
*/

struct FS_DIRENT *FS__fat_readdir(FS_DIR *pDir) {
  FS__fat_dentry_type *s;
  FS_u32 dirindex;
  FS_u32 dsec;
  FS_u16 bytespersec;
  char *buffer;
  int err;

  if (!pDir) {
    return 0;  /* No valid pointer to a FS_DIR structure */
  }
  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return 0;
  }
  bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
  dirindex = pDir->dirpos / bytespersec;
  while (dirindex < (FS_u32)pDir->size) {
    dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, dirindex);
    if (dsec == 0) {
      /* Cannot convert logical sector */
      FS__fat_free(buffer);
      return 0;
    }
    /* Read directory sector */
    err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
    if (err < 0) {
      FS__fat_free(buffer);
      return 0;
    }
    /* Scan for valid directory entry */
    s = (FS__fat_dentry_type*)&buffer[pDir->dirpos % bytespersec];
    while (1) {
      if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
        break;  /* End of sector reached */
      }
      if (s->data[11] != 0x00) { /* not an empty entry */
        if (s->data[0] != (unsigned char)0xe5) { /* not a deleted file */
          if (s->data[11] != (FS_FAT_ATTR_READ_ONLY | FS_FAT_ATTR_HIDDEN | FS_FAT_ATTR_SYSTEM | FS_FAT_VOLUME_ID)) {
            break;  /* Also not a long entry, so it is a valid entry */
          }
        }
      }
      s++;
      pDir->dirpos += 32;
    }
    if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
      /* Valid entry found, copy it.*/
      pDir->dirpos += 32;
      FS__CLIB_memcpy(pDir->dirent.d_name, s->data, 8);
      pDir->dirent.d_name[8] = '.';
      FS__CLIB_memcpy(&pDir->dirent.d_name[9], &s->data[8], 3);
      pDir->dirent.d_name[12] = 0;
      pDir->dirent.FAT_DirAttr = s->data[11];
      FS__fat_free(buffer);
      return &pDir->dirent;
    }
    dirindex++;
  }
  FS__fat_free(buffer);
  return 0;
}


/*********************************************************************
*
*             FS__fat_MkRmDir
*
  Description:
  FS internal function. Create or remove a directory. If you call this 
  function to remove a directory (MkDir==0), you must make sure, that 
  it is already empty.

  Parameters:
  pDirName    - Directory name. 
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  MkDir       - ==0 => Remove directory.
                !=0 => Create directory.
  
  Return value:
  ==0         - Directory has been created.
  ==-1        - An error has occured.
*/

int  FS__fat_MkRmDir(const char *pDirName, int Idx, char MkDir) {
  FS_size_t len;
  FS_u32 dstart;
  FS_u32 dsize;
  FS_u32 unit;
  FS_i32 i;
  int lexp_a;
  int lexp_b;
  char realname[12];
  char *filename;

  if (Idx < 0) {
    return -1; /* Not a valid index */
  }
  dsize = FS__fat_findpath(Idx, pDirName, &filename, &unit, &dstart);
  if (dsize == 0) {
    return -1;  /* Path not found */
  }
  FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0); /* Turn on busy signal */
  len = FS__CLIB_strlen(filename);
  if (len != 0) {
    FS__fat_make_realname(realname, filename);  /* Convert name to FAT real name */
    i =  FS__fat_find_dir(Idx, unit, realname, dstart, dsize);
    lexp_a = (i!=0) && (MkDir);  /* We want to create a direcory , but it does already exist */
    lexp_b = (i==0) && (!MkDir); /* We want to remove a direcory , but it does not exist */
    lexp_a = lexp_a || lexp_b;
    if (lexp_a) {
      /* We want to create, but dir does already exist or we want to remove, but dir is not there */
      /* turn off busy signal */
      FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
      return -1;
    }
  }
  else {
    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    return -1;
  }
  /* 
      When you get here, variables have following values:
       dstart="current"  
       dsize="size of current"  
       realname="real dir name to create" 
  */
  if (MkDir) {
    i = _FS_fat_create_directory(Idx, unit,realname, dstart, dsize);  /* Create the directory */
  }
  else {
    i = FS__fat_DeleteFileOrDir(Idx, unit, realname, dstart, dsize, 0);  /* Remove the directory */
  }
  if (i >= 0) {
    /* If the operation has been successfull, flush the cache.*/
    i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_FLUSH_CACHE, 2, (void*)0);
    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn of busy signal */
    if (i < 0) {
      return -1;
    }
    return 0;
  }
  FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn of busy signal */
  return -1;
}


#endif /* FS_POSIX_DIR_SUPPORT */

