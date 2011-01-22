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
File        : fat_open.c
Purpose     : FAT routines for open/delete files
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
#ifndef FS_FARCHARPTR
#define FS_FARCHARPTR char *
#endif
#include "fs_dev.h"
#include "fs_api.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_lbl.h"
#include "fs_fat.h"
#include "fs_clib.h"


/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

#ifndef FS_FAT_NOFAT32
  #define FS_FAT_NOFAT32        0
#endif /* FS_FAT_NOFAT32 */


/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*             _FS_fat_find_file
*
  Description:
  FS internal function. Find the file with name pFileName in directory
  DirStart. Copy its directory entry to pDirEntry.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pFileName   - File name. 
  pDirEntry   - Pointer to an FS__fat_dentry_type data structure.
  DirStart    - 1st cluster of the directory.
  DirSize     - Sector (not cluster) size of the directory.
 
  Return value:
  >=0         - File found. Value is the first cluster of the file.
  <0          - An error has occured.
*/

static FS_i32 _FS_fat_find_file(int Idx, FS_u32 Unit, const char *pFileName,
                                    FS__fat_dentry_type *pDirEntry,
                                    FS_u32 DirStart, FS_u32 DirSize) {
  FS__fat_dentry_type *s;
  FS_u32 i;
  FS_u32 dsec;
  int len;
  int err; 
  int c;
  char *buffer;

  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return -1;
  }
  len = FS__CLIB_strlen(pFileName);
  if (len > 11) {
    len = 11;
  }
  /* Read directory */
  for (i = 0; i < DirSize; i++) {
    dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
    if (dsec == 0) {
      FS__fat_free(buffer);
      return -1;
    }
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    if (err < 0) {
      FS__fat_free(buffer);
      return -1;
    }
    s = (FS__fat_dentry_type*)buffer;
    while (1) {
      if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
        break;  /* End of sector reached */
      }
      c = FS__CLIB_strncmp((char*)s->data, pFileName, len);
      if (c == 0) {  /* Name does match */
        if (s->data[11] & FS_FAT_ATTR_ARCHIVE) {
          break;  /* Entry found */
        }
      }
      s++;
    }
    if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
      /* Entry found. Return number of 1st block of the file */
      if (pDirEntry) {
        FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
      }
      FS__fat_free(buffer);
      dsec  = (FS_u32)s->data[26];
      dsec += (FS_u32)s->data[27] * 0x100UL;
      dsec += (FS_u32)s->data[20] * 0x10000UL;
      dsec += (FS_u32)s->data[21] * 0x1000000UL;
      return ((FS_i32)dsec);
    }
  }
  FS__fat_free(buffer);
  return -1;
}


/*********************************************************************
*
*             _FS_fat_IncDir
*
  Description:
  FS internal function. Increase directory starting at DirStart.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory.
  pDirSize    - Pointer to an FS_u32, which is used to return the new 
                sector (not cluster) size of the directory.
 
  Return value:
  ==1         - Success.
  ==-1        - An error has occured.
*/

static int _FS_fat_IncDir(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize) {
  FS_u32 i;
  FS_u32 dsec;
  FS_i32 last;
  char *buffer;
  int err;

  if (DirStart == 0) { 
    /* Increase root directory only, if not FAT12/16  */
    i = FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt;
    if (i != 0) {
      return -1;  /* Not FAT32 */
    }
  }
  last = FS__fat_FAT_find_eof(Idx, Unit, DirStart, 0);
  if (last < 0) {
    return -1;  /* No EOF marker found */
  }
  last = FS__fat_FAT_alloc(Idx, Unit, last);  /* Allocate new cluster */
  if (last < 0) {
    return -1;
  }
  *pDirSize = *pDirSize + FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
  /* Clean new directory cluster */
  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return -1;
  }
  FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
  for (i = *pDirSize - FS__FAT_aBPBUnit[Idx][Unit].SecPerClus; i < *pDirSize; i++) {
    dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
    if (dsec == 0) {
      FS__fat_free(buffer);
      return -1;
    }
    err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    if (err < 0) {
      FS__fat_free(buffer);
      return -1;
    }
  }
  FS__fat_free(buffer);
  return 1;
}


