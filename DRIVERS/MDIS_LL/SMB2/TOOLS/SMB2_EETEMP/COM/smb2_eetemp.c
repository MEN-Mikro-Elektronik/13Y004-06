/****************************************************************************
 *************                                                    ***********
 *************                   SMB2_EETEMP                     ************
 *************                                                    ***********
 ****************************************************************************/
/*!
 *         \file smb2_eetemp.c
 *       \author michael.roth@men.de
 *
 *        \brief Tool to read the temperature from EEPROMS via the SMB2_API
 *
 *                - init SMB2_API library (SMB2API_Init)
 *                - read data from SMB device (SMB2API_ReadWordData)
 *                - exit SMB2_API library (SMB2API_Exit)
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 2011-2019, MEN Mikro Elektronik GmbH
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
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
# pragma warning(disable:4996)
#endif

/*-------------------------------------+
|    MAKROS                            |
+-------------------------------------*/
#ifdef _BIG_ENDIAN_
# define SWAPWORD(w) (w)
# define SWAPLONG(l) (l)
#elif defined(_LITTLE_ENDIAN_)
# define SWAPWORD(w) ( (((w)&0xff)<<8) + (((w)&0xff00)>>8) )

#else
# error "Must define _BIG_ENDIAN_ or _LITTLE_ENDIAN_"
#endif

#if defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
# error "Don't define _BIG/_LITTLE_ENDIAN_ together"
#endif

/*-------------------------------------+
|    DEFINES                           |
+-------------------------------------*/
#define DEFAULT_TEMP_SENSE_ADDR  0x3E
#define SMB_FLAGS                0x0
#define TEMP_OFFS                0x5 

/*-------------------------------------+
|    PROTOTYPES                        |
+-------------------------------------*/
static void CalcTemp(int32 temp, double *eetemp);
static void PrintError(char*, int32);

/********************************* header **********************************/
/** Prints the headline
 */
static void header(void)
{
	printf(
		"\n========================="
		"\n===    SMB2_EETEMP    ==="
		"\n========================="
		"\n\n"
	);
}

/********************************** usage **********************************/
/** Prints the program usage
 */
static void usage(void)
{
	printf(
		"\nUsage:     smb2_eetemp  devName  [<opts>]"
		"\nFunction:  read board temperatures from EEPROMs of "
		"\nOptions:\n"
		"    devName         device name e.g. smb2_1                           \n"
		"    [-a=hex]        address of temp. sensor (EEPROM)...........[0x3E] \n"
		"    [-t=deg]        max. temperature (in %cC) - 0 means not used...[0]\n"
		"    [-l <opts>]     read temperature in a loop                        \n"
		"       [-d=<time>]    delay between loop (in ms)................[500] \n"
		"\nCalling examples:                          \n"
		"\n - show current board temperature:         \n"
		"     smb2_eetemp smb2_1                      \n"
		"\n - set max. temperature for tests and show temperature in a loop:   \n"
		"     smb2_eetemp smb2_1 -t=80 -l             \n", 248
	);
    printf("\nCopyright (c) 2011-2019, MEN Mikro Elektronik GmbH\n%s\n\n", IdentString);
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
	u_int32 delay=0, maxtemp=0, smbAddr=0x0, loop;
	int32   temp=0;
	double  eetemp;
	char    *deviceP=NULL;
	void    *smbHdl=NULL;

	/*------------------+
	|  Check arguments  |
	+------------------*/
	errstr = UTL_ILLIOPT("?a=ld=t=", ebuf);
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

	/* SMB device(EEPROM) address */
	optp = UTL_TSTOPT("a=");
	if (optp)
		sscanf(optp, "%x", &smbAddr);
	else
		smbAddr = DEFAULT_TEMP_SENSE_ADDR; /* 0x3E */

	/* max. temperature */
	optp = UTL_TSTOPT("t=");
	if (optp)
		sscanf( optp, "%d", &maxtemp );
	else
		maxtemp = 0;

	loop = (UTL_TSTOPT("l") ? 1 : 0);

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

	/* some output at the beginning */
	header();
	printf("Accessing %s: smbAddr 0x%02x; max.temp.: %d %cC\n", deviceP, smbAddr, maxtemp, 248);

	/*-----------------------------+
	|  Read temperature in a loop  |
	+-----------------------------*/
	do {
		/* read current temperature of EEPROM */
		err = SMB2API_ReadWordData( smbHdl, SMB_FLAGS, (u_int8)smbAddr, TEMP_OFFS, (u_int16*)&temp );
		if (err) {
			PrintError( "SMB2API_ReadWordData", err );
			goto ERR_EXIT;
		}

		CalcTemp(temp, &eetemp);

		if (maxtemp && (eetemp > (double)maxtemp)) {
			printf( "\n *** WARNING: Current board temperature(%.2lf %cC)"
					"\n              is higher than %d %cC!\n", eetemp, 248, maxtemp, 248 );
		}
		else {
			printf("\nCurrent Board temperature: %.2lf %cC\n", eetemp, 248);
		}

		if (!loop)
			break;

		UOS_Delay(delay);

	} while (UOS_KeyPressed() == -1);


ERR_EXIT:
	/*--------------------+
	|  Exit SMB2 library  |
	+--------------------*/
	err = SMB2API_Exit(&smbHdl);
	if (err)
		PrintError("SMB2API_Exit", err);

	ret = 1;

EXIT:
	return ret;
}

/********************************* CalcTemp *********************************/
/** Routine to calculate the actual EEPROM temperature 
 *
 *  \param temp       \IN  raw temperature value
 *  \param eetemp     \OUT formatted temperature sensor value
 */
static void CalcTemp(int32 temp, double *eetemp)
{
	/*------------------------+
	|  Calculate temperature  |
	+------------------------*/
	/* Temperature register format is 2nd complement, 
	 * with the LSB equal to 0.0625°C e.g. 0x1E74 <=> -24,75°
	 * -------------------------------------------
	 * Reserved | SignMSB                LSB | 0 0
	 *          |  ^   Temperature Range  ^  |
	 * 15 14 13 |  12 11 10 9 8 7 6 5 4 3 2  | 1 0
	 * -------------------------------------------
	 */
	if (SWAPWORD(temp) & 0x1000)
		*eetemp = (~SWAPWORD(temp) & 0xFFC) * (-0.0625);

	else
		*eetemp = (SWAPWORD(temp) & 0xFFC) * 0.0625;
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

