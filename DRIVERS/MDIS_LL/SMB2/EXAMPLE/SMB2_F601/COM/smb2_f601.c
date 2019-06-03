/****************************************************************************
 ************                                                    ************
 ************                   SMB2_F601                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file smb2_f601.c
 *       \author dieter.pfeuffer@men.de
 *
 *       \brief  Example program for F601 I2C Expander (Philips PCF8574)
 *
 *               - init SMB2_API library (SMB2API_Init)
 *               - control F601 I/Os
 *               - exit SMB2_API library (SMB2API_Exit)
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *     \switches (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2006-2019, MEN Mikro Elektronik GmbH
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>
#include <MEN/f601io.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void usage(void);
static void PrintIoNames( void );
static int32 PrintInStates( void *smbHdl );
static void PrintError(char *info, int32 errCode);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage: smb2_f601 [<opts>] <device> [<opts>]\n");
	printf("Function: Control F601 I/Os\n");
	printf("Options:\n");
	printf("    device       device name                          \n");
	printf("    -o=<val>	 set outputs                          \n");			
	printf("                   val = bit1:OUT2 | bit0:OUT1        \n");
	printf("                   e.g. '2' or '0x02': OUT2=1, OUT1=0 \n");
	printf("    -g           get state of inputs                  \n");
	printf("    -l           loop until keypress:                 \n");
	printf("                   toggle outputs get inputs          \n");
	printf("\n");
    printf("\nCopyright (c) 2006-2019, MEN Mikro Elektronik GmbH\n%s\n\n", IdentString)
}

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
	int32	n, err, ret=1;
	char	*device, *str, *errstr, buf[40];
	int32	get, out, loop;
	void	*smbHdl;
	u_int8	outVal;

	/*----------------------+
    |  check arguments      |
    +----------------------*/
	if( (errstr = UTL_ILLIOPT("o=gl?", buf)) ){
		printf("*** %s\n", errstr);
		return(1);
	}

	if( UTL_TSTOPT("?") ){
		usage();
		return(1);
	}

	if( argc < 3 ){
		usage();
		return(1);
	}
	
	/*----------------------+
    |  get arguments        |
    +----------------------*/
	for (device=NULL, n=1; n<argc; n++)
		if( *argv[n] != '-' ){
			device = argv[n];
			break;
		}

	if( !device) {
		usage();
		return(1);
	}

	out  = ((str = UTL_TSTOPT("o=")) ? atoi(str) : -1);
	if( out != -1 ){
		/* hex? */
		if( 0 == strncmp( str, "0x", 2 ))
			sscanf( str, "%x", &out );
	}
	
	get	 = (UTL_TSTOPT("g") ? 1 : 0);
	loop = (UTL_TSTOPT("l") ? 1 : 0);

	/*----------------------+
    |  init SMB2 library    |
    +----------------------*/
	err = SMB2API_Init( device, &smbHdl );
	if( err ){
		PrintError("SMB2API_Init", err);
		return(1);
	}

	if( out != -1 ){
		/*----------------------+
		|  set outputs          |
		+----------------------*/
		err = SMB2API_WriteByte( smbHdl, 0, F601IO_ADDR,
				(u_int8)(out | F601IO_IN_MASK) );
		if( err ){
			PrintError("SMB2API_WriteByte", err);
			goto ERR_EXIT;
		}
	}

	if( get ){
		/*----------------------+
		|  get input states     |
		+----------------------*/
		PrintIoNames();
		if( PrintInStates( smbHdl ) )
			goto ERR_EXIT;
	}

	if( loop ){
		/*------------------------------+
		|  toggle outputs / get inputs  |
		+------------------------------*/
		outVal = 0xff;

		/* repeat until keypress */
		do {
			/* toggle outputs */
			err = SMB2API_WriteByte( smbHdl, 0, F601IO_ADDR,
				(u_int8)(outVal | F601IO_IN_MASK) );
			if( err ){
				PrintError("SMB2API_WriteByte", err);
				goto ERR_EXIT;
			}

			PrintIoNames();

			printf("output state :  ");
			for( n=7; n>=0; n-- ){
				if( G_F601IO_tbl[n].dir != dirIn )
					printf("%d     ", (outVal>>n)&1 );
				else
					printf("x     ");
			}
			printf("\n");

			/* print input states */
			if( PrintInStates( smbHdl ) )
				goto ERR_EXIT;

			outVal = ~outVal;

			UOS_Delay(2000);

		} while( UOS_KeyPressed() == -1 );
	}

	ret = 0;

ERR_EXIT:
	/*----------------------+
    |  exit SMB2 library    |
    +----------------------*/
	err = SMB2API_Exit( &smbHdl );
	if( err ){
		PrintError("SMB2API_Exit", err);
		ret = 1;
	}

	return ret;
}

/***************************************************************************/
/** Print i/o names
 *
*/
static void PrintIoNames( void )
{
	int32	n;

	printf("      i/o    : ");
	for( n=7; n>=0; n-- )
		printf("%s  ", G_F601IO_tbl[n].name);
	printf("\n");
}

/***************************************************************************/
/** Print input states
 *
 *  \param smbHdl	\IN  SMB handle
 *  \return			success (0) or error (1)
*/
static int32 PrintInStates( void *smbHdl )
{
	int32	err, n;
	u_int8	inVal;

	err = SMB2API_ReadByte( smbHdl, 0, F601IO_ADDR, &inVal );
	if( err ){
		PrintError("SMB2API_ReadByte", err);
		return(1);
	}

	printf("input state  :  ");
	for( n=7; n>=0; n-- ){
		if( G_F601IO_tbl[n].dir != dirOut )
			printf("%d     ", (inVal>>n)&1 );
		else
			printf("x     ");
	}
	printf("\n\n");

	return 0;
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

