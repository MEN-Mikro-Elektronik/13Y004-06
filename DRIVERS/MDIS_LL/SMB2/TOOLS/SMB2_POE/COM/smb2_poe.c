/****************************************************************************
 ************                                                    ************
 ************                    SMB2_POE                        ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  smb2_poe.c
 *      \author  michael.roth@men.de
 *        $Date: 2014/12/17 11:27:29 $
 *    $Revision: 1.5 $
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
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: smb2_poe.c,v $
 * Revision 1.5  2014/12/17 11:27:29  MRoth
 * R: 1) enabling single PoE ports not working
 *    2) cosmetics
 * M: 1) removed double bit shifting
 *    2a) revised complete code
 *     b) changed/added parameters -s and -c
 *
 * Revision 1.4  2014/10/15 13:00:57  awerner
 * R: disable warnings (Win Secure functions) with pragma cause compiler error in linux
 * M: Added WINNT define to only disable warnings in Windows
 *
 * Revision 1.3  2014/07/03 19:05:06  MRoth
 * R: build error
 * M: fixed typo
 *
 * Revision 1.2  2014/07/03 17:45:59  MRoth
 * R: tool not working as expected
 * M: fixed data for SMBus write commands
 *
 * Revision 1.1  2014/02/20 18:22:32  MRoth
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2014 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/
static const char RCSid[]="$Id: smb2_poe.c,v 1.5 2014/12/17 11:27:29 MRoth Exp $";

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
	printf("(c)Copyright 2014 by MEN Mikro Elektronik GmbH\n%s\n", RCSid);
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
	char  *str, *portStr;
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
