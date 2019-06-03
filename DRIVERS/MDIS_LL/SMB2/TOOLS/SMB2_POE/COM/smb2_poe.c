/****************************************************************************
 ************                                                    ************
 ************                    SMB2_POE                        ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  smb2_poe.c
 *      \author  michael.roth@men.de
 *
 *       \brief  Tool to set/get ports power states via the SMB2_API
 *               Only for the 14G301-02 PIC firmware
 *                - init SMB2_API library (SMB2API_Init)
 *                - read data from SMB device (SMB2API_ReadByteData)
 *                - write data from SMB device (SMB2API_WriteByteData)
 *                - exit SMB2_API library (SMB2API_Exit)
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *    \switches  (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2014-2019, MEN Mikro Elektronik GmbH
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

/*--------------------------------------+
|  INCLUDES                             |
+--------------------------------------*/
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

/*--------------------------------------+
|  DEFINES                              |
+--------------------------------------*/
#define DEFAULT_PIC_ADDR         0xC0
#define GET_FW_REV               0x80
#define GET_PWR_STAT             0x81
#define SET_PWR_STAT             0x01

#define SMB_FLAGS                0x00

/*-------------------------------------+
|    PROTOTYPES                        |
+-------------------------------------*/
static void PrintError(char*, int32);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf(
		"\nUsage:     smb2_poe  <device> [<opts>] <opts>"
		"\nFunction:  set/get power states of the POE controller ports\n"
		"        -- only applicable for the 14G301-02 PIC firmware! --"
		"\nOptions:\n"
		"    device      device name (e.g. smb2_1)         \n"
		"    [-a=hex]    address of PIC..............[0xC0]\n"
		"    [-p=<port>] PoE port # (0..3)............[all]\n"
		"     -s         set PoE port power state          \n"
		"     -c         clear PoE port power state        \n"
		"     -g         get PoE port power state          \n"
		"     -r         get firmware revision             \n"
		"\nCalling examples:\n"
		"\n - show power states of all PoE ports:\n"
		"     smb2_poe smb2_1 -g\n"
		"\n - enable PoE (set power state) for all ports:\n"
		"     smb2_poe smb2_1 -s\n"
		"\n - disable PoE (clear power state) for port #2:\n"
		"     smb2_poe smb2_1 -p=2 -c\n"
		"\n - show firmware revision:\n"
		"     smb2_poe smb2_1 -r\n"
		"\n"
	);
	printf("Copyright (c) 2014-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
}

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return           success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	int   err, ret=0, i=0;
	char  *errstr=NULL, ebuf[100];
	char  *optp=NULL, *deviceP=NULL;
	char  *portStr;
	int32 port, set, clr;
	int32 portBit=0, smbAddr, rev, get, state;
	void  *smbHdl=NULL;

	/*----------------------+
	|  check arguments      |
	+----------------------*/
	if ((errstr = UTL_ILLIOPT("?a=p=scgr", ebuf))) {
		printf("*** %s\n", errstr);
		return (1);
	}
	if (UTL_TSTOPT("?")) {
		usage();
		return (1);
	}
	if (argc < 3) {
		usage();
		return (1);
	}

	/*----------------------+
	|  get arguments        |
	+----------------------*/
	for (deviceP = NULL, i = 1; i < argc; i++) {
		if (*argv[i] != '-') {
			deviceP = argv[i];
			break;
		}
	}
	if (!deviceP) {
		usage();
		return (1);
	}

	/* SMB device (PIC) address */
	optp = UTL_TSTOPT("a=");
	if (optp)
		sscanf( optp, "%x", &smbAddr );
	else
		smbAddr = DEFAULT_PIC_ADDR; /* 0xC0 */

	port = ((portStr = UTL_TSTOPT("p=")) ? atoi(portStr) : -1);
	if (port > 3) {
		printf("\n***ERROR: only ports 0..3 available!\n");
		usage();
		return (1);
	}
	if (port != (-1)) {
		portBit = 1 << port;
	} else {
		portStr = "0..3";
		portBit = 0x0f;
	}

	rev = (UTL_TSTOPT("r") ? 1 : 0);
	get = (UTL_TSTOPT("g") ? 1 : 0);
	set = (UTL_TSTOPT("s") ? 1 : 0);
	clr = (UTL_TSTOPT("c") ? 1 : 0);

	/*-------------------+
	|  verify arguments  |
	+-------------------*/
	if (get && (set || clr)) {
		printf("\n***ERROR: Don't use -s or -c and -g at the same time!\n");
		usage();
		return (1);
	}
	if (set && clr) {
		printf("\n***ERROR: Don't use -s and -c at the same time!\n");
		usage();
		return (1);
	}
	if (!get && !set && !clr && !rev) {
		printf("\n***ERROR: use at least one option (-s, -c, -g or -r) \n");
		usage();
		return (1);
	}

	/*--------------------+
	|  Init SMB2 library  |
	+--------------------*/
	err = SMB2API_Init(deviceP, &smbHdl);
	if (err) {
		PrintError("SMB2API_Init", err);
		ret = 1;
		goto EXIT;
	}

	/*--------------------+
	|  Get firmware Rev.  |
	+--------------------*/
	if (rev) {
		/* read firmware revision of PIC */
		err = SMB2API_ReadByteData(smbHdl, SMB_FLAGS, (u_int8)smbAddr, GET_FW_REV, (u_int8*)&rev);
		if (err) {
			PrintError("SMB2API_ReadByteData", err);
			goto ERR_EXIT;
		}
		printf("firmware revision: %d.%d\n", (rev >> 4), (rev & 0x0f));
		goto CLEAN;
	}

	/*---------------------------+
	|  Get current power states  |
	+---------------------------*/
	err = SMB2API_ReadByteData(smbHdl, SMB_FLAGS, (u_int8)smbAddr, GET_PWR_STAT, (u_int8*)&state);
	if (err) {
		PrintError("SMB2API_ReadByteData", err);
		goto ERR_EXIT;
	}

	/* Show power states */
	if (get) {
		if (port == (-1)) {
			printf("port : 3  2  1  0\n");
			printf("state: ");
			for (i = 3; i >= 0; i--)
				printf("%d  ", (state >> i) & 1);
			printf("\n\n");
		} else {
			printf("port #%d: %d\n", port, (state & portBit) ? 1 : 0);
		}
		goto CLEAN;
	}

	/* Clear power states */
	if (clr) {
		printf("disable PoE for port(s) #%s --> ", portStr);
		state &= ~portBit;
	}

	/* Set power states */
	if (set) {
		printf("enable PoE for port(s) #%s --> ", portStr);
		state |= portBit;
	}

	/*-------------------------+
	|  Set/Clear power states  |
	+-------------------------*/
	err = SMB2API_WriteByteData(smbHdl, SMB_FLAGS, (u_int8)smbAddr, SET_PWR_STAT, (u_int8)state);
	if (err) {
		printf("FAIL\n");
		PrintError("SMB2API_WriteByteData", err);
		goto ERR_EXIT;
	}
	printf("OK\n");
	goto CLEAN;


ERR_EXIT:
	ret = 1;

CLEAN:
	/*--------------------+
	|  Exit SMB2 library  |
	+--------------------*/
	err = SMB2API_Exit(&smbHdl);
	if (err) {
		ret = 1;
		PrintError("SMB2API_Exit", err);
	}

EXIT:
	return ret;
}

/******************************* PrintError ********************************/
/** Routine to print SMB2API/MDIS error message
 *
 *  \param info       \IN info string
 *  \param errCode    \IN error code number
 */
static void PrintError(char *info, int32 errCode)
{
	static char errMsg[512];

	if (!errCode) {
		errCode = UOS_ErrnoGet();
	}
	printf("*** can't %s: %s\n", info, SMB2API_Errstring(errCode, errMsg));
}
