/****************************************************************************
 ************                                                    ************
 ************                   SMB2_SIMP                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file smb2_simp.c
 *       \author dieter.pfeuffer@men.de
 *        $Date: 2009/06/22 11:59:09 $
 *    $Revision: 1.5 $
 *
 *       \brief  Simple example program for SMB2_API usage
 *
 *               - init SMB2_API library (SMB2API_Init)
 *               - print smb2_api rev string (SMB2API_Ident)
 *               - print smb2 driver rev string (using MDIS_API)
 *               - ready byte from SMB device (smbHdl->ReadByte)
 *               - exit SMB2_API library (SMB2API_Exit)
 *
 *     Required: libraries: mdis_api, usr_oss, smb2_api
 *     \switches (none)
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: smb2_simp.c,v $
 * Revision 1.5  2009/06/22 11:59:09  dpfeuffer
 * R: Porting to MIDS5
 * M: added support for 64bit (MDIS_PATH)
 *
 * Revision 1.4  2006/08/04 16:58:35  ts
 * include <stdlib.h> to avoid implicit declaration warnings
 *
 * Revision 1.3  2006/03/06 11:35:49  cs
 * fixed: sscanf(str,"%x",valP) takes only (int*) as argument (VxW 6.2)
 *
 * Revision 1.2  2006/03/03 10:52:34  DPfeuffer
 * - error handling fixed
 * - ERROR label replaced by ERR_EXIT
 *
 * Revision 1.1  2006/02/28 15:57:27  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/smb2_api.h>

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info, int32 errCode);
static int32 PrintDrvRevString( char *device );

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	int32	err, ret=1;
	int	addr;
	u_int16 smbAddr;
	char	*device;
	void	*smbHdl;
	u_int8	byte;

	if (argc < 3 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: smb2_simp <devName> <smbDevAddr>\n");
		printf("Function: print revision strings, read byte from SMB device\n");
		printf("Options:\n");
		printf("    devName       device name of SMB2 driver instance\n");
		printf("    smbDevAddr    address of SMB device\n");
		printf("\n");
		return(1);
	}

	device = argv[1];

	/* hex address? */
	if( 0 == strncmp( argv[2], "0x", 2 ))
		sscanf( argv[2], "%x", &addr );
	/* dec address */
	else
		addr = (u_int16)atoi(argv[2]);

	smbAddr=(u_int16)addr;

	/*--------------------+
    |  init library       |
    +--------------------*/
	err = SMB2API_Init( device, &smbHdl );
	if( err ){
		PrintError("SMB2API_Init", err);
		return(1);
	}

	/*----------------------------------------------+
    |  SMB2API_Xxx functions can be called directly |
	|  ( print smb2_api rev string )                |
    +----------------------------------------------*/
	printf("API rev string\n");
	printf("==============\n%s\n\n", SMB2API_Ident() );

	/*----------------------------------------------+
    |  MDIS_API calls can be made                   |
	|  ( print smb2 driver rev string )             |
    +----------------------------------------------*/
	if( PrintDrvRevString( device ) )
		goto ERR_EXIT;

	printf("Accessing %s: smbAddr 0x%x\n", device, smbAddr);
	/*----------------------------------------------+
    |  SMB2API_Xxx functions can also be called via |
	|  function pointers (like smb2 kernel library) |
	+----------------------------------------------*/
	err = ((SMB_HANDLE*)smbHdl)->ReadByte( smbHdl, 0x00, smbAddr, &byte );
	if( err ){
		PrintError("smbHdl->ReadByte", err);
		goto ERR_EXIT;
	}
	printf("read byte: 0x%x\n", byte);

	ret = 0;

ERR_EXIT:
	/*--------------------+
    |  exit library       |
    +--------------------*/
	err = SMB2API_Exit( &smbHdl );
	if( err ){
		PrintError("SMB2API_Exit", err);
		ret = 1;
	}

	return ret;
}

/***************************************************************************/
/** Print SMB2API/MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info, int32 errCode)
{
    static char errMsg[512];

	if( !errCode )
		errCode = UOS_ErrnoGet();

	printf("*** can't %s: %s\n", info, SMB2API_Errstring( errCode, errMsg ));
}

/***************************************************************************/
/** Print driver revision strings
 *
 *  \param path       \IN  path to device
 *  \return	          success (0) or error (1)
*/
static int32 PrintDrvRevString( char *device )
{
	MDIS_PATH	path;
	int32		size, ret=1;
	char		*buf=NULL;
	M_SG_BLOCK	blk;

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("M_open", 0);
		return(1);
	}

	/*----------------------+
    |  get revision string  |
    +----------------------*/
	/* get required size */
	if( M_getstat( path, M_MK_REV_SIZE, &size ) < 0){
		PrintError("M_getstat(M_MK_REV_SIZE)", 0);
		goto ERR_EXIT;
	}

	/* alloc buffer */
	if( (buf = (char*)malloc(size)) == NULL)
		goto ERR_EXIT;

	blk.size = size;
	blk.data = (void*)buf;

	/* get revision string */
	if( M_getstat( path, M_MK_BLK_REV_ID, (int32*)&blk) < 0){
		PrintError("M_getstat(M_MK_BLK_REV_ID)", 0);
		goto ERR_EXIT;
	}

	/* print string */
	printf("Driver rev string\n");
	printf("=================\n%s\n",buf);

	ret = 0;

	/*--------------------+
    |  close  path        |
    +--------------------*/
ERR_EXIT:
	if( M_close(path) < 0 ){
		PrintError("M_close", 0);
		ret = 1;
	}

	if( buf )
		free(buf);

	return ret;
}

