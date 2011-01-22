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
File        : fat_in.c
Purpose     : FAT read routines
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


/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_fread
*
  Description:
  FS internal function. Read data from a file.

  Parameters:
  pData       - Pointer to a data buffer for storing data transferred
                from file. 
  Size        - Size of an element to be transferred from file to data
                buffer
  N           - Number of elements to be transferred from the file.
  pFile       - Pointer to a FS_FILE data structure.
  
  Return value:
  Number of elements read.
*/

FS_size_t FS__fat_fread(void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile) {
  FS_size_t todo;
  FS_u32 i;
  FS_u32 j;
  FS_u32 fatsize;
  FS_u32 fileclustnum;
  FS_u32 diskclustnum;
  FS_u32 prevclust;
  FS_u32 dstart;
  FS_u32 dsize;
  FS_u32 datastart;
  char *buffer;
  int err;
                                            
  if (!pFile) {
      return 0;  /* No valid pointer to a FS_FILE structure */
  }
  /* Check if media is OK */
  err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
  if (err == FS_LBL_MEDIACHANGED) {
    /* Media has changed */
    pFile->error = FS_ERR_DISKCHANGED;
    return 0;
  }
  else if (err < 0) {
    /* Media cannot be accessed */
    pFile->error = FS_ERR_READERROR;
    return 0;
  }
  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return 0;
  }
  fatsize = FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].FATSz16;
  if (fatsize == 0) {
    /* FAT32 */
     fatsize = FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].FATSz32;
  }
  dstart    = FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].RsvdSecCnt + FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].NumFATs * fatsize;
  dsize     = ((FS_u32)((FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].RootEntCnt) * FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE;
  datastart = dstart + dsize;
  prevclust = 0;
  todo = N * Size;
  while (todo) {
    if (pFile->filepos >= pFile->size) {
      /* EOF has been reached */
      pFile->error = FS_ERR_EOF;
      FS__fat_free(buffer);
      return ((N * Size - todo) / Size);
    }
    fileclustnum = pFile->filepos / (FS_FAT_SEC_SIZE * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus);
    if (prevclust == 0) {
      diskclustnum = pFile->CurClust; 
      if (diskclustnum == 0) {
        /* Find current cluster by starting at 1st cluster of the file */
        diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, fileclustnum);
      }
    }
    else {
      /* Get next cluster of the file */
      diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
    }
    prevclust       = diskclustnum;
    pFile->CurClust = diskclustnum;
    if (diskclustnum == 0) {
      /* Could not find current cluster */
      pFile->error = FS_ERR_READERROR;
      FS__fat_free(buffer);
      return ((N * Size - todo) / Size);
    }
    diskclustnum -= 2;
    j = (pFile->filepos % (FS_FAT_SEC_SIZE * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus))/ FS_FAT_SEC_SIZE;
    while (1) {
      if (!todo) {
        break;  /* Nothing more to write */
      }
      if (j >= (FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus) {
        break;  /* End of the cluster reached */
      }
      if (pFile->filepos >= pFile->size) {
        break;  /* End of the file reached */
      }
      err = FS__lb_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                    datastart +
                    diskclustnum * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus + j,
                    (void*)buffer);
      if (err < 0) {
        pFile->error = FS_ERR_READERROR;
        FS__fat_free(buffer);
        return ((N * Size - todo) / Size);
      }
      i = pFile->filepos % FS_FAT_SEC_SIZE;
      while (1) {
        if (!todo) {
          break;  /* Nothing more to write */
        }
        if (i >= FS_FAT_SEC_SIZE) {
          break;  /* End of the sector reached */
        }
        if (pFile->filepos >= pFile->size) {
          break;  /* End of the file reached */
        }
        *((char*)(((char*)pData) + N * Size - todo)) = buffer[i];
        i++;
        pFile->filepos++;
        todo--;
      }
      j++;
    }  /* Sector loop */
  }  /* Cluster loop */
  if (i >= FS_FAT_SEC_SIZE) {
    if (j >= FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus) {
      pFile->CurClust = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
    }
  }
  FS__fat_free(buffer);
  return ((N * Size - todo) / Size);
}

