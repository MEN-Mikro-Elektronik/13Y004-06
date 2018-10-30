/*****************************************************************************
 *************                                                    ************
 *************                      SMB2_BMC                      ************
 *************                                                    ************
 ****************************************************************************/
/*!
 *         \file  smb2_bmc.c
 *       \author  roman.schneider@men.de
 *        $Date: 2014/07/04 14:35:27 $
 *    $Revision: 1.2 $
 *
 *        \brief  Tool to control BMC features e.g. on F75P CPU boards
 *
 *     Required:  libraries: mdis_api, usr_oss, smb2_api
 *
 *--------------------------------[ History ]--------------------------------
 *
 * $Log: smb2_bmc.c,v $
 * Revision 1.2  2014/07/04 14:35:27  MRoth
 * R: malfunction of wdts/wdmts/rstcl/csg
 * M: a) replaced SMB2API_QuickComm with SMB2API_WriteByte function
 *    b) changed wdts, wdmts functions to 100/10 msec steps
 *    c) added BMC_RST_REASON_CLR_DATA in rstcl function
 *    d) fixed link state interpretation in csg function
 *
 * Revision 1.1  2014/01/07 17:38:02  MRoth
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2013 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
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
static const char RCSid[]="$Id: smb2_bmc.c,v 1.2 2014/07/04 14:35:27 MRoth Exp $";

/*--------------------------------------+
|    INCLUDES                           |
+--------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/smb2_api.h>
#include <MEN/bmc_api.h>
#include "cmd_tbl.h"

/*--------------------------------------+
|    DEFINES                            |
+--------------------------------------*/
#define SMB_FLAGS    0x0

/*--------------------------------------+
|    TYPEDEFS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|    GLOBALS                            |
+--------------------------------------*/
void      *G_smbHdl=NULL;
u_int32    G_smbAddr;

/*--------------------------------------+
|    PROTOTYPES                         |
+--------------------------------------*/
static void  PrintError    (char *info, int32 errCode);
static int32 WriteBlockData(u_int8 cmd_addr, u_int8 lenghtExpect, u_int8 *blkData);
static int32 ReadBlockData (u_int8 cmd_addr, u_int8 lenghtExpect, u_int8 *blkData);
static int32 ReadWordData  (u_int8 cmd_addr, u_int16 *wrdData);
static int32 WriteWordData (u_int8 cmd_addr, u_int16 wrdData);
static int32 WriteByte     (u_int8 bData);
static int32 WriteByteData (u_int8 cmdAddr, u_int8 bData);
static int32 ReadByteData  (u_int8 cmdAddr, u_int8 *bData);


/*********************************** Usage **********************************/
/** Prints the program usage
 *
 */
static void Usage( void )
{
	printf(
		"\nUsage:     smb2_bmc <dev-name> <smb-addr> <cmd> [<opts>]\n"
		"\nFunction:  Tool to control SMBus-BMCs (e.g. on F75P)\n"
		"           via BMC-API commands \n"
		"Options:\n"
		"  dev-name       : device name e.g. smb2_1 \n"
		"  smb-addr       : smb device address (default: 0x9C) \n"
		"  cmd            : command to execute \n"
		"\n"
		"Command List:                                             [Default]\n"
		"\n"
		"  --- FS_BMC_MANAGEMENT ---\n"
		"  gfr            : get firmware revision \n"
		"  ghwb           : get hw board \n"
		"  shwb <boardid> : set hw board - default: F75P............[0x0010]\n"
		"  feat           : features \n"
		"\n"
		"  --- FS_WATCHDOG ---\n"
		"  wdo            : wdog on \n"
		"  wdf            : wdog off \n"
		"  wdt     <opts> : wdog trigger time - in miliseconds.......[30000]\n"
		"  wdtg           : wdog time get \n"
		"  wdts    <opts> : wdog time set - 100ms steps(1=100ms)........[10]\n"
		"  wdsg           : wdog state get \n"
		"  wdam           : wdog arm \n"
		"  wdas           : wdog arm state \n"
		"  wdmts   <opts> : wdog minimum time set - 10ms steps(1=10ms)..[10]\n"
		"  wdmtg          : wdog minimum time get \n"
		"\n"
		"  --- FS_POWER_RESET_CONTROL ---\n"
		"  epfms   <opts> : ext. power fail mode set (0 or 1)............[0]\n"
		"  epfmg          : ext. power fail mode get \n"
		"  rims    <opts> : reset in mode set - enabled(0) masked(1).....[0]\n"
		"  rimg           : reset in mode get \n"
		"  swr     <opts> : sw reset \n"
		"  swcr    <opts> : sw cold reset \n"
		"  swrtcr  <opts> : sw rtc reset \n"
		"  swh     <opts> : sw halt \n"
		"  rstrg          : rst reason get \n"
		"  rstcl          : rst reason clr \n"
		"\n"
		"  --- FS_VOLTAGE_REPORTING ---\n"
		"  vmidx          : voltage max idx \n"
		"  vsidx   <opts> : voltage set idx \n"
		"  vget           : voltage get \n"
		"  vgeta          : voltage get all \n"
		"\n"
		"  --- FS_LIFETIME_REPORTING ---\n"
		"  pcc            : power cycle count \n"
		"  ohc            : operation hours count \n"
		"\n"
		"  --- FS_EVENT_LOG ---\n"
		"  evls           : evlog stat \n"
		"  evlw    <opts> : evlog write \n"
		"  evlwidx <opts> : evlog write idx \n"
		"  evlr    <opts> : evlog read \n"
		"\n"
		"  --- FS_STATUS_OUTPUTS ---\n"
		"  sog            : status output get \n"
		"  sos     <opts> : status output set \n"
		"\n"
		"  --- FS_CLUSTER_SUPPORT ---\n"
		"  csg            : cluster chan state get \n"
		"  css     <opts> : cluster chan state set \n"
		"\n"
		"\nCalling examples:\n"
		"\n - get firmware revision:\n"
		"     smb2_bmc smb2_1 0x9c gfr \n"
		"\n - software reset: \n"
		"     smb2_bmc smb2_1 0x9c swr \n"
		"\n"
		"\n(c)Copyright 2013 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid
	);
}