/*********************************************************************
*
*             _FS_fat_create_file
*
  Description:
  FS internal function. Create a file in the directory specified
  with DirStart. Do not call, if you have not checked before for 
  existing file with name pFileName.

  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pFileName   - File name. 
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Sector size of the directory starting at DirStart.
  
  Return value:
  >=0         - 1st cluster of the new file.
  ==-1        - An error has occured.
  ==-2        - Cannot create, because directory is full.
*/

static FS_i32 _FS_fat_create_file(int Idx, FS_u32 Unit,  const char *pFileName,
                                    FS_u32 DirStart, FS_u32 DirSize) {
  FS__fat_dentry_type *s;
  FS_u32 i;
  FS_u32 dsec;
  FS_i32 cluster;
  int len;
  int err;
  FS_u16 val;
  char *buffer;

  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return -1;
  }
  len = FS__CLIB_strlen(pFileName);
  if (len > 11) {
    len = 11;
  }
  /* Read directory */
  for (i = 0; i < DirSize; i++) {
    dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
    if (dsec == 0) {
      FS__fat_free(buffer);
      return -1;
    }
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    if (err < 0) {
      FS__fat_free(buffer);
      return -1;
    }
    s = (FS__fat_dentry_type*)buffer;
    while (1) {
      if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
        break;  /* End of sector reached */
      }
      if (s->data[0] == 0x00) {
        break;  /* Empty entry found */
      }
      if (s->data[0] == (unsigned char)0xe5) {
        break;  /* Deleted entry found */
      }
      s++;
    }
    if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
      /* Free entry found. Make entry and return 1st block of the file */
      FS__CLIB_strncpy((char*)s->data, pFileName, len);
      s->data[11] = FS_FAT_ATTR_ARCHIVE;
      /* Alloc block in FAT */
      cluster = FS__fat_FAT_alloc(Idx, Unit, -1);
      if (cluster >= 0) {
        s->data[12]     = 0x00;                           /* Res */
        s->data[13]     = 0x00;                           /* CrtTimeTenth (optional, not supported) */
        s->data[14]     = 0x00;                           /* CrtTime (optional, not supported) */
        s->data[15]     = 0x00;
        s->data[16]     = 0x00;                           /* CrtDate (optional, not supported) */
        s->data[17]     = 0x00;
        s->data[18]     = 0x00;                           /* LstAccDate (optional, not supported) */
        s->data[19]     = 0x00;
        val             = FS_X_OS_GetTime();
        s->data[22]     = (unsigned char)(val & 0xff);   /* WrtTime */
        s->data[23]     = (unsigned char)(val / 256);
        val             = FS_X_OS_GetDate();
        s->data[24]     = (unsigned char)(val & 0xff);   /* WrtDate */
        s->data[25]     = (unsigned char)(val / 256);
        s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */ 
        s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
        s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
        s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
        s->data[28]     = 0x00;                           /* FileSize */
        s->data[29]     = 0x00;
        s->data[30]     = 0x00;
        s->data[31]     = 0x00;
        err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0) {
          FS__fat_free(buffer);
          return -1;
        }
      }
      FS__fat_free(buffer);
      return cluster;
    }
  }
  FS__fat_free(buffer);
  return -2;      /* Directory is full */
}


/*********************************************************************
*
*             Global functions section 1
*
**********************************************************************

  Functions in this section are global, but are used inside the FAT
  File System Layer only.
  
*/

/*********************************************************************
*
*             FS__fat_DeleteFileOrDir
*
  Description:
  FS internal function. Delete a file or directory.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pName       - File or directory name. 
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Sector size of the directory starting at DirStart.
  RmFile      - 1 => remove a file
                0 => remove a directory
  
  Return value:
  >=0         - Success. 
  <0          - An error has occured.
*/

