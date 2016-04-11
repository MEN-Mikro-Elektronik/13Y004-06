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
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: smb2_ctrl.h,v $
 * Revision 1.5  2009/06/22 11:59:22  dpfeuffer
 * R: MDVE warnings
 * M: added prefix SMB2CTRL to external globals and functions
 *
 * Revision 1.4  2007/02/20 15:51:01  DPfeuffer
 * - SMB2 error codes no longer mapped into device specific error code range
 *   (now done by smb2 lib)
 *
 * Revision 1.3  2006/05/31 08:22:19  DPfeuffer
 * - added commands -id, -list
 * - SMB2 error codes fixed
 *
 * Revision 1.2  2006/03/07 14:31:56  DPfeuffer
 * usr_oss.h added (required for VxWorks main() renaming)
 *
 * Revision 1.1  2006/02/28 15:57:37  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright by MEN Mikro Elektronik GmbH, Nuremberg, Germany 
 ****************************************************************************/

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







 