/****************************************************************************/
/** Program main function
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
int main( int argc, char *argv[] )
{
	char *deviceP=NULL;
	int err, i=0, ret=0;

	/*------------------+
	|  Check arguments  |
	+------------------*/
	/* at least four args required:
	 * argv[0] : smb2_bmc
	 * argv[1] : <driver-device-name>
	 * argv[2] : <smb-device-address>
	 * argv[3] : <command>
	 */
	if( argc < 4 ) {
		printf("\n*** not enough arguments! ***\n");
		Usage();
		return 1;
	}

	/*----------------+
	|  Get arguments  |
	+----------------*/
	deviceP = argv[1];

	/* SMB device(BMC) address */
	if( !(strncmp( argv[2], "0x", 2 )) ) {	/* hex address? */
		sscanf( argv[2], "%x", &G_smbAddr );
	}
	else{ /* default */
		G_smbAddr = BMC_DEFAULT_ADDR;
	}

	/*--------------------+
	|  Init SMB2 library  |
	+--------------------*/
	err = SMB2API_Init( deviceP, &G_smbHdl );
	if( err ) {
		PrintError( "SMB2API_Init", err );
		return 1;
	}

	/*------------------+
	|  execute command  |
	+------------------*/
	/* search the entered command */
	for( i=0; i<NUMBER_OF_COMMANDS; i++ ) {
		if( strcmp(argv[3], commands[i].command) == 0 ) {
			/* call service handler */
			ret = commands[i].Service(argc, argv);
		}
	}

	/*--------------------+
	|  Exit SMB2 library  |
	+--------------------*/
	err = SMB2API_Exit( &G_smbHdl );
	if( err ) {
		PrintError( "SMB2API_Exit", err );
		ret = 1;
	}

	return ret;
}