FS_i32 FS__fat_DeleteFileOrDir(int Idx, FS_u32 Unit,  const char *pName,
                                    FS_u32 DirStart, FS_u32 DirSize, char RmFile) {
  FS__fat_dentry_type *s;
  FS_u32 dsec;
  FS_u32 i;
  FS_u32 value;
  FS_u32 fatsize;
  FS_u32 filesize;
  FS_i32 len;
  FS_i32 bytespersec;
  FS_i32 fatindex;
  FS_i32 fatsec;
  FS_i32 fatoffs;
  FS_i32 lastsec;
  FS_i32 curclst;
  FS_i32 todo;
  char *buffer;
  int fattype;
  int err;
  int err2;
  int lexp;
  int x;
  unsigned char a;
  unsigned char b;
#if (FS_FAT_NOFAT32==0)
  unsigned char c;
  unsigned char d;
#endif /* FS_FAT_NOFAT32==0 */

  buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
  if (!buffer) {
    return 0;
  }
  fattype = FS__fat_which_type(Idx, Unit);
#if (FS_FAT_NOFAT32!=0)
  if (fattype == 2) {
    FS__fat_free(buffer);
    return -1;
  }
#endif  /* FS_FAT_NOFAT32!=0 */
  fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
  if (fatsize == 0) {
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
  }
  bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
  len = FS__CLIB_strlen(pName);
  if (len > 11) {
    len = 11;
  }
  /* Read directory */
  for (i = 0; i < DirSize; i++) {
    curclst = -1;
    dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
    if (dsec == 0) {
      FS__fat_free(buffer);
      return -1;
    }
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    if (err < 0) {
      FS__fat_free(buffer);
      return -1;
    }
    /* Scan for pName in the directory sector */
    s = (FS__fat_dentry_type*) buffer;
    while (1) {
      if (s >= (FS__fat_dentry_type*)(buffer + bytespersec)) {
        break;  /* End of sector reached */
      }
      x = FS__CLIB_strncmp((char*)s->data, pName, len);
      if (x == 0) { /* Name does match */
        if (s->data[11] != 0) {
          break;  /* Entry found */
        }
      }
      s++;
    }
    if (s < (FS__fat_dentry_type*)(buffer + bytespersec)) {
      /* Entry has been found, delete directory entry */
      s->data[0]  = 0xe5;
      s->data[11] = 0;
      err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
      if (err < 0) {
        FS__fat_free(buffer);
        return -1;
      }
      /* Free blocks in FAT */
      /*
         For normal files, there are no more clusters freed than the entrie's filesize
         does indicate. That avoids corruption of the complete media in case there is
         no EOF mark found for the file (FAT is corrupt!!!). 
         If the function should remove a directory, filesize if always 0 and cannot
         be used for that purpose. To avoid running into endless loop, todo is set
         to 0x0ffffff8L, which is the maximum number of clusters for FAT32.
      */
      if (RmFile) {
        filesize  = s->data[28] + 0x100UL * s->data[29] + 0x10000UL * s->data[30] + 0x1000000UL * s->data[31];
        todo      = filesize / (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec);
        value     = filesize % (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec);
        if (value != 0) {
          todo++;
        }
      } 
      else {
        todo = (FS_i32)0x0ffffff8L;
      }
      curclst = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
      lastsec = -1;
      /* Free cluster loop */
      while (todo) {
        if (fattype == 1) {
          fatindex = curclst + (curclst / 2);    /* FAT12 */
        }
#if (FS_FAT_NOFAT32==0)
        else if (fattype == 2) {
          fatindex = curclst * 4;               /* FAT32 */
        }
#endif  /* FS_FAT_NOFAT32==0 */
        else {
          fatindex = curclst * 2;               /* FAT16 */
        }
        fatsec = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;
        if (fatsec != lastsec) {
          err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
          if (err < 0) {
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
            if (err < 0) {
              FS__fat_free(buffer);
              return -1;
            }
            /* Try to repair original FAT sector with contents of copy */
            FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
          }
          lastsec = fatsec;
        }
        if (fattype == 1) {
          if (fatoffs == (bytespersec - 1)) {
            a = buffer[fatoffs];
            if (curclst & 1) {
              buffer[fatoffs] &= 0x0f;
            }
            else {
              buffer[fatoffs]  = 0x00;
            }
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            err2 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
            lexp = (err < 0);
            lexp = lexp || (err2 < 0);
            if (lexp) {
              FS__fat_free(buffer);
              return -1;
            }
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
            if (err < 0) {
              err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
              if (err < 0) {
                FS__fat_free(buffer);
                return -1;
              }
              /* Try to repair original FAT sector with contents of copy */
              FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
            }
            lastsec = fatsec + 1;
            b = buffer[0];
            if (curclst & 1) {
              buffer[0]  = 0x00;;
            }
            else {
              buffer[0] &= 0xf0;
            }
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
            err2 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
            lexp = (err < 0);
            lexp = lexp || (err2 < 0);
            if (lexp) {
              FS__fat_free(buffer);
              return -1;
            }
          }
          else {
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            if (curclst & 1) {
              buffer[fatoffs]     &= 0x0f;
              buffer[fatoffs + 1]  = 0x00;
            }
            else {
              buffer[fatoffs]      = 0x00;
              buffer[fatoffs + 1] &= 0xf0;
            }
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            err2 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
            lexp = (err < 0);
            lexp = lexp || (err2 < 0);
            if (lexp) {
              FS__fat_free(buffer);
              return -1;
            }
          }
          if (curclst & 1) {
            curclst = ((a & 0xf0) >> 4) + 16 * b;
          }
          else {
            curclst = a + 256 * (b & 0x0f);
          }
          curclst &= 0x0fff;
          if (curclst >= 0x0ff8) {
            FS__fat_free(buffer);
            return 0;
          }
        }
#if (FS_FAT_NOFAT32==0)
        else if (fattype == 2) { /* FAT32 */
          a = buffer[fatoffs];
          b = buffer[fatoffs + 1];
          c = buffer[fatoffs + 2];
          d = buffer[fatoffs + 3] & 0x0f;
          buffer[fatoffs]      = 0x00;
          buffer[fatoffs + 1]  = 0x00;
          buffer[fatoffs + 2]  = 0x00;
          buffer[fatoffs + 3]  = 0x00;
          err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
          err2 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
          lexp = (err < 0);
          lexp = lexp || (err2 < 0);
          if (lexp) {
            FS__fat_free(buffer);
            return -1;
          }
          curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
          curclst &= 0x0fffffffL;
          if (curclst >= (FS_i32)0x0ffffff8L) {
            FS__fat_free(buffer);
            return 0;
          }
        }
#endif /* FS_FAT_NOFAT32==0 */
        else {
          a = buffer[fatoffs];
          b = buffer[fatoffs + 1];
          buffer[fatoffs]     = 0x00;
          buffer[fatoffs + 1] = 0x00;
          err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
          err2 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
          lexp = (err < 0);
          lexp = lexp || (err2 < 0);
          if (lexp) {
            FS__fat_free(buffer);
            return -1;
          }
          curclst  = a + 256 * b;
          curclst &= 0xffff;
          if (curclst >= (FS_i32)0xfff8) {
            FS__fat_free(buffer);
            return 0;
          }
        }
        todo--;
      } /* Free cluster loop */
    } /*  Delete entry */
    if (curclst > 0) {
      FS__fat_free(buffer);
      return curclst;
    }
  } /* for */
  FS__fat_free(buffer);
  return curclst;
}


