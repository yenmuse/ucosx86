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
File        : ide.h
Purpose     : Header file for the file system's IDE driver
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __IDE_H__
#define __IDE_H__

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

int FS__IDE_Init(FS_u32 Unit);
int FS__IDE_ReadSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer);
int FS__IDE_WriteSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer);

#endif  /* __IDE_H__ */