/****************************************************************************/
/** Get firmware revision
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 GetFirmwareRevision( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	ret = ReadBlockData( BMC_FIRMWARE_REV, 7, blkData );
	if( ret ) {
		return ret;
	}

	printf( "firmware revision: %d.%d.%d\n",
				blkData[1], blkData[2], blkData[3] );
	printf( "build number: %d\n", (u_int16)((blkData[5]<<8) | blkData[4]) );
	printf( "verified flag: 0x%x\n", blkData[6] );
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Get board name
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 GetHWBoard( int argc, char* argv[] )
{
	int32 ret=0, i;
	u_int16 wrdData;
	int boardtype=0;

	ret = ReadWordData( BMC_HW_BOARD, &wrdData );
	if( ret ) {
		return ret;
	}

	for( i=0; i<BMC_NBR_OF_BRDTYPES; i++ ) {

		if( wrdData == bmcBrdTypeTbl[i].boardCode ) {
			boardtype = 1;
			printf( "%s (code=0x%x)\n", bmcBrdTypeTbl[i].boardName, wrdData );
		}
	}

	if( !boardtype ) {
		printf( " Unknown board name (code=0x%x)\n", wrdData );
	}

	return 0;
}


/****************************************************************************/
/** Set board ID
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 SetHWBoard( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &wrdData );
	}
	else {
		wrdData = BMC_BOARD_CONSTANT_F75P;
	}

	ret = WriteWordData( BMC_HW_BOARD, wrdData );
	if( ret ) {
		return ret;
	}

	printf( " Board: 0x%x\n", wrdData );
	
	return 0;
}


/****************************************************************************/
/** Features
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Features( int argc, char* argv[] )
{
	int32 ret=0, i, j;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	ret = ReadBlockData( BMC_FEATURES, 9, blkData );
	if( ret ) {
		return ret;
	}

	for( i=0; i<BMC_FEATURE_NBR; i++) {
		for( j=0; j<BMC_FEATURE_NBR; j++ ) {
			if( blkData[i+1] & bmcFeatureTbl[i][j].mask ) {
				printf( "%s\n", bmcFeatureTbl[i][j].name );
			}
		}
	}

	return 0;
}


/****************************************************************************/
/** Watchdog on
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_On( int argc, char* argv[] )
{
	int32 ret=0;

	ret = WriteByte( BMC_WDOG_ON );
	if( ret ) {
		return ret;
	}

	printf( " WDog: On\n" );
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog off
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_Off( int argc, char* argv[] )
{
	int32 ret=0;

	ret = WriteByteData( BMC_WDOG_OFF, BMC_WDOG_OFF_DATA );
	if( ret ) {
		return ret;
	}

	printf( " WDog: Off\n" );
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog trigger
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_trigger( int argc, char* argv[] )
{
	int32 ret=0;
	int32 trigTime;
	u_int8 bData;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &trigTime );
	}
	else {
		trigTime = 30000;	/* 30sec */
	}

	/* check WDog state */
	ret = ReadByteData( BMC_WDOG_STATE_GET, &bData );
	if( ret ) {
		return ret;
	}
	
	if( bData == 0x01 ) {		/* WDOG enabled */

		printf( " Watchdog state: enabled -> trigger all %dmsec\n", trigTime );

		/* trigger loop */
		do {
			UOS_Delay( trigTime );
			ret = WriteByte( BMC_WDOG_TRIG );
			if( ret ) {
				return ret;
			}
			printf( " Watchdog triggered - Press any key to abort\n" );

		} while( UOS_KeyPressed() == -1 );
	}
	else {
		printf( " Watchdog state: disabled -> no trigger" );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog time get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_time_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData;

	ret = ReadWordData( BMC_WDOG_TIME_GET, &wrdData );
	if( ret ) {
		return ret;
	}

	if( wrdData == 0xFFFF ) {
		printf( " WDog time get: Error\n" );
	}
	else {
		/* print time in seconds */
		printf( " WDog time get: %d sec\n", wrdData/10 ); 
	}

	return 0;
}


/****************************************************************************/
/** Watchdog time set
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_time_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &wrdData );
	}
	else {
		wrdData = 10;	/* 1 sec */
	}

	ret = WriteWordData( BMC_WDOG_TIME_SET, wrdData );
	if( ret ) {
		return ret;
	}

	printf( " WDog time set to: %d msec\n", wrdData*100 );
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog state get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_state_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData;

	ret = ReadByteData( BMC_WDOG_STATE_GET, &bData );
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0x00:
			printf( " WDog state: Off\n" );
			break;
		case 0x01:
			printf( " WDog state: On\n" );
			break;
		case 0xFF:
			printf( " WDog state: Error (0xFF)\n" );
			break;
		default:
			printf( " *** WDog state error(code: 0x%0x)!\n", bData );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog arm
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_arm( int argc, char* argv[] )
{
	int32 ret=0;

	ret = WriteByte( BMC_WDOG_ARM );
	if( ret ) {
		return ret;
	}

	printf( " WDog armed\n" );
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog arm state
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_arm_state( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData;

	ret = ReadByteData( BMC_WDOG_ARM_STATE, &bData );
	if( ret ) {
		return ret;
	}

	switch (bData)
	{
		case 0x01:
			printf( " WDog arm state: Armed\n" );
			break;
		case 0x00:
			printf( " WDog arm state: Not armed\n" );
			break;
		case 0xFF:
			printf( " WDog arm state: Error\n" );
			break;
		default:
			printf( " *** WDog arm state error(code: 0x%0x)!\n", bData );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Watchdog minimum time set
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_min_time_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &wrdData );
	}
	else {
		wrdData = 10;	/* 100 msec */
	}

	ret = WriteWordData(BMC_WDOG_MIN_TIME_SET, wrdData);
	if( ret ) {
		return ret;
	}

	printf( " WDog minimum time set to: %d msec\n", wrdData*10 );
	printf( "\n" );
	
	return 0;
}


