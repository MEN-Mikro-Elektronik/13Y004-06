/****************************************************************************
 *************                                                    ***********
 *************                   SMB2_TEST                     ************
 *************                                                    ***********
 ****************************************************************************/
/*!
 *         \file smb2_test.c
 *       \author dieter.pfeuffer@men.de
 *
 *        \brief Test tool for SMB2 functionality.
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2016-2019, MEN Mikro Elektronik GmbH
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

/*-------------------------------------+
|    INCLUDES                          |
+-------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
# pragma warning(disable:4996)
#endif

/*-------------------------------------+
|    DEFINES                           |
+-------------------------------------*/
#define SMB_FLAGS                0x0

/*-------------------------------------+
|    PROTOTYPES                        |
+-------------------------------------*/
static void PrintError(char*, int32);
static void PrintTime( void );

/********************************** usage **********************************/
/** Prints the program usage
 */
static void usage(void)
{
	printf("\n"
		"Usage:     smb2_test  devName  [<opts>]                             \n"
		"Function:  Test with SMB2API_ReadByte.                              \n"
		"Options:                                                            \n"
		"  devName         device name e.g. smb2_1                           \n"
		"  -a=hex          address of smb dev                                \n"
		"  [-l <opts>]     read in a loop                                    \n"
		"    [-d=<time>]    delay between read (in ms)..................[500]\n"
		"    [-s]           stop after error                                 \n"
		"\n"
		"(c)Copyright 2016 by MEN Mikro Elektronik GmbH\n"
	);
}

/***************************************************************************/
/** Program main function
 *
 *  \param argc    \IN argument counter
 *  \param argv    \IN argument vector
 *
 *  \return        success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	int     err, ret=0, i=0;
	char    *errstr=NULL, ebuf[100];
	char    *optp=NULL;
	u_int32 delay=0, smbAddr=0x0, loop, stop;
	char    *deviceP=NULL;
	void    *smbHdl=NULL;
	u_int8	byteData;
	u_int32	callCount=0;

	/*------------------+
	|  Check arguments  |
	+------------------*/
	errstr = UTL_ILLIOPT("?a=lsd=", ebuf);
	if (errstr) {
		printf("*** %s\n", errstr);
		usage();
		ret=1;
		goto EXIT;
	}
	if (UTL_TSTOPT("?")) {
		usage();
		goto EXIT;
	}

	/*----------------+
	|  Get arguments  |
	+----------------*/
	for (i = 1; i < argc; i++) {
		if (*argv[i] != '-') {
			if (deviceP == NULL) {
				deviceP = argv[i];
				break;
			}
		}
	}
	if (!deviceP) {
		printf("\n***ERROR: missing SMB device name!\n");
		usage();
		ret=1;
		goto EXIT;
	}

	/* SMB device address */
	optp = UTL_TSTOPT("a=");
	if (optp)
		sscanf(optp, "%x", &smbAddr);
	else{
		printf("\n***ERROR: missing SMB device address!\n");
		usage();
		ret=1;
		goto EXIT;
	}

	loop = (UTL_TSTOPT("l") ? 1 : 0);
	stop = (UTL_TSTOPT("s") ? 1 : 0);

	/* delay */
	optp = UTL_TSTOPT("d=");
	if (optp)
		sscanf(optp, "%d", &delay);
	else
		delay = 500;

	/*--------------------+
	|  Init SMB2 library  |
	+--------------------*/
	err = SMB2API_Init(deviceP, &smbHdl);
	if (err) {
		PrintError("SMB2API_Init", err);
		ret=1;
		goto EXIT;
	}

	printf("callCount: %d, start time: ", callCount);
	PrintTime();

	/*-----------------+
	|  Read in a loop  |
	+-----------------*/
	do {
		callCount++;
		
		/* Read Byte */
		err = SMB2API_ReadByte( smbHdl, SMB_FLAGS, (u_int8)smbAddr, (u_int8*)&byteData );
		if( err ){
			PrintError( "SMB2API_ReadByte", err );
			printf("callCount: %d, error time: ", callCount);
			PrintTime();
			if (stop)
				goto ERR_EXIT;
		}

		if (!loop)
			break;

		if (delay)
			UOS_Delay(delay);

	} while (UOS_KeyPressed() == -1);

	printf("callCount: %d, stop time: ", callCount);
	PrintTime();

	goto ERR_EXIT;

ERR_EXIT:
	
	if( smbHdl != NULL ){ 
		/*--------------------+
		|  Exit SMB2 library  |
		+--------------------*/
		err = SMB2API_Exit(&smbHdl);
		if (err)
			PrintError("SMB2API_Exit", err);
	}

	ret = 1;

EXIT:
	return ret;
}

/******************************* PrintError *********************************/
/** Routine to print SMB2API/MDIS error message
 *
 *  \param info       \IN info string
 *  \param errCode    \IN error code number
 */
static void PrintError(char *info, int32 errCode)
{
	static char errMsg[512];

	if (!errCode)
		errCode = UOS_ErrnoGet();

	printf("*** can't %s: %s\n", info, SMB2API_Errstring( errCode, errMsg ));

}

/******************************* GetSystemDate ********************************/
/** Routine to get the systemdate
 *
 */
static void PrintTime( void )
{
	char dt[20];
	struct tm *timeP;
	time_t systime;

	/* get current time */
	time( &systime );
	timeP = localtime( &systime );
	
	
	
	strftime( dt, sizeof(dt), "%c", timeP );
	printf("%s\n", dt);
}
