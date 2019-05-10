/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  smb2_drv.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/06/22 11:59:39 $
 *    $Revision: 3.4 $
 *
 *       \brief  Header file for SMB2 driver containing
 *               SMB2 specific status codes and
 *               SMB2 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2003-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SMB2_DRV_H
#define _SMB2_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif


/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** structure for simple (byte/word) data transfers */
typedef struct
{
	u_int32 flags; 		/**< function specific flags */	
	u_int16 addr;		/**< device address */
	u_int8 cmdAddr;		/**< data to be sent in command field */
	u_int8 readWrite;	/**< SMB_READ or SMB_WRITE access */
	union{ 
		u_int8 byteData;	/**< byte data to transfer */
		u_int16 wordData;	/**< word data to transfer */
		u_int16 alertCnt;	/**< number of received alerts */
	}u;
}SMB2_TRANSFER;

/** structure for WriteBlockData, ReadBlockData, BlockProcessCall */
typedef struct
{
	u_int32 flags; 			/**< function specific flags */	
	u_int16 addr;			/**< device address */
	u_int8 cmdAddr;			/**< data to be sent in command field */
	union{
		u_int8 length;		/**< length to transfer */
		u_int8 writeLen;	/**< write data length */
	}u;
	u_int8 readLen;			/**< read data length */
	u_int8 data[SMB_BLOCK_MAX_BYTES]; /**< data to transfer
								 - data to write starts here
							     - data to receive starts at offset writeDataLen
								 - writeLen + readLen is limited to 32 bytes
	                             dataP            --> +---------------+
	                                                  | data to write |
	                                                  |       .       |
	                                                  |       .       |
	                             dataP + writeLen --> +---------------+
	                                                  | data to read  |
								                      |       .       |
								                      +---------------+  */
}SMB2_TRANSFER_BLOCK;

/** structure for AlertCbInstall, AlertCbRemove */
typedef struct
{
	u_int16 addr;		/**< device address */
	u_int32 sigCode;	/**< UOS_SIG_XXX code */
}SMB2_ALERT;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name SMB2 specific Getstat/Setstat standard codes 
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define SMB2_xxx      M_DEV_OF+0x00   /**<  S: Perform Software Trigger */
/**@}*/

/** \name SMB2 specific Getstat/Setstat block codes */
/**@{*/
#define SMB2_BLK_QUICK_COMM			M_DEV_BLK_OF+0x00  /**<   S: QuickComm */
#define SMB2_BLK_WRITE_BYTE			M_DEV_BLK_OF+0x01  /**<   S: WriteByte */
#define SMB2_BLK_READ_BYTE			M_DEV_BLK_OF+0x02  /**< G  : ReadByte */
#define SMB2_BLK_WRITE_BYTE_DATA	M_DEV_BLK_OF+0x03  /**<   S: WriteByteData */
#define SMB2_BLK_READ_BYTE_DATA		M_DEV_BLK_OF+0x04  /**< G  : ReadByteData */
#define SMB2_BLK_WRITE_WORD_DATA	M_DEV_BLK_OF+0x05  /**<   S: WriteWordData */
#define SMB2_BLK_READ_WORD_DATA		M_DEV_BLK_OF+0x06  /**< G  : ReadWordData */
#define SMB2_BLK_WRITE_BLOCK_DATA	M_DEV_BLK_OF+0x07  /**<   S: WriteBlockData */
#define SMB2_BLK_READ_BLOCK_DATA	M_DEV_BLK_OF+0x08  /**< G  : ReadBlockData */
#define SMB2_BLK_PROCESS_CALL		M_DEV_BLK_OF+0x09  /**< G  : ProcessCall */
#define SMB2_BLK_BLOCK_PROCESS_CALL	M_DEV_BLK_OF+0x0a  /**< G  : BlockProcessCall */
#define SMB2_BLK_ALERT_RESPONSE		M_DEV_BLK_OF+0x0b  /**< G  : AlertResponse */
#define SMB2_BLK_ALERT_CB_INSTALL	M_DEV_BLK_OF+0x0c  /**<   S: AlertCbInstall */
#define SMB2_BLK_ALERT_CB_REMOVE	M_DEV_BLK_OF+0x0d  /**<   S: AlertCbRemove */
#define SMB2_BLK_I2C_XFER			M_DEV_BLK_OF+0x0e  /**< G  : I2cXfer */

/**@}*/

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void SMB2_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
 /* we have an MDIS4 men_types.h and mdis_api.h included */
 /* only 32bit compatibility needed!                     */
 #define INT32_OR_64  int32
 #define U_INT32_OR_64 u_int32
 typedef INT32_OR_64  MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
      }
#endif

#endif /* _SMB2_DRV_H */


