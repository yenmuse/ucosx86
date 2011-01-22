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
File        : ide_x_hw.h
Purpose     : IDE hardware layer for Segger SED137XE board accessing
              a CF card in true IDE mode with P7,P8,P9,P10.
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __IDE_X_HW_H__
#define __IDE_X_HW_H__

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

/* Control line functions */
 void FS_IDE_HW_X_BusyLedOff(FS_u32 Unit);
 void FS_IDE_HW_X_BusyLedOn(FS_u32 Unit);
void FS_IDE_HW_X_HWReset(FS_u32 Unit);

/* ATA I/O register access functions */
unsigned char FS_IDE_HW_X_GetAltStatus(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetCylHigh(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetCylLow(FS_u32 Unit);
FS_u16 FS_IDE_HW_X_GetData(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetDevice(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetError(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetSectorCount(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetSectorNo(FS_u32 Unit);
unsigned char FS_IDE_HW_X_GetStatus(FS_u32 Unit);

void FS_IDE_HW_X_SetCommand(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetCylHigh(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetCylLow(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetData(FS_u32 Unit, FS_u16 data);
void FS_IDE_HW_X_SetDevControl(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetDevice(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetFeatures(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetSectorCount(FS_u32 Unit, unsigned char Data);
void FS_IDE_HW_X_SetSectorNo(FS_u32 Unit, unsigned char Data);

/* Status detection functions */
char FS_IDE_HW_X_DetectStatus(FS_u32 Unit);


int FS__IDE_Init(FS_u32 Unit);
int FS__IDE_ReadSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer);
int FS__IDE_WriteSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer);


#endif  /* __IDE_X_HW_H__ */


