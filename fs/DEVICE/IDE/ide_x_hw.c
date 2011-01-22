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
 */ 
 /********************************************************************* 
 * 
 *             #include Section 
 * 
 ********************************************************************** 
 */ 
  
 #include "fs_port.h" 
 #include "fs_conf.h" 
  
 #if FS_USE_IDE_DRIVER 
  
 #include "ide_x_hw.h" 
 #include "includes.h" 
 #include "std.h" 
 #include "io.h" 
 /*  
   The following header file is part of the IAR compiler for M16C/80.  
   If you use a different compiler, you may have to include a different  
   file or you may even have to replace the SFR access within this  
   file. 
 */ 
  
  
  
 /********************************************************************* 
 * 
 *             #define Macros 
 * 
 ********************************************************************** 
 */ 
  
 /* 
    To meet timing specification, you may have to add delays here 
    when porting to a different CPU. For MC80 at 16MHz, there is no 
    delay required. 
 */ 
  
 #define FS__IDE_DELAY_WRITE    
 #define FS__IDE_DELAY_READ 
 #define FS__IDE_DELAY_RESET 
  
 #define IDX_HD_PORT_DATA        (0) 
 #define IDX_HD_PORT_ERROR       (1) 
 #define IDX_HD_PORT_SECT_COUNT  (2) 
 #define IDX_HD_PORT_SECT_NUM    (3) 
 #define IDX_HD_PORT_CYL_LOW     (4) 
 #define IDX_HD_PORT_CYL_HIGH    (5) 
 #define IDX_HD_PORT_DRV_HEAD    (6) 
 #define IDX_HD_PORT_STATUS      (7) 
 #define IDX_HD_PORT_COMMAND     (7) 
  
 #define HD_STATUS_BSY                   (0x80) 
 #define HD_STATUS_DRDY                  (0x40) 
 #define HD_STATUS_DF                    (0x20) 
 #define HD_STATUS_DRQ                   (0x08) 
 #define HD_STATUS_ERR                   (0x01) 
  
 #define HD_READ             (0x20) 
 #define HD_WRITE            (0x30) 
  
 static u32 ide_ports[][8] = { 
     {0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7}, 
 }; 
  
 #define IDX_NS                  (0) 
 #define IDX_NH                  (1) 
 #define IDX_NC                  (2) 
  
 /* 
 ata0-master: type=disk, path="disk", mode=flat, cylinders=900, heads=15, spt=17 
 */ 
 static u32 hd_params[] = { 
       17, 
       15, 
       900 
 }; 
  
 #define LBA_TO_C(lba)   (((lba) / hd_params[IDX_NS]) / hd_params[IDX_NH]) 
 #define LBA_TO_H(lba)   (((lba) / hd_params[IDX_NS]) % hd_params[IDX_NH]) 
 #define LBA_TO_S(lba)   (((lba) % hd_params[IDX_NS]) + 1) 
  
 /********************************************************************* 
 * 
 *             Global functions section 
 * 
 ********************************************************************** 
   */ 
  
 static u32 ll_hd_rw(FS_u32 Unit, FS_u8 cmd, FS_u8 *buffer,  
           FS_u32 sectors, FS_u32 cyl, FS_u32 head, FS_u32 sect) 
 { 
     u8 status; 
     u32 size; 
     u16 *p = (u16*)buffer; 
  
     while(inb(ide_ports[Unit][IDX_HD_PORT_STATUS]) & HD_STATUS_BSY); 
      
     outb(sectors, ide_ports[Unit][IDX_HD_PORT_SECT_COUNT]); 
     outb(sect, ide_ports[Unit][IDX_HD_PORT_SECT_NUM]); 
     outb((u8)cyl, ide_ports[Unit][IDX_HD_PORT_CYL_LOW]); 
     outb((u8)(cyl >> 8), ide_ports[Unit][IDX_HD_PORT_CYL_HIGH]); 
     outb(0xa0 | head, ide_ports[Unit][IDX_HD_PORT_DRV_HEAD]); 
     outb(cmd, ide_ports[Unit][IDX_HD_PORT_COMMAND]); 
      
     while(!(inb(ide_ports[Unit][IDX_HD_PORT_STATUS]) & HD_STATUS_DRDY)); 
  
     while(sectors--) 
     { 
         size = 512 / 2; 
         if(cmd == HD_READ) 
         { 
                 while(size--) 
                         *p++ = inw(ide_ports[Unit][IDX_HD_PORT_DATA]); 
         } 
         else if(cmd == HD_WRITE) 
         { 
                 while(size--) 
                 outw(*p++, ide_ports[Unit][IDX_HD_PORT_DATA]); 
         } 
         while(!((inb(ide_ports[Unit][IDX_HD_PORT_STATUS])) & HD_STATUS_DRDY)); 
     } 
     return 0; 
 } 
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_BusyLedOn 
 * 
   Description: 
   FS driver hardware layer function. Turn on busy led. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   None. 
 */ 
  
 void  FS_IDE_HW_X_BusyLedOn(FS_u32 Unit) { 
          ; 
 } 
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_BusyLedOff 
 * 
   Description: 
   FS driver hardware layer function. Turn off busy led. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_BusyLedOff(FS_u32 Unit) {  
  ;       
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_HWReset 
 * 
   Description: 
   FS driver hardware layer function. This function is called, when  
   the driver detects a new media is present. For ATA HD drives, there  
   is no action required and this function can be empty. 
   When using a CF card, please be aware, that the card needs to be 
   power cycled while ~OE is grounded. If the card is inserted, VCC &  
   GND will provide the card before ~OE is connected and the card will  
   be in PC Card ATA mode. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_HWReset(FS_u32 Unit) { 
   ; 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetAltStatus 
 * 
   Description: 
   FS driver hardware layer function. Read the ALTERNATE STATUS register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the ALTERNATE STATUS register. 
 */ 
  
 unsigned char FS_IDE_HW_X_GetAltStatus(FS_u32 Unit) { 
   return 0; 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetCylHigh 
 * 
   Description: 
   FS driver hardware layer function. Read the CYLINDER HIGH register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the CYLINDER HIGH register. 
 */ 
  
 unsigned char FS_IDE_HW_X_GetCylHigh(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_CYL_HIGH]);  
 } 
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetCylLow 
 * 
   Description: 
   FS driver hardware layer function. Read the CYLINDER LOW register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the CYLINDER LOW register. 
 */ 
  
 unsigned char FS_IDE_HW_X_GetCylLow(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_CYL_LOW]);  
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetData 
 * 
   Description: 
   FS driver hardware layer function. Read the RD DATA register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the RD DATA register. 
 */ 
  
    FS_u16 FS_IDE_HW_X_GetData(FS_u32 Unit) { 
     return inw(ide_ports[Unit][IDX_HD_PORT_DATA]);  
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetDevice 
 * 
   Description: 
   FS driver hardware layer function. Read the DEVICE/HEAD register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the DEVICE/HEAD register. 
 */ 
  
    unsigned char FS_IDE_HW_X_GetDevice(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_DRV_HEAD]);  
 } 
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetError 
 * 
   Description: 
   FS driver hardware layer function. Read the ERROR register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the ERROR register. 
 */ 
  
    unsigned char FS_IDE_HW_X_GetError(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_ERROR]);  
 } 
  
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetSectorCount 
 * 
   Description: 
   FS driver hardware layer function. Read the SECTOR COUNT register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the SECTOR COUNT register. 
 */ 
  
   unsigned char FS_IDE_HW_X_GetSectorCount(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_SECT_COUNT]);  
 } 
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetSectorNo 
 * 
   Description: 
   FS driver hardware layer function. Read the SECTOR NUMBER register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the SECTOR NUMBER register. 
 */ 
  
   unsigned char FS_IDE_HW_X_GetSectorNo(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_SECT_NUM]);  
   return 0; 
 } 
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_GetStatus 
 * 
   Description: 
   FS driver hardware layer function. Read the STATUS register. 
  
   Parameters: 
   Unit        - Unit number. 
   
   Return value: 
   Value of the STATUS register. 
 */ 
  
    unsigned char FS_IDE_HW_X_GetStatus(FS_u32 Unit) { 
     return inb(ide_ports[Unit][IDX_HD_PORT_STATUS]);  
 } 
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetCommand 
 * 
   Description: 
   FS driver hardware layer function. Set the COMMAND register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the COMMAND register. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_SetCommand(FS_u32 Unit, unsigned char Data) { 
     outb(Data, ide_ports[Unit][IDX_HD_PORT_COMMAND]); 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetCylLow 
 * 
   Description: 
   FS driver hardware layer function. Set the CYLINDER LOW register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the CYLINDER LOW register. 
   
   Return value: 
   None. 
 */ 
  
    void FS_IDE_HW_X_SetCylLow(FS_u32 Unit, unsigned char Data) { 
     outb(Data, ide_ports[Unit][IDX_HD_PORT_CYL_LOW]); 
 } 
  
  
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetCylHigh 
 * 
   Description: 
   FS driver hardware layer function. Set the CYLINDER HIGH register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the CYLINDER HIGH register. 
   
   Return value: 
   None. 
 */ 
  
    void FS_IDE_HW_X_SetCylHigh(FS_u32 Unit, unsigned char Data) {  
     outb(Data, ide_ports[Unit][IDX_HD_PORT_CYL_HIGH]); 
    ; 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetData 
 * 
   Description: 
   FS driver hardware layer function. Set the WR DATA register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Data to be set. 
   
   Return value: 
   None. 
 */ 
  
    void FS_IDE_HW_X_SetData(FS_u32 Unit, FS_u16 Data) { 
     outw(Data, ide_ports[Unit][IDX_HD_PORT_DATA]); 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetDevice 
 * 
   Description: 
   FS driver hardware layer function. Set the DEVICE/HEAD register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the DEVICE/HEAD register. 
   
   Return value: 
   None. 
 */ 
  
    void FS_IDE_HW_X_SetDevice(FS_u32 Unit, unsigned char Data) { 
        outb(Data, ide_ports[Unit][IDX_HD_PORT_DRV_HEAD]); 
 }  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetFeatures 
 * 
   Description: 
   FS driver hardware layer function. Set the FEATURES register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the FEATURES register. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_SetFeatures(FS_u32 Unit, unsigned char Data) {  
    ; 
 } 
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetSectorCount 
 * 
   Description: 
   FS driver hardware layer function. Set the SECTOR COUNT register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the SECTOR COUNT register. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_SetSectorCount(FS_u32 Unit, unsigned char Data) { 
     outb(Data, ide_ports[Unit][IDX_HD_PORT_SECT_COUNT]); 
 } 
  
  
  
  
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetSectorNo 
 * 
   Description: 
   FS driver hardware layer function. Set the SECTOR NUMBER register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the SECTOR NUMBER register. 
   
   Return value: 
   None. 
 */ 
  
  
 void FS_IDE_HW_X_SetSectorNo(FS_u32 Unit, unsigned char Data) {  
     outb(Data, ide_ports[Unit][IDX_HD_PORT_SECT_NUM]); 
 } 
 /********************************************************************* 
 * 
 *             FS_IDE_HW_X_SetDevControl 
 * 
   Description: 
   FS driver hardware layer function. Set the DEVICE CONTROL register. 
  
   Parameters: 
   Unit        - Unit number. 
   Data        - Value to write to the DEVICE CONTROL register. 
   
   Return value: 
   None. 
 */ 
  
 void FS_IDE_HW_X_SetDevControl(FS_u32 Unit, unsigned char Data) { 
   ; 
 } 
  
  
 char FS_IDE_HW_X_DetectStatus(FS_u32 Unit) { 
         return 0; 
 } 
  
  
  
 int FS__IDE_Init(FS_u32 Unit){ 
     return 0; 
 } 
  
  
 int FS__IDE_ReadSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer){ 
     ll_hd_rw(Unit, HD_READ, pBuffer, 1, LBA_TO_C(Sector), LBA_TO_H(Sector), LBA_TO_S(Sector)); 
     return 0; 
 } 
  
  
  
 int FS__IDE_WriteSector(FS_u32 Unit,unsigned long Sector,unsigned char *pBuffer){           
     ll_hd_rw(Unit, HD_WRITE, pBuffer, 1, LBA_TO_C(Sector), LBA_TO_H(Sector), LBA_TO_S(Sector)); 
         return 0; 
 } 
 #endif /* FS_USE_IDE_DRIVER */  