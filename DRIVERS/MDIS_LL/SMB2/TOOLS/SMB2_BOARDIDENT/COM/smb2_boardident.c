/****************************************************************************
 ************                                                    ************
 ************                  SMB2_BOARDIDENT                   ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *		   \file smb2_boardident.c
 *		 \author michael.roth@men.de
 *
 *        \brief Tool to dump board informations from SMBus devices via the SMB2_API
 *
 *               - init SMB2_API library (SMB2API_Init)
 *               - read data from SMB device (SMB2API_ReadByteData)
 *               - show data from SMB device
 *               - exit SMB2_API library (SMB2API_Exit)
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *     \switches _BIG_ENDIAN_/_LITTLE_ENDIAN_
 *
 *
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 2009-2019, MEN Mikro Elektronik GmbH
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
|   INCLUDES                           |
+-------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>
#include <MEN/eeprod.h>		/* for EEPROD2 struct/constants  */

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
# pragma warning(disable:4996)
#endif

/*-------------------------------------+
|   DEFINES                            |
+-------------------------------------*/
#define DEFAULT_ID_PROM_ADDR    0xAE
#define SMB_FLAGS    0x0
#define ZZ           0xEE

/*-------------------------------------+
|   MAKROS                             |
+-------------------------------------*/
#ifdef _BIG_ENDIAN_
# define SWAPWORD(w) (w)
# define SWAPLONG(l) (l)
#elif defined(_LITTLE_ENDIAN_)
# define SWAPWORD(w) ((((w)&0xff)<<8) + (((w)&0xff00)>>8))
# define SWAPLONG(l) ((((l)&0xff)<<24) + (((l)&0xff00)<<8) + \
                     (((l)&0xff0000)>>8) + (((l)&0xff000000)>>24))
#else
# error "Must define _BIG_ENDIAN_ or _LITTLE_ENDIAN_"
#endif

#if defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
# error "Don't define _BIG/_LITTLE_ENDIAN_ together"
#endif

/*-------------------------------------+
|   GLOBALS                            |
+-------------------------------------*/
void    *SMB2EEPROD2_smbHdl;
EEPROD2 G_eeprd2;

/*-------------------------------------+
|   PROTOTYPES                         |
+-------------------------------------*/
static int32   SmbIdPromRead( u_int8 );
static void    SmbIdPromDump( void );
static int32   CheckParity( void );
static u_int8  CalcParity( u_int8*, u_int32 );
static void    PrintError( char*, int32 );

/********************************* header **********************************/
/**  Prints the headline
 */
static void header(void)
{
	printf("\n==========================="
		   "\n===   SMB2_BOARDIDENT   ==="
		   "\n===========================");
    printf("\nCopyright (c) 2009-2019, MEN Mikro Elektronik GmbH\n%s\n\n", IdentString);
}

/********************************* usage ***********************************/
/**  Prints the program usage
 */