/*********************************************************************
*
*             FS__fat_make_realname
*
  Description:
  FS internal function. Convert a given name to the format, which is
  used in the FAT directory.
  
  Parameters:
  pOrgName    - Pointer to name to be translated
  pEntryName  - Pointer to a buffer for storing the real name used
                in a directory.

  Return value:
  None.
*/

void FS__fat_make_realname(char *pEntryName, const char *pOrgName) {
  FS_FARCHARPTR ext;
  FS_FARCHARPTR s;
  int i;

  s = (FS_FARCHARPTR)pOrgName;
  ext = (FS_FARCHARPTR) FS__CLIB_strchr(s, '.');
  if (!ext) {
    ext = &s[FS__CLIB_strlen(s)];
  }
  i=0;
  while (1) {
    if (s >= ext) {
      break;  /* '.' reached */
    }
    if (i >= 8) {
      break;  /* If there is no '.', this is the end of the name */
    }
    if (*s == (char)0xe5) {
      pEntryName[i] = 0x05;
    }
    else {
      pEntryName[i] = (char)FS__CLIB_toupper(*s);
    }
    i++;
    s++;
  }
  while (i < 8) {
    /* Fill name with spaces*/
    pEntryName[i] = ' ';
    i++;
  }
  if (*s == '.') {
    s++;
  }
  while (i < 11) {
    if (*s != 0) {
      if (*s == (char)0xe5) {
        pEntryName[i] = 0x05;
      }
      else {
        pEntryName[i] = (char)FS__CLIB_toupper(*s);
      }
      s++;
    }
    else {
      pEntryName[i] = ' ';
    }
    i++;
  }
  pEntryName[11]=0;
}


