/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  smb2_api.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2007/07/24 13:32:23 $
 *    $Revision: 3.3 $
 *
 *     \project  SMB2 API
 *       \brief  system managment bus routines interface
 *    \switches  SMB_COMPILE - for module compilation
 *				 MAC_IO_MAPPED - only valid for SMB_PORT_xxx
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: smb2_api.h,v $
 * Revision 3.3  2007/07/24 13:32:23  DPfeuffer
 * - undo: SMB2API_Exit Prototype to match changed SMB2 lib API
 *
 * Revision 3.2  2006/10/05 17:44:12  cs
 * changed:
 *   - SMB2API_Exit Prototype to match changed SMB2 lib API
 *
 * Revision 3.1  2006/02/28 15:57:49  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2006 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/
#ifndef _SMB2_API_H
#  define _SMB2_API_H

#  ifdef __cplusplus
      extern "C" {
#  endif

#define SMB2_API
#include <MEN/smb2.h>


/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
int32 __MAPILIB SMB2API_Init( char *device, void **smbHdlP );

char* __MAPILIB SMB2API_Ident( void );

int32 __MAPILIB SMB2API_Exit( void **smbHdlP );

int32 __MAPILIB SMB2API_QuickComm(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 readWrite );

int32 __MAPILIB SMB2API_WriteByte(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 data );
int32 __MAPILIB SMB2API_ReadByte(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 *dataP );

int32 __MAPILIB SMB2API_WriteByteData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr, u_int8 data );
int32 __MAPILIB SMB2API_ReadByteData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr, u_int8 *dataP );

int32 __MAPILIB SMB2API_WriteWordData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr, u_int16 data );
int32 __MAPILIB SMB2API_ReadWordData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr, u_int16 *dataP );

int32 __MAPILIB SMB2API_WriteBlockData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr,
	u_int8 length, u_int8 *dataP);
int32 __MAPILIB SMB2API_ReadBlockData(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr,
	u_int8 *lengthP, u_int8 *dataP);

int32 __MAPILIB SMB2API_ProcessCall(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr, u_int16 *dataP );

int32 __MAPILIB SMB2API_BlockProcessCall(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 cmdAddr,
	u_int8 writeLen, u_int8 *writeDataP, u_int8 *readLenP, u_int8 *readDataP );

int32 __MAPILIB SMB2API_AlertResponse(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int16 *alertCntP);

int32 __MAPILIB SMB2API_AlertCbInstall(
	void *smbHdl, u_int16 addr, void (*cbFuncP)( void *cbArg ), void *cbArgP );

int32 __MAPILIB SMB2API_AlertCbInstallSig(
	void *smbHdl, u_int16 addr, void (*cbFuncP)( void *cbArg ), void *cbArgP, u_int32 sigCode );

int32 __MAPILIB SMB2API_AlertCbRemove(
	void *smbHdl, u_int16 addr, void **cbArgP );

/* SMB2API_ReservedFctP1..P4 */

int32 __MAPILIB SMB2API_SmbXfer(
	void *smbHdl, u_int32 flags, u_int16 addr, u_int8 readWrite,
	u_int8 cmdAddr, u_int8 size, u_int8 *dataP );

int32 __MAPILIB SMB2API_I2CXfer(
	void *smbHdl, SMB_I2CMESSAGE msg[], u_int32 num );

char* __MAPILIB SMB2API_Errstring(
	int32 errCode, char	*strBuf );


#  ifdef __cplusplus
      }
#  endif

#endif/*_SMB2_API_H*/




