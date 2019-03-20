/***********************  I n c l u d e  -  F i l e  ************************/
/*!  
 *        \file  smb2_ctrl.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/06/22 11:59:22 $
 *    $Revision: 1.5 $
 * 
 *  	 \brief SMB2_CTRL master include file
 *                      
 *     Switches: -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright (c) 2019, MEN Mikro Elektronik GmbH
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

#ifndef _SMB2_CTRL_H
#define _SMB2_CTRL_H

#ifdef __cplusplus
      extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <MEN/men_typs.h>
#include <MEN/usr_utl.h>
#include <MEN/usr_oss.h>
#include <MEN/mdis_err.h>

#include <MEN/smb2_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define DATABUF_SIZE		0x10000		/* 64kB buffer */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
extern u_int32	SMB2CTRL_alertCallCount;	/* alert counter */
extern void		*SMB2CTRL_smbHdl;			/* SMB2 handle */
extern u_int8	SMB2CTRL_flag;
extern u_int32	SMB2CTRL_errCount;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
extern int32 SMB2CTRL_CmdHelp( void );
extern void SMB2CTRL_PrintStatus( int32 errCode );
extern char* SMB2CTRL_StrCatSep(char *dest, const char *src, const char *sep);

/* SMB2_API functionality */
extern int32 SMB2CTRL_QuickComm(void);
extern int32 SMB2CTRL_WriteByte(void);
extern int32 SMB2CTRL_ReadByte(void);
extern int32 SMB2CTRL_WriteByteData(void);
extern int32 SMB2CTRL_ReadByteData(void);
extern int32 SMB2CTRL_WriteWordData(void);
extern int32 SMB2CTRL_ReadWordData(void);
extern int32 SMB2CTRL_WriteBlockData(void);
extern int32 SMB2CTRL_ReadBlockData(void);
extern int32 SMB2CTRL_ProcessCall(void);
extern int32 SMB2CTRL_BlockProcessCall(void);
extern int32 SMB2CTRL_AlertResponse(void);
extern int32 SMB2CTRL_AlertCbInstall(void);
extern int32 SMB2CTRL_AlertCbInstallSig(void);
extern int32 SMB2CTRL_AlertCbRemove(void);
extern int32 SMB2CTRL_I2CXfer(void);
extern int32 SMB2CTRL_Errstring(void);
extern int32 SMB2CTRL_Ident(void);

/* Tools */
extern int32 SMB2CTRL_List(void);
extern int32 SMB2CTRL_Mtest(void);

#ifdef __cplusplus
   }
#endif

#endif /* _SMB2_CTRL_H */ 