/****************************************************************************/
/** Watchdog minimum time get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 WDog_min_time_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData;

	ret = ReadWordData( BMC_WDOG_MIN_TIME_GET, &wrdData );
	if( ret ) {
		return ret;
	}

	switch( wrdData )
	{
		case 0xFFFF:
			printf(" WDog minimum time get: Error\n");
			break;
		case 0x0000:
			printf(" WDog minimum time get: Timeout\n");
			break;
		default:
			/* print time in seconds */
			printf( " WDog minimum time get: %d sec\n", wrdData/10 );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Ext. power fail mode set
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Ext_pwr_fail_mode_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0;

	
	if( argc > 4 ) {
		sscanf( argv[4], "%d", &bData );
	}

	ret = WriteByteData( BMC_EXT_PWR_FAIL_MODE, bData );
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0:
			printf( " Ext. power fail mode set to 'Ignore'(0)\n" );
			break;
		case 1:
			printf( " Ext. power fail mode set to 'Treat as error'(1)\n" );
			break;
		default:
			printf( " *** Error: Wrong value.\n Only 0 or 1 allowed.\n" );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Ext. power fail mode get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Ext_pwr_fail_mode_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0xFF;

	ret = ReadByteData( BMC_EXT_PWR_FAIL_MODE, &bData );
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0:
			printf( " Ext. power fail mode: Ignore(0)\n"
					" Assertion of external power failure signal is only logged.\n" );
			break;
		case 1:
			printf( " Ext. power fail mode: Treated as error(1)\n"
					" Assertion of external power failure is treated as an error.\n"
					" i.e. event is logged and board is reset.\n" );
			break;
		default:
			printf( " *** Ext. power fail mode get error(code:0x%0x)!\n", bData );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Reset in mode set
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Reset_in_mode_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0;

	
	if( argc > 4 ) {
		sscanf( argv[4], "%d", &bData );
	}

	ret = WriteByteData( BMC_RST_IN_MODE, bData );
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0:
			printf( " Reset in mode: Resets enabled(0)\n" );
			break;
		case 1:
			printf( " Reset in mode: Resets masked(1)\n" );
			break;
		default:
			printf( " *** Error: Wrong value.\n Only 0 or 1 allowed.\n" );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Reset in mode get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Reset_in_mode_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0xFF;

	ret = ReadByteData( BMC_RST_IN_MODE, &bData );
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0:
			printf( " Reset in mode: Resets enabled(0)\n" );
			break;
		case 1:
			printf( " Reset in mode: Resets masked(1)\n" );
			break;
		default:
			printf( " *** Reset in mode get error(code:0x%0x)!\n", bData );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** SW reset
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 SW_reset( int argc, char* argv[] )
{
	int32 ret=0, temp=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	blkData[0] = 0xa0;
	blkData[1] = 0xde;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &temp );

		/* divide the 16 bit variable into two 8 bit variables */
		blkData[2] = (u_int8)temp;
		blkData[3] = (u_int8)(temp>>8); 
	}

	ret = WriteBlockData( BMC_SW_RESET, 4, blkData );
	if( ret ) {
		return ret;
	}
	
	printf( "\n" );
	
	return 0;
}


/****************************************************************************/
/** SW cold reset
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 SW_cold_reset( int argc, char* argv[] )
{
	int32 ret=0, temp=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	blkData[0] = 0xa3;
	blkData[1] = 0xde;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &temp );

		/* divide the 16 bit variable into two 8 bit variables */
		blkData[2] = (u_int8)temp;
		blkData[3] = (u_int8)(temp>>8); 
	}

	ret = WriteBlockData( BMC_SW_COLD_RESET, 4, blkData );
	if( ret ) {
		return ret;
	}
	
	printf( "\n" );
	
	return 0;
}


/****************************************************************************/
/** SW RTC reset
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 SW_rtc_reset( int argc, char* argv[] )
{
	int32 ret=0, temp=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	blkData[0] = 0xa5;
	blkData[1] = 0xde;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &temp );

		/* divide the 16 bit variable into two 8 bit variables */
		blkData[2] = (u_int8)temp;
		blkData[3] = (u_int8)(temp>>8); 
	}

	ret = WriteBlockData( BMC_SW_RTC_RESET, 4, blkData );
	if( ret ) {
		return ret;
	}
	
	printf( "\n" );
	
	return 0;
}


