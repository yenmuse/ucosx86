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
File        : ide_drv.c
Purpose     : File system generic IDE driver
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

#include "fs_port.h"
#include "fs_dev.h" 
#include "fs_lbl.h" 
#include "fs_conf.h"

#if FS_USE_IDE_DRIVER

#include "fs_api.h"
#include "ide_x_hw.h"
#include "ide.h"


/*********************************************************************
*
*             Local Variables        
*
**********************************************************************
*/

static FS_u32   _FS_ide_logicalstart[FS_IDE_MAXUNIT];     /* start of partition */
static char     _FS_ide_mbrbuffer[0x200];                 /* buffer for reading MBR */   
static char     _FS_ide_diskchange[FS_IDE_MAXUNIT];       /* signal flag for driver */
static char     _FS_ide_busycnt[FS_IDE_MAXUNIT];          /* counter for BSY LED on/off */


/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*             _FS_IDE_DevStatus
*
  Description:
  FS driver function. Get status of the media.

  Parameters:
  Unit        - Unit number.
 
  Return value:
  ==1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
  ==0                       - Device okay and ready for operation.
  <0                        - An error has occured.
*/

static int _FS_IDE_DevStatus(FS_u32 Unit) {
  static int init;
  int x;
  char a;

  if (!init) {
    /* 
       The function is called the first time. For each unit,
       the flag for 'diskchange' is set. That makes sure, that
       FS_LBL_MEDIACHANGED is returned, if there is already a
       media in the reader.
    */
    for (init = 0; init < FS_IDE_MAXUNIT; init++) {
      _FS_ide_diskchange[init] = 1;
    }
    init = 1;
  }
  if (Unit >= FS_IDE_MAXUNIT) {
    return -1;  /* No valid unit number */
  }
  a = FS_IDE_HW_X_DetectStatus(Unit);  /* Check if a card is present */
  if (a) {
    return -1;  /* No card in reader */
  }
  
  /* When you get here, then there is a card in the reader */
   a = _FS_ide_diskchange[Unit];  // Check if the media has changed 
  if (a) {
     
     //  A diskchange took place. The following code reads the MBR of the
     //  card to get its partition information.
    
    _FS_ide_diskchange[Unit] = 0;  // Reset 'diskchange' flag 
    FS__IDE_Init(Unit);
    x = FS__IDE_ReadSector(Unit, 0, (unsigned char*)&_FS_ide_mbrbuffer[0]);
    if (x != 0) {
      return -1;
    }
    // Calculate start sector of the first partition 
    _FS_ide_logicalstart[Unit]  = _FS_ide_mbrbuffer[0x1c6];
    _FS_ide_logicalstart[Unit] += (0x100UL * _FS_ide_mbrbuffer[0x1c7]);
    _FS_ide_logicalstart[Unit] += (0x10000UL * _FS_ide_mbrbuffer[0x1c8]);
    _FS_ide_logicalstart[Unit] += (0x1000000UL * _FS_ide_mbrbuffer[0x1c9]);
    return FS_LBL_MEDIACHANGED;
  }
  return 0;
}


/*********************************************************************
*
*             _FS_IDE_DevRead
*
  Description:
  FS driver function. Read a sector from the media.

  Parameters:
  Unit        - Unit number.
  Sector      - Sector to be read from the device.
  pBuffer     - Pointer to buffer for storing the data.
 
  Return value:
  ==0         - Sector has been read and copied to pBuffer.
  <0          - An error has occured.
*/

static int _FS_IDE_DevRead(FS_u32 Unit, FS_u32 Sector, void *pBuffer) {
  int x;
 
  if (Unit >= FS_IDE_MAXUNIT) {
    return -1;  /* No valid unit number */
  }
  x = FS__IDE_ReadSector(Unit, Sector + _FS_ide_logicalstart[Unit], (unsigned char*)pBuffer);
 
  if (x != 0) {
    x = -1;
  }
  return x;
}