/*********************************************************************
*
*             FS__fat_find_dir
*
  Description:
  FS internal function. Find the directory with name pDirName in directory
  DirStart.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pDirName    - Directory name; if zero, return the root directory.
  DirStart    - 1st cluster of the directory.
  DirSize     - Sector (not cluster) size of the directory.
 
  Return value:
  >0          - Directory found. Value is the first cluster of the file.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_find_dir(int Idx, FS_u32 Unit, char *pDirName, FS_u32 DirStart, 
                        FS_u32 DirSize) {
  FS__fat_dentry_type *s;
  FS_u32 dstart;
  FS_u32 i;
  FS_u32 dsec;
  FS_u32 fatsize;
  int len;
  int err;
  int c;
  char *buffer;

  if (pDirName == 0) {
    /* Return root directory */
    if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16) {
      fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
      dstart  = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    }
    else {
      fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
      dstart  = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize
                + (FS__FAT_aBPBUnit[Idx][Unit].RootClus - 2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    }
  }
  else {
    /* Find directory */
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer) {
      return 0;
    }
    len = FS__CLIB_strlen(pDirName);
    if (len > 11) {
      len = 11;
    }
    /* Read directory */
    for (i = 0; i < DirSize; i++) {
      dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
      if (dsec == 0) {
        FS__fat_free(buffer);
        return 0;
      }
      err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
      if (err < 0) {
        FS__fat_free(buffer);
        return 0;
      }
      s = (FS__fat_dentry_type*)buffer;
      while (1) {
        if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
          break;  /* End of sector reached */
        }
        c = FS__CLIB_strncmp((char*)s->data, pDirName, len);
        if (c == 0) { /* Name does match */
          if (s->data[11] & FS_FAT_ATTR_DIRECTORY) {
            break;  /* Entry found */
          }
        }
        s++;
      }
      if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE)) {
        /* Entry found. Return number of 1st block of the directory */
        FS__fat_free(buffer);
        dstart  = (FS_u32)s->data[26];
        dstart += (FS_u32)0x100UL * s->data[27];
        dstart += (FS_u32)0x10000UL * s->data[20];
        dstart += (FS_u32)0x1000000UL * s->data[21];
        return dstart;
      }
    }
    dstart = 0;
    FS__fat_free(buffer);
  }
  return dstart;
}


/*********************************************************************
*
*             FS__fat_dir_realsec
*
  Description:
  FS internal function. Translate a directory relative sector number
  to a real sector number on the media.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory. This is zero to address 
                the root directory. 
  DirSec      - Sector in the directory.
 
  Return value:
  >0          - Directory found. Value is the sector number on the media.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_dir_realsec(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 DirSec) {
  FS_u32 rootdir;
  FS_u32 rsec;
  FS_u32 dclust;
  FS_u32 fatsize;
  int fattype;
  int lexp;
  unsigned char secperclus;

  fattype = FS__fat_which_type(Idx, Unit);
  lexp = (0 == DirStart);
  lexp = lexp && (fattype != 2);
  if (lexp) {
    /* Sector in FAT12/FAT16 root directory */
    rootdir = FS__fat_find_dir(Idx, Unit, 0, 0, 0);
    rsec = rootdir + DirSec;
  }
  else {
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0) {
      fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }
    secperclus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    dclust = DirSec / secperclus;
    if (0 == DirStart) {
      /* FAT32 root directory */
      rsec = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
    } 
    else {
      rsec = FS__fat_diskclust(Idx, Unit, DirStart, dclust);
      if (rsec == 0) {
        return 0;
      }
    }
    rsec -= 2;
    rsec *= secperclus;
    rsec += FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    rsec += ((FS_u32)((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) * FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE;
    rsec += (DirSec % secperclus);
  }
  return rsec;
}