static void usage(void)
{
	printf(
		"\nUsage:    smb2_boardident  devName  [smbAddr]  [<opts>] \n"
		"\nFunction: read data from SMB board information EEPROM \n"
		"Options:\n"
		"   devName    SMB device name e.g. smb2_1\n"
		"   [smbAddr]  SMB address of the board information EEPROM - default: 0xae\n"
		"   [-r]       dump raw board information EEPROM data \n"
		"\nCalling examples:\n"
		"\n- dump board information EEPROM (with raw data): \n"
		"    smb2_boardident smb2_1 -r \n"
		"\n- dump data from specified SMB address: \n"
		"    smb2_boardident smb2_1 0xac  \n");
    printf("\nCopyright (c) 2009-2019, MEN Mikro Elektronik GmbH\n%s\n\n", IdentString);
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
	int     err, ret=0, i=0;
	char    *errstr=NULL, ebuf[100];

	int     raw;
	char    *deviceP=NULL, *addrP=NULL;
	u_int32 smbAddr=0x0;
	u_int8  byte;

	/*--------------------+
	|  check arguments    |
	+--------------------*/
	errstr = UTL_ILLIOPT("?r", ebuf);  /* check args */
	if(errstr) {
		printf("*** %s\n", errstr);
		usage();
		return 1;
	}
	if( UTL_TSTOPT("?") ) {
		usage();
		return 0;
	}

	/*----------------+
	|  Get arguments  |
	+----------------*/
	for( i=1; i<argc; i++ ) {
		if( *argv[i] != '-' ) {
			if( deviceP == NULL ) {
				deviceP = argv[i];
			}
			else if( addrP == NULL ) {
				addrP = argv[i];
				break;
			}
		}
	}

	if( !deviceP ) {
		usage();
		return 0;
	}

	/* SMB device(EEPROM) address */
	if( addrP != NULL ) {
		sscanf( addrP, "%x", (unsigned int*)&smbAddr );
	}
	else {
		smbAddr = DEFAULT_ID_PROM_ADDR; /* 0xae */
	}

	/* show raw data ? */
	raw = ( UTL_TSTOPT("r") ? 1 : 0 );

	/*--------------------+
	|  Init SMB2 library  |
	+--------------------*/
	err = SMB2API_Init( deviceP, &SMB2EEPROD2_smbHdl );
	if( err ) {
		PrintError( "SMB2API_Init", err );
		ret=1;
		goto EXIT;
	}

	/* some output at the beginning */
	header();
	printf("Accessing %s: smbAddr 0x%2x\n", deviceP, (unsigned int)smbAddr);

	/* read content of EEPROM */
	ret = SmbIdPromRead( (u_int8)smbAddr );
	if( ret == 1 ) {
		printf("***ERROR getting EEPROM access\n");
		goto CLEANUP;
	}

	/* check EEPROD2 ID and parity */
	if( (G_eeprd2.pd_id == 0xFF) || ( ((G_eeprd2.pd_id >> 4) != EEID_PD2) &&
									  ((G_eeprd2.pd_id >> 4) != EEID_PD) ) ) {
		printf( "\n***ERROR - Invalid or empty EEPROM!!!\n" );
		ret = 1;
		goto CLEANUP;
	}
	else {
		/*-----------------+
		|  dump IDPROM     |
		+-----------------*/
		if( (G_eeprd2.pd_id >> 4) == EEID_PD2 ) {
			/* check validity of EEPROM */
			if( CheckParity() != 0 ) {
				printf( "\nCHKSUM ERROR - Invalid EEPROM!!!\n" );
				printf( "\nDUMP EEPROM (INVALID!!):\n" );
				ret = 1;
			}
			else {
				printf("\nDUMP EEPROM:\n");
				ret = 0;
			}
		}
		else {  /* EEID_PD */
			printf("\nDUMP EEPROM:\n");
			ret = 0;
		}

		SmbIdPromDump();

		/* display RAW data */
		if( raw ) {
			printf("\nRAW EEPROM DATA: \n\n");
			for( i=0; i<sizeof(EEPROD2); i++ ) {
				err = SMB2API_ReadByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS, (u_int8)smbAddr, (u_int8)i, &byte );
				UOS_Delay(10); /* wait 10ms */
				if( err ) {
					PrintError( "SMB2API_ReadByteData", err );
					ret=1;
				}
				printf(" %02X", byte );
				if( !((i+1)%8) ) {
					printf("\n");
				}
			}
		}
	}

CLEANUP:
	/*--------------------+
	|  Exit SMB2 library  |
	+--------------------*/
	err = SMB2API_Exit( &SMB2EEPROD2_smbHdl );
	if( err ) {
		PrintError( "SMB2API_Exit", err );
		ret=1;
	}

EXIT:
	return ret;
}

/******************************* SmbIdPromRead ********************************/
/** Routine to read data from SMB2 ID-EEPROM to the EEPROD2 struct.
 *
 *  \param smbAddr	\IN address of SMB device
 *
 *  \return			success (0) or error (1)
 */
static int32 SmbIdPromRead( u_int8 smbAddr )
{
	int    err=0;
	u_int8 i=0;
	u_int8 *arr;
	
	arr = (u_int8*)&G_eeprd2;

	for( i=0; i<sizeof(EEPROD2); i++ ) {
		err = SMB2API_ReadByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS, smbAddr, i, &arr[i] );
		UOS_Delay(10); /* wait 10ms */
		if( err ) {
			PrintError( "SMB2API_ReadByteData", err );
			return 1;
		}
	}
	UOS_Delay(50); /* wait 50ms */

	return 0;
}

/******************************* SmbIdPromDump ********************************/
/** Routine to print data of the SMB2 ID-EEPROM (EEPROD/EEPROD2)
 */
