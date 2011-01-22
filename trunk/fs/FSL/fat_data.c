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
File        : fat_data.c
Purpose     : File system's FAT File System Layer data buffers
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
#include "fs_fat.h"

/*********************************************************************
*
*             Global Variables
*
**********************************************************************
*/

FS__FAT_BPB FS__FAT_aBPBUnit[FS_MAXDEV][FS_FAT_MAXUNIT];	//fs_maxdevΪ1 maxunitΪ2