/****************************************************************************/
/** SW halt
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 SW_halt( int argc, char* argv[] )
{
	int32 ret=0, temp=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	blkData[0] = 0x77;
	blkData[1] = 0xde;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &temp );

		/* divide the 16 bit variable into two 8 bit variables */
		blkData[2] = (u_int8)temp;
		blkData[3] = (u_int8)(temp>>8); 
	}

	ret = WriteBlockData(BMC_SW_HALT, 4, blkData);
	if( ret ) {
		return ret;
	}
	
	printf( "\n" );
	
	return 0;
}


/****************************************************************************/
/** Rst reason get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Rst_reason_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	ret = ReadBlockData( BMC_RST_REASON_GET, 8, blkData );
	if( ret ) {
		return ret;
	}

	if( blkData[0] == 0xff ) {
		printf(" Reset reason get : Error \n");
	}

	if( blkData[1] == 0 ) {
		printf(" Not processor specific\n");
	}
	else {
		printf(" Processor ID : %x \n", blkData[1]);
	}

	if( (blkData[2]==0) && (blkData[3]==0) ) {
		printf(" No reset was issued by the BMC since the reset reason register was cleared\n");
	}
	else {
		printf( " Event code: 0x%x%x \n", blkData[3], blkData[2] );
	}

	printf( " EV_INFO_1 : %x\n", blkData[4] );
	printf( " EV_INFO_2 : %x\n", blkData[5] );
	printf( " EV_INFO_3 : %x\n", blkData[6] );
	printf( " EV_INFO_4 : %x\n", blkData[7] );

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Rst reason clr
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Rst_reason_clr( int argc, char* argv[] )
{
	int32 ret=0;

	ret = WriteByteData( BMC_RST_REASON_CLR, BMC_RST_REASON_CLR_DATA );
	if( ret ) {
		return ret;
	}

	printf(" Reset reason : Cleared\n");

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** volt max idx
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Volt_max_idx( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData;

	ret = ReadByteData( BMC_VOLT_MAX_IDX, &bData);
	if( ret ) {
		return ret;
	}

	switch( bData )
	{
		case 0x00:
			printf( " No voltages are monitored\n" );
			break;
		case 0xFF:
			printf( " Max. volt idx : Error\n" );
			break;
		default:
			printf( " Ext. volt : 1 .. %d\n", bData );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** volt set idx
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Volt_set_idx( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0;

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &bData );
	}

	ret = WriteByteData( BMC_VOLTAGE_SET_IDX, bData );
	if( ret ) {
		return ret;
	}

	printf( " Set idx : %d\n", bData );
	
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** voltage get
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Volt_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	u_int16 erg;

	ret = ReadBlockData( BMC_VOLTAGE_GET, 9, blkData );
	if( ret ) {
		return ret;
	}

	if( blkData[0] == 0xff ) {
		printf( "***Error : invalid index, further response data is not valid!\n" );
	}
	else {
		printf( " Actual   : %d mV \n", erg= (((u_int16)blkData[2])<<8)+blkData[1] );
		printf( " Nominal  : %d mV \n", erg= (((u_int16)blkData[4])<<8)+blkData[3] );
		printf( " Low.lim. : %d mV \n", erg= (((u_int16)blkData[6])<<8)+blkData[5] );
		printf( " Up. lim. : %d mV \n", erg= (((u_int16)blkData[8])<<8)+blkData[7] );
		printf( "\n Info : Negative values cannot be expressed! \n" );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** get all voltages
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Volt_get_all( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 i=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	u_int16 erg;

	for( i=0; i<25; i++ ) {
		ret = WriteByteData( BMC_VOLTAGE_SET_IDX, i );
		if( ret ){
			printf("***Error: Could not set voltage index.\n");
			return ret;
		}

		ret = ReadBlockData( BMC_VOLTAGE_GET, 9, blkData );
		if( ret ) {
			printf( "***Error: Could not read voltage.\n" );
			return ret;
		}

		if( blkData[0] == 0xff ) {
			/* no valid voltage, so continue with other voltage index */
			continue;
		}

		if( i < 3 ) {
			printf( " EXT_VOLTAGE %d\n", i );
		}
		else if( i == 3 ) {
			printf( " EXT_STANDBY_VOLTAGE\n" );
		}
		else if( i == 4 ) {
			printf( " BATT_VOLTAGE\n" );
		}
		else {
			printf( " IMPL_SPEC %d\n", i );
		}

		if( (erg = (((u_int16)blkData[2])<<8)+blkData[1]) ) {
			printf( " Actual   : %d mV \n", erg= (((u_int16)blkData[2])<<8)+blkData[1] );
			printf( " Nominal  : %d mV \n", erg= (((u_int16)blkData[4])<<8)+blkData[3] );
			printf( " Low.lim. : %d mV \n", erg= (((u_int16)blkData[6])<<8)+blkData[5] );
			printf( " Up. lim. : %d mV \n\n", erg= (((u_int16)blkData[8])<<8)+blkData[7] );
		}
		else {
			printf( " Voltage not available\n\n" );
		}
	}
	
	printf( "\n Info : Negative values cannot be expressed!\n" );
	
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Pwrcycl cnt
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Pwrcycl_cnt( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	u_int32 erg=0;

	ret = ReadBlockData( BMC_PWRCYCL_CNT, 4, blkData );
	if( ret ) {
		return ret;
	}

	printf( " Power cycles : %d\n",
			erg=(((((((u_int32)blkData[3])<<8)+blkData[2])<<8)+blkData[1])<<8)+blkData[0] );

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Op hours count
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Op_hrs_cnt( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	u_int32 erg;

	ret = ReadBlockData( BMC_OP_HRS_CNT, 4, blkData );
	if( ret ) {
		return ret;
	}

	printf( " Operation time : %d hours\n",
			erg=(((((((u_int32)blkData[3])<<8)+blkData[2])<<8)+blkData[1])<<8)+blkData[0] );

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Evlog stat
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Evlog_stat( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	u_int16 erg;

	ret = ReadBlockData( BMC_EVLOG_STAT, 6, blkData );
	if( ret ) {
		return ret;
	}

	if( blkData[0] == 0xFF ) {
		printf( " Event log status : Error\n" );
	}

	if( blkData[1] == 1 ) {
		printf( " Timestamps are derived from RTC\n" );
	}
	else if( blkData[1] == 0 ) {
		printf( " Timestamps are derived from operation time counter\n" );
	}
	else {
		printf( " RTCTS: Error\n" );
	}

	printf( " Max. entries : %d\n", erg=(((u_int16)blkData[3])<<8)+blkData[2] );
	printf( " Act. entries : %d\n", erg=(((u_int16)blkData[5])<<8)+blkData[4] );
	
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Evlog write
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Evlog_write( int argc, char* argv[] )
{
	int32 ret=0, temp=0, temp1=0, temp2=0, temp3=0, temp4=0;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	if( argc > 4 ) {
		sscanf( argv[4], "%x", &temp );
		sscanf( argv[5], "%x", &temp1 );
		sscanf( argv[6], "%x", &temp2 );
		sscanf( argv[7], "%x", &temp3 );
		sscanf( argv[8], "%x", &temp4 );

		blkData[0] = (u_int8)temp;
		blkData[1] = (u_int8)(temp>>8);
		blkData[2] = (u_int8)temp1;
		blkData[3] = (u_int8)temp2;
		blkData[4] = (u_int8)temp3;
		blkData[5] = (u_int8)temp4;
	}
	else {
		temp = 0;
		blkData[0] = (u_int8)temp;
	}

	ret = WriteBlockData( BMC_EVLOG_WRITE, 6, blkData );
	if( ret ) {
		return ret;
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Evlog write idx
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Evlog_write_idx( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData=0;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &wrdData );
	}

	ret = WriteWordData( BMC_EVLOG_WRITE_IDX, wrdData );
	if( ret ) {
		return ret;
	}
	
	printf( " Event log index set to : %d\n", wrdData );
	
	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Evlog read
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Evlog_read( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 blkDatamax[SMB_BLOCK_MAX_BYTES], blkDataevent[SMB_BLOCK_MAX_BYTES];
	u_int16 erg, wrdData, i=0, max=255;
	u_int64 ausg;
	int32 end=0;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &end );
	}
	else {
		ret = ReadBlockData( BMC_EVLOG_READ, 13, blkDataevent );
		if( ret ) {
			return ret;
		}

		if( blkDataevent[0] == 0xFF ) {
			printf( " Event log : Error, further data invalid\n" );
		}

		printf( " Timestamp : %d seconds \n",
			ausg=(((((((((u_int64)blkDataevent[5] & 0x7F)<<8)+blkDataevent[4])<<8)
				+blkDataevent[3])<<8)+blkDataevent[2])<<8)+blkDataevent[1] );

		if( blkDataevent[5] & 0x80 ) {
			printf( " RTC time valid\n" );
		}
		else {
			printf( " RTC time invalid!\n" );
		}

		if(blkDataevent[6] == 0) {
			printf( " Not processor specific\n" );
		}
		else {
			printf( " Processor ID: %d\n", blkDataevent[6] );
		}

		printf( " Event Code : %x\n", erg=((u_int16)blkDataevent[8]<<8)+blkDataevent[7] );

		printf( " Event INFO_1 : 0x%04x\n", blkDataevent[9] );
		printf( " Event INFO_2 : 0x%04x\n", blkDataevent[10] );
		printf( " Event INFO_3 : 0x%04x\n", blkDataevent[11] );
		printf( " Event INFO_4 : 0x%04x\n", blkDataevent[12] );
		printf( "\n" );
	
		end = (-1);
	}
	
	if( end != (-1) ) {
		ret = ReadBlockData( BMC_EVLOG_STAT, 6, blkDatamax );
		if( ret ) {
			return ret;
		}

		ret = ReadWordData( BMC_HW_BOARD, &wrdData ); /* Get the board ID */
		if( ret ) {
			return ret;
		}

		max = ((((u_int16)blkDatamax[3])<<8)+blkDatamax[2]);

		printf( "Maximum = %d\n", max );

		if( (end>=0) && (end<=max) ) {

			for( i=0; i<=end; i++ ) {

				ret = WriteWordData( BMC_EVLOG_WRITE_IDX, i);
				if( ret ) {
					return ret;
				}

				ret = ReadBlockData(BMC_EVLOG_READ , 13, blkDataevent);
				if( ret ) {
					return ret;
				}

				if( blkDataevent[0] == 0xFF ) {
					printf( " Event log : Error, further data invalid\n" );
				}

				printf( " Timestamp : %d seconds \n",
					ausg=(((((((((u_int64)blkDataevent[5] & 0x7F)<<8)+blkDataevent[4])<<8)
						+blkDataevent[3])<<8)+blkDataevent[2])<<8)+blkDataevent[1] );

				if( blkDataevent[5] & 0x80 ) {
					printf( " RTC time valid\n" );
				}
				else {
					printf( " RTC time invalid!\n" );
				}
				if( blkDataevent[6] == 0 ) {
					printf( " Not processor specific\n" );
				}
				else {
					printf( " Processor ID : %d\n", blkDataevent[6] );
				}

				printf( " Event Code : %x\n", erg=((u_int16)blkDataevent[8]<<8)+blkDataevent[7] );

				printf( " Event INFO_1 : 0x%04x\n", blkDataevent[9] );
				printf( " Event INFO_2 : 0x%04x\n", blkDataevent[10] );
				printf( " Event INFO_3 : 0x%04x\n", blkDataevent[11] );
				printf( " Event INFO_4 : 0x%04x\n", blkDataevent[12] );
				printf( "\n" );
			}
		}
		else {
			printf( "***Error: Index %d not supported.\n", end );
		}
	}


	return 0;
}


/****************************************************************************/
/** Get status output
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Status_output_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData, erg=0;

	ret = ReadByteData( BMC_STATUS_OUTPUT_GET, &bData );
	if( ret ) {
		return ret;
	}

	if( bData & 0x01 ) {
		printf( " Current LED status : On\n" );
	}
	else {
		printf( " Current LED status : Off\n" );
	}
	
	if( bData & 0x02 ) {
		printf( " Current Hot swap LED status : On\n" );
	}
	else {
		printf( " Current Hot swap LED status : Off\n" );
	}

	printf( " Current user outputs status : %d\n", erg=(bData & 0x7C) );

	if( bData & 0x80 ) {
		printf( "***Error : Other bits are invalid\n" );
	}

	return 0;
}


/****************************************************************************/
/** Set status output
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Status_output_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int16 wrdData=0;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &wrdData );
	}

	ret = WriteWordData( BMC_STATUS_OUTPUT_SET, wrdData );
	if( ret ) {
		return ret;
	}

	printf( " Status output set to : %d\n", wrdData );

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Get cluster channel state
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Cluster_chan_state_get( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0;

	ret = ReadByteData( BMC_CLUS_CHAN_GET, &bData );
	if( ret ) {
		return ret;
	}

	if( bData & 0x80 ) {
		printf( " Link state : Link Fail\n" );
	}
	else {
		printf( " Link state : Link OK\n" );
	}
	
	bData &= 0x7f; /* only first 7 bit*/
	switch( bData )
	{
		case 0x00:
			printf( " Cluster chan. state : Disabled\n" );
			break;
		case 0x01:
			printf( " Cluster chan. state : Work-by\n" );
			break;
		case 0x02:
			printf( " Cluster chan. state : Active\n" );
			break;
		case 0x7F:
			printf( " Cluster chan. state : Error\n" );
			break;
		default:
			printf( " Cluster chan. state : Unknown\n" );
	}




	return 0;
}


/****************************************************************************/
/** Set cluster channel state
 *
 *---------------------------------------------------------------------------
 *  \param     argc    \IN argument counter
 *  \param     argv    \IN argument vector
 *
 *  \return    0 | error code
 *
 */
extern int32 Cluster_chan_state_set( int argc, char* argv[] )
{
	int32 ret=0;
	u_int8 bData=0;

	if( argc > 4 ) {
		sscanf( argv[4], "%d", &bData );
	}

	if( bData == 1 ) {
		ret = WriteByteData( BMC_CLUS_CHAN_SET, bData );
		if( ret ) {
			return ret;
		}
		printf( "State_set: %d\n", bData );
	}
	else {
		printf( " Illegal chan. state : Only Work-by (0x01) allowed\n" );
		printf( " Info : Other channel states cannot be commanded through this command.\n" );
		printf( "      \"Disabled\" state can be commanded indirectly via the Fs_power_reset_control.\n" );
		printf( "      \"Active\" state can be only entered if the BMC detects the other channel as disabled.\n" );
	}

	printf( "\n" );

	return 0;
}


/****************************************************************************/
/** Read one data block from a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param     cmdAddr         \IN device command or index value
 *  \param     lengthExpect    \IN number of bytes expected
 *  \param     blkData         \OUT read data block (1..32 bytes)
 *
 *  \return    0 | error code
 *
 */
static int32 ReadBlockData( u_int8 cmdAddr, u_int8 lengthExpect, u_int8 *blkData )
{
	int err=0;
	u_int8 lengthGet;

	err = SMB2API_ReadBlockData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, &lengthGet, blkData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_ReadBlockData", err );
		return err;
	}

	if( lengthGet != lengthExpect ) {
		printf( "*** Error: illegal length\n" );
		printf( "Length: %d\n", lengthGet );
		printf( "Lenght expected: %d\n", lengthExpect );
		return -1;
	}

	return 0;
}