static void SmbIdPromDump(void)
{
	u_int8  prodday=0, prodmonth=0;
	u_int8  repday=0, repmonth=0;
	u_int16 prodyear=0, repyear=0;

	printf( "\n EEPROD-ID  = 0x%X\n", (G_eeprd2.pd_id >> 4) );

	if( G_eeprd2.pd_serial == 0xFFFFFFFF ) {
		printf( " Serial#    = 0x%X\n", (unsigned int)(SWAPLONG(G_eeprd2.pd_serial)) );
	}
	else {
		printf( " Serial#    = %08u\n", (unsigned int)(SWAPLONG(G_eeprd2.pd_serial)) );
	}

	if( (G_eeprd2.pd_revision[0] == 0xFF) || (G_eeprd2.pd_revision[1] == 0xFF)
	    || (G_eeprd2.pd_revision[2] == 0xFF) ) {
		printf( " Revision   = %02X.%02X.%02X\n", G_eeprd2.pd_revision[0],
			G_eeprd2.pd_revision[1], G_eeprd2.pd_revision[2] );
	}
	else {
		printf( " Revision   = %02d.%02d.%02d\n", G_eeprd2.pd_revision[0],
			G_eeprd2.pd_revision[1], G_eeprd2.pd_revision[2] );
	}

	/* handle HW name */
	if( ((u_int8)G_eeprd2.pd_hwName[0]) == 0xFF ) {
		printf( " HW-Name    = FF%X-%02X\n",
					(u_int8)G_eeprd2.pd_hwName[0], G_eeprd2.pd_model );
	}
	else {
		if( G_eeprd2.pd_model == ZZ ) {
			printf( " HW-Name    = %sZZ\n", G_eeprd2.pd_hwName );
		}
		else {
			printf( " HW-Name    = %s%02u\n", G_eeprd2.pd_hwName, G_eeprd2.pd_model );
		}
	}

	/* get ProdDat */
	G_eeprd2.pd_prodat = SWAPWORD(G_eeprd2.pd_prodat);
	prodday = G_eeprd2.pd_prodat & 0x001F;
	prodmonth = (G_eeprd2.pd_prodat >> 5) & 0x000F;
	prodyear  = (G_eeprd2.pd_prodat  >> 9) & 0x007F;
	prodyear += EEPROD2_DATE_YEAR_BIAS;

	if( G_eeprd2.pd_prodat == 0xFFFF ) {
		printf( " Prod. Date = FFFF-FF-FF\n" );
	}
	else {
		printf( " Prod. Date = %04u-%02u-%02u\n", prodyear, prodmonth, prodday );
	}

	/* get RepDat */
	G_eeprd2.pd_repdat = SWAPWORD(G_eeprd2.pd_repdat);
	repday	= G_eeprd2.pd_repdat & 0x001F;
	repmonth = (G_eeprd2.pd_repdat >> 5) & 0x000F;
	repyear  = (G_eeprd2.pd_repdat  >> 9) & 0x007F;
	repyear += EEPROD2_DATE_YEAR_BIAS;

	if (G_eeprd2.pd_repdat == 0xFFFF) {
		printf( " Rep. Date  = FFFF-FF-FF\n" );
	}
	else {
		printf( " Rep. Date  = %04u-%02u-%02u\n", repyear, repmonth, repday );
	}

}

/******************************** CheckParity *********************************/
/** Routine to check the parity of the board information EEPROM
 *  \return			success (0) or error (1)
 */
static int32 CheckParity( void )
{
	u_int8 chksum=0, parity=0;
	u_int8 *arr;
	int error=0;

	arr = (u_int8*)&G_eeprd2;

	/* check parity */
	chksum = G_eeprd2.pd_id & 0x0F;
	parity = CalcParity( &arr[1], (sizeof(EEPROD2)-1) );
	if( parity != chksum ) {
		error = 1; /* Checksum wrong */
	}
	/* we only report an error if we don't have a valid parity */
	return error;
}

/********************************* CalcParity *********************************/
/** Routine to compute the 4-bit parity over a number of bytes
 *
 *  \param ptr	\IN pointer to EEPROD2
 *  \param len	\IN sizeof(EEPROD2)
 *
 *  \return		parity
 */
static u_int8 CalcParity( u_int8 *ptr, u_int32 len )
{
	u_int8 parity = 0xF;

	while( len-- ) {
		parity ^= (*ptr >> 4);
		parity ^= (*ptr & 0xf);
		ptr++;
	}

	return parity;
}

/********************************* PrintError *********************************/
/** Routine to print SMB2API/MDIS error message
 *
 *  \param info		\IN info string
 *  \param errCode	\IN error code number
 */
static void PrintError( char *info, int32 errCode )
{
	static char errMsg[512];

	if( !errCode )
		errCode = UOS_ErrnoGet();

	printf( "*** can't %s: %s\n", info, SMB2API_Errstring( errCode, errMsg ) );

}