/*********************************************************************
*
*             FS__fat_dirsize
*
  Description:
  FS internal function. Return the sector size of the directory 
  starting at DirStart.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory. This is zero to address 
                the root directory. 
 
  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_dir_size(int Idx, FS_u32 Unit, FS_u32 DirStart) {
  FS_u32 dsize;
  FS_i32 value;

  if (DirStart == 0) {
    /* For FAT12/FAT16 root directory, the size can be found in BPB */
    dsize = ((FS_u32)((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt)
            * FS_FAT_DENTRY_SIZE) / ((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec);
    if (dsize == 0) {
      /* Size in BPB is 0, so it is a FAT32 (FAT32 does not have a real root dir) */
      value = FS__fat_FAT_find_eof(Idx, Unit, FS__FAT_aBPBUnit[Idx][Unit].RootClus, &dsize);
      if (value < 0) {
        dsize = 0;
      }
      else {
        dsize *= FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
      }
    }
  }
  else {
    /* Calc size of a sub-dir */
    value = FS__fat_FAT_find_eof(Idx, Unit, DirStart, &dsize);
    if (value < 0) {
      dsize = 0;
    }
    else {
      dsize *= FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    }
  }
  return dsize;
}


/*********************************************************************
*
*             FS__fat_findpath
*
  Description:
  FS internal function. Return start cluster and size of the directory
  of the file name in pFileName.
  
  Parameters:
  Idx         - Index of device in the device information table 
                referred by FS__pDevInfo.
  pFullName   - Fully qualified file name w/o device name.
  pFileName   - Pointer to a pointer, which is modified to point to the
                file name part of pFullName.
  pUnit       - Pointer to an FS_u32 for returning the unit number.
  pDirStart   - Pointer to an FS_u32 for returning the start cluster of
                the directory.

  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_findpath(int Idx, const char *pFullName, FS_FARCHARPTR *pFileName, 
                        FS_u32 *pUnit, FS_u32 *pDirStart) {
  FS_u32 dsize;
  FS_i32 i;
  FS_i32 j;
  FS_FARCHARPTR dname_start;
  FS_FARCHARPTR dname_stop;
  FS_FARCHARPTR chprt;
  int x;
  char dname[12];
  char realname[12];

  /* Find correct unit (unit:name) */
  *pFileName = (FS_FARCHARPTR)FS__CLIB_strchr(pFullName, ':');
  if (*pFileName) {
    /* Scan for unit number */
    *pUnit = FS__CLIB_atoi(pFullName);
    (*pFileName)++;
  }
  else {
    /* Use 1st unit as default */
    *pUnit = 0;
    *pFileName = (FS_FARCHARPTR) pFullName;
  }
  /* Check volume */
  x = !FS__fat_checkunit(Idx, *pUnit);
  if (x) {
    return 0;
  }
  /* Setup pDirStart/dsize for root directory */
  *pDirStart = 0;
  dsize      = FS__fat_dir_size(Idx, *pUnit, 0);
  /* Find correct directory */
  do {
    dname_start = (FS_FARCHARPTR)FS__CLIB_strchr(*pFileName, '\\');
    if (dname_start) {
      dname_start++;
      *pFileName = dname_start;
      dname_stop = (FS_FARCHARPTR)FS__CLIB_strchr(dname_start, '\\');
    }
    else {
      dname_stop = 0;
    }
    if (dname_stop) {
      i = dname_stop-dname_start;
      if (i >= 12) {
        j = 0;
        for (chprt = dname_start; chprt < dname_stop; chprt++) {
          if (*chprt == '.') {
            i--;
          }
          else if (j < 12) {
            realname[j] = *chprt;
            j++;
          }
        }
        if (i >= 12) {
          return 0;
        }
      }
      else {
        FS__CLIB_strncpy(realname, dname_start, i);
      }
      realname[i] = 0;
      FS__fat_make_realname(dname, realname);
      *pDirStart =  FS__fat_find_dir(Idx, *pUnit, dname, *pDirStart, dsize);
      if (*pDirStart) {
        dsize  =  FS__fat_dir_size(Idx, *pUnit, *pDirStart);
      }
      else {
        dsize = 0;    /* Directory NOT found */
      }
    }
  } while (dname_start);
  return dsize;
}