/*********************************************************************
*
*             _FS_IDE_DevWrite
*
  Description:
  FS driver function. Write sector to the media.

  Parameters:
  Unit        - Unit number.
  Sector      - Sector to be written to the device.
  pBuffer     - Pointer to data to be stored.
 
  Return value:
  ==0         - Sector has been written to the device.
  <0          - An error has occured.
*/


static int _FS_IDE_DevWrite(FS_u32 Unit, FS_u32 Sector, void *pBuffer) {
  int x;
  
  if (Unit >= FS_IDE_MAXUNIT) {
    return -1;  /* No valid unit number */
  }
 
  x = FS__IDE_WriteSector(Unit, Sector + _FS_ide_logicalstart[Unit], (unsigned char*)pBuffer);

  if (x != 0) {
    x = -1;
  } 
  
  return x;
}


/*********************************************************************
*
*             _FS_IDE_DevIoCtl
*
  Description:
  FS driver function. Execute device command.

  Parameters:
  Unit        - Unit number.
  Cmd         - Command to be executed.
  Aux         - Parameter depending on command.
  pBuffer     - Pointer to a buffer used for the command.
 
  Return value:
  Command specific. In general a negative value means an error.
*/

static int _FS_IDE_DevIoCtl(FS_u32 Unit, FS_i32 Cmd, FS_i32 Aux, void *pBuffer) {
  FS_u32 *info;
  int x;
  char a;

  Aux = Aux;  /* Get rid of compiler warning */
  if (Unit >= FS_IDE_MAXUNIT) {
    return -1;  /* No valid unit number */
  }
  switch (Cmd) {
    case FS_CMD_INC_BUSYCNT:
      _FS_ide_busycnt[Unit]++;
      if (_FS_ide_busycnt[Unit] > 0) {
        FS_IDE_HW_X_BusyLedOn(Unit);
      }
      break;
  /*  case FS_CMD_DEC_BUSYCNT:
      _FS_ide_busycnt[Unit]--;
      if (_FS_ide_busycnt[Unit] <= 0) {
        _FS_ide_busycnt[Unit] = 0;
        FS_IDE_HW_X_BusyLedOff(Unit);
      }
      break;*/
    case FS_CMD_CHK_DSKCHANGE:
      a = FS_IDE_HW_X_DetectStatus(Unit);
      if (a) {
        _FS_ide_diskchange[Unit] = 1;
      }    
      break;
    case FS_CMD_GET_DEVINFO:
      if (!pBuffer) {
        return -1;
      }
      info = pBuffer;
      FS__IDE_Init(Unit);
      x = FS__IDE_ReadSector(Unit, 0, (unsigned char*)&_FS_ide_mbrbuffer[0]);
      if (x != 0) {
        return -1;
      }
      /* hidden */
      *info = _FS_ide_mbrbuffer[0x1c6];
      *info += (0x100UL * _FS_ide_mbrbuffer[0x1c7]);
      *info += (0x10000UL * _FS_ide_mbrbuffer[0x1c8]);
      *info += (0x1000000UL * _FS_ide_mbrbuffer[0x1c9]);
      info++;
      /* head */
      *info = _FS_ide_mbrbuffer[0x1c3]; 
      info++;
      /* sec per track */
      *info = _FS_ide_mbrbuffer[0x1c4]; 
      info++;
      /* size */
      *info = _FS_ide_mbrbuffer[0x1ca];
      *info += (0x100UL * _FS_ide_mbrbuffer[0x1cb]);
      *info += (0x10000UL * _FS_ide_mbrbuffer[0x1cc]);
      *info += (0x1000000UL * _FS_ide_mbrbuffer[0x1cd]);
      break;
    default:
      break;
  }
  return 0;
}


/*********************************************************************
*
*             Global variables
*
**********************************************************************
*/

const FS__device_type FS__idedevice_driver = {
  "IDE device",
  _FS_IDE_DevStatus,
  _FS_IDE_DevRead,
  _FS_IDE_DevWrite,
  _FS_IDE_DevIoCtl
};

#endif /* FS_USE_IDE_DRIVER */

