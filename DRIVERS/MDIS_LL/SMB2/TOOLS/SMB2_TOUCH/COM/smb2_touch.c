/****************************************************************************
 ************                                                    ************
 ************                   SMB2_TOUCH                       ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file smb2_touch.c
 *       \author dieter.pfeuffer@men.de
 *        $Date: 2013/09/18 09:28:15 $
 *    $Revision: 1.1 $
 *
 *       \brief  Tool that performs a single read byte access to a SMB device
 *
 *               Note: Used from Windows SmbInstaller.
 *
 *     Required: libraries: mdis_api, usr_oss, smb2_api
 *     \switches (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2013-2019, MEN Mikro Elektronik GmbH
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/usr_oss.h>
#include <MEN/smb2_api.h>

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info, int32 errCode);

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          0 for success or error code from SMB2 API
 */
int main(int argc, char *argv[])
{
	int32	err;
	int	addr;
	u_int16 smbAddr;
	char	*device;
	void	*smbHdl;
	u_int8	byte;

	if (argc < 3 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: smb2_touch <devName> <smbDevAddr>\n");
		printf("Function: Read byte from SMB device\n");
		printf("          and return SMB2-API error codes as exit code.\n");
		printf("Options:\n");
		printf("    devName       device name of SMB2 driver instance\n");
		printf("    smbDevAddr    address of SMB device (hex or dec)\n");
		printf("\n");
		return(-1);
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
		return(err);
	}

	/*--------------------+
    |  read byte          |
    +--------------------*/
	err = SMB2API_ReadByte( smbHdl, 0, smbAddr, &byte );
	if( err ){
		PrintError("SMB2API_ReadByte", err);
		goto ERR_EXIT;
	}
	printf("read byte: 0x%x\n", byte);

	err = 0;

ERR_EXIT:
	/*--------------------+
    |  exit library       |
    +--------------------*/
	SMB2API_Exit( &smbHdl );

	return(err);
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