/*********************************************************************
*
*             Global functions section 2
*
**********************************************************************

  These are real global functions, which are used by the API Layer
  of the file system.
  
*/

/*********************************************************************
*
*             FS__fat_fopen
*
  Description:
  FS internal function. Open an existing file or create a new one.

  Parameters:
  pFileName   - File name. 
  pMode       - Mode for opening the file.
  pFile       - Pointer to an FS_FILE data structure.
  
  Return value:
  ==0         - Unable to open the file.
  !=0         - Address of the FS_FILE data structure.
*/

FS_FILE *FS__fat_fopen(const char *pFileName, const char *pMode, FS_FILE *pFile) {
  FS_u32 unit;
  FS_u32 dstart;
  FS_u32 dsize;
  FS_i32 i;
  FS_FARCHARPTR fname;
  FS__fat_dentry_type s;
  char realname[12];
  int lexp_a;
  int lexp_b;
  
  if (!pFile) {
    return 0;  /* Not a valid pointer to an FS_FILE structure*/
  }
  dsize = FS__fat_findpath(pFile->dev_index, pFileName, &fname, &unit, &dstart);
  if (dsize == 0) {
    return 0;  /* Directory not found */
  }
  FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  /* Turn on busy signal */
  FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name */
  /* FileSize = 0 */
  s.data[28] = 0x00;      
  s.data[29] = 0x00;
  s.data[30] = 0x00;
  s.data[31] = 0x00;
  i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize);
  /* Delete file */
  lexp_b = (FS__CLIB_strcmp(pMode, "del") == 0);    /* Delete file request */
  lexp_a = lexp_b && (i >= 0);                      /* File does exist */
  if (lexp_a) {
    i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, 1);
    if (i != 0) {
      pFile->error = -1;
    }
    else {
      pFile->error = 0;
    }
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_FLUSH_CACHE, 2, (void*)0);
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    return 0;
  }
  else if (lexp_b) {
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    pFile->error = -1;
    return 0;
  }
  /* Check read only */
  lexp_a = ((i >= 0) && ((s.data[11] & FS_FAT_ATTR_READ_ONLY) != 0)) &&
          ((pFile->mode_w) || (pFile->mode_a) || (pFile->mode_c));
  if (lexp_a) {
    /* Files is RO and we try to create, write or append */
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    return 0;
  }
  lexp_a = ( i>= 0) && (!pFile->mode_a) && (((pFile->mode_w) && (!pFile->mode_r)) || 
          ((pFile->mode_w) && (pFile->mode_c) && (pFile->mode_r)) );
  if (lexp_a) {
    /* Delete old file */
    i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, 1);
    /* FileSize = 0 */
    s.data[28] = 0x00;      
    s.data[29] = 0x00;
    s.data[30] = 0x00;
    s.data[31] = 0x00;
    i=-1;
  }
  if ((!pFile->mode_c) && (i < 0)) {
    /* File does not exist and we must not create */
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    return 0;
  }
  else if ((pFile->mode_c) && (i < 0)) {
    /* Create new file */
    i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize);
    if (i < 0) {
      /* Could not create file */
      if (i == -2) {
        /* Directory is full, try to increase */
        i = _FS_fat_IncDir(pFile->dev_index, unit, dstart, &dsize);
        if (i > 0) {
          i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize);
        }
      }
      if (i < 0) {
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        return 0;
      }
    }
  }
  pFile->fileid_lo  = unit;
  pFile->fileid_hi  = i;
  pFile->fileid_ex  = dstart;
  pFile->EOFClust   = -1;
  pFile->CurClust   = 0;
  pFile->error      = 0;
  pFile->size       = (FS_u32)s.data[28];   /* FileSize */
  pFile->size      += (FS_u32)0x100UL * s.data[29];
  pFile->size      += (FS_u32)0x10000UL * s.data[30];
  pFile->size      += (FS_u32)0x1000000UL * s.data[31];
  if (pFile->mode_a) {
    pFile->filepos   = pFile->size;
  }
  else {
    pFile->filepos   = 0;
  }
  pFile->inuse     = 1;
  return pFile;
}