/****************************************************************************/
/** Write a data block to a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param     cmdAddr         \IN device command or index value
 *  \param     lengthExpect    \IN number of bytes expected
 *  \param     blkData         \IN data block to write (1..32 bytes)
 *
 *  \return    0 | error code
 *
 */
static int32 WriteBlockData( u_int8 cmdAddr, u_int8 lengthExpect, u_int8 *blkData )
{
	int err=0;
	u_int8 lenghtSet;
	
	lenghtSet = lengthExpect;

	err = SMB2API_WriteBlockData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, lenghtSet, blkData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_WriteBlockData", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Read one data word to a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param     cmdAddr    \IN device command or index value
 *  \param     wrdData    \IN read word
 *
 *  \return    0 | error code
 *
 */
static int32 ReadWordData( u_int8 cmdAddr, u_int16 *wrdData )
{
	int err=0;

	err = SMB2API_ReadWordData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, wrdData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_ReadWordData", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Write one data word to a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param     cmdAddr    \IN device command or index value
 *  \param     wrdData    \IN word to write
 *
 *  \return    0 | error code
 *
 */
static int32 WriteWordData( u_int8 cmdAddr, u_int16 wrdData )
{
	int err=0;

	err = SMB2API_WriteWordData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, wrdData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_WriteWordData", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Write command to a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param    bData      \IN byte to write
 *
 *  \return    0 | error code
 *
 */
static int32 WriteByte( u_int8 bData )
{
	int err=0;

	err = SMB2API_WriteByte( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, bData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_WriteByte", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Write one data byte to a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param    cmdAddr    \IN device command or index value
 *  \param    bData      \IN byte to write
 *
 *  \return    0 | error code
 *
 */
static int32 WriteByteData( u_int8 cmdAddr, u_int8 bData )
{
	int err=0;

	err = SMB2API_WriteByteData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, bData );

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_WriteByteData", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Read one data byte from a SMB device
 *
 *---------------------------------------------------------------------------
 *  \param     cmdAddr    \IN device command or index value
 *  \param     bData      \OUT read byte
 *
 *  \return    0 | error code
 *
 */
static int32 ReadByteData( u_int8 cmdAddr, u_int8 *bData )
{
	int err=0;

	err = SMB2API_ReadByteData( G_smbHdl, SMB_FLAGS, 
				(u_int16)G_smbAddr, cmdAddr, bData);

	UOS_Delay(20); /* wait 20ms */

	if( err ) {
		PrintError( "SMB2API_ReadByteData", err );
		return err;
	}

	return 0;
}


/****************************************************************************/
/** Routine to print SMB2API/MDIS error message
 *
 *---------------------------------------------------------------------------
 *  \param     info       \IN info string
 *  \param     errCode    \IN error code number
 *
 *  \return    void
 *
 */
static void PrintError( char *info, int32 errCode )
{
	static char errMsg[512];

	if( !errCode ) {
		errCode = UOS_ErrnoGet();
	}

	printf( "***can't %s: %s\n", info, SMB2API_Errstring( errCode, errMsg ) );

}

