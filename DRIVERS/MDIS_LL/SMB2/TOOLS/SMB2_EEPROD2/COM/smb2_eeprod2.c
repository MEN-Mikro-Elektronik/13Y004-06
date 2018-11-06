/******************************************************************************
 *************                                                    *************
 *************                    SMB2_EEPROD2                    *************
 *************                                                    *************
 ******************************************************************************/
/*!
 *        \file  smb2_eeprod2.c
 *      \author  michael.roth@men.de
 *        $Date: 2014/10/15 13:00:52 $
 *    $Revision: 1.12 $
 *
 *       \brief  Tool to program the board information EEPROM and SMBus devices
  *             (e.g. SPD EEPROMs) from binary files via the SMB2_API
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
 *    \switches  _BIG_ENDIAN_/_LITTLE_ENDIAN_
 *
 *-----------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ******************************************************************************/
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

static const char RCSid[]="$Id: smb2_eeprod2.c,v 1.12 2014/10/15 13:00:52 awerner Exp $";

/*--------------------------------------+
|   INCLUDES                            |
+--------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>			/* for toupper() */
#include <time.h>			/* for system time functions */
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>
#include <MEN/eeprod.h>		/* for EEPROD2 struct/constants */

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
# pragma warning(disable:4996)
#endif

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define DEFAULT_ID_PROM_ADDR	0xAE
#define SPD_WP_ADDR_A			0x60
#define SPD_WP_ADDR_B			0x64
#define SMB_FLAGS				0x0
#define MIN_SERIAL_NO			0x0
#define MAX_SERIAL_NO			0xFFFF	/* 65535 should be enough */
#define ZZ						0xEE
#define LINE_BUF				80

/*--------------------------------------+
|   MAKROS                              |
+--------------------------------------*/
#ifdef _BIG_ENDIAN_
# define SWAPWORD(w) (w)
# define SWAPLONG(l) (l)
#elif defined(_LITTLE_ENDIAN_)
# define SWAPWORD(w) ( (((w)&0xff)<<8) + (((w)&0xff00)>>8) )
# define SWAPLONG(l) ( (((l)&0xff)<<24) + (((l)&0xff00)<<8) + \
						(((l)&0xff0000)>>8) + (((l)&0xff000000)>>24) )
#else
# error "Must define _BIG_ENDIAN_ or _LITTLE_ENDIAN_"
#endif

#if defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
# error "Don't define _BIG/_LITTLE_ENDIAN_ together"
#endif

/*--------------------------------------+
|   TYPEDEFS                            |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
void    *SMB2EEPROD2_smbHdl;
EEPROD2 G_eeprd2;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static int32   GetSerial( u_int32* );
static int32   SmbIdPromRead( u_int8 );
static int32   SmbIdPromWrite( u_int8 );
static void    SmbIdPromDump( void );
static int32   SmbSPDWriteProtect( u_int8 );
static int32   SmbProgramFile( u_int8, char* );
static u_int16 GetDateFormat( u_int16, u_int8, u_int8 );
static void    GetSystemDate( u_int16*, u_int8*, u_int8* );
static int32   CheckParity( void );
static u_int8  CalcParity( u_int8*, u_int32 );
static void    PrintError( char*, int32 );


/************************************ usage ***********************************/
/** Prints the program usage
 */
static void usage( void )
{
	printf(
		"\nUsage:     smb2_eeprod2  devName  [board]  [<opts>] \n"
		"\nFunction:  - write/read data to/from board information EEPROMs"
		"\n           - program SMB devices from file (e.g. SPD EEPROMs)"
		"\n           - lock (write protect) SPD EEPROMs"
		"\n           - program hardware ID to BMC"
		"\nOptions: \n"
		"   devName   device name e.g. smb2_1\n"
		"   [board]   board name e.g. MM01- or F018E\n"
		"   [-d]      dump board information EEPROM\n"
		"   [-e]      erase board information EEPROM \n"
		"   [-s]      write protect SPD EEPROM\n"
		"   [-a=hex]  address of SMB2 device - default: 0xae\n"
		"   [-n=dec]  serial number\n"
		"   [-r=dec]  Revision number - default: 00.00.00\n"
		"   [-m=dec]  model number    - default: 00\n"
		"   [-p=dec]  production date - default: current system date\n"
		"             - format: yyyy-mm-dd\n"
		"   [-l]      last repair date (current system date)\n"
		"   [-f=str]  program file data into SMB devices e.g. SPD EEPROMs\n"
		"             - file must be binary formatted\n"
		"\nCalling examples:\n"
		"\n- dump board information EEPROM: \n"
		"    smb2_eeprod2 smb2_1 -d \n"
		"\n- program EEPROM of F18E on address 0xae with default values:\n"
		"    smb2_eeprod2 smb2_1 F018E \n"
		"\n- program EEPROM at address 0xaa and specified prod. date: \n"
		"    smb2_eeprod2 smb2_1 MM01- -a=0xaa -p=2009-07-20 \n"
		"\n- program only new revision number and repair date at address 0xae:"
		"\n    smb2_eeprod2 smb2_1 -r=00.01.00 -l \n"
		"\n- program file data to SMB device (e.g. SPD EEPROM): \n"
		"    smb2_eeprod2 smb2_1 -f=08SC25-00IC200_300.bin -a=0xa0 \n"
		"\n- write-protect SPD EEPROM: \n"
		"    smb2_eeprod2 smb2_1 -a=0x60 -s \n"
		"\n(c)Copyright 2009 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid
	);
}


/******************************************************************************/
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
	char    input=0;
	int     dump, erase, spd, proddate=0, repdate=0, revision=0;
	u_int32 serial=0, modelno=0, smbAddr=0x0;
	u_int32 prodYYYY=0, prodDD=0, prodMM=0;
	u_int32 repYYYY=0, repDD=0, repMM=0;
	u_int32 rev[3]={0};
	u_int8  byte, par=0;
	u_int8  *arr;
	char    *boardnameP=NULL, *deviceP=NULL, *filenameP=NULL;

	/*------------------+
	|  Check arguments  |
	+------------------*/
	errstr = UTL_ILLIOPT( "?desn=a=r=m=p=lf=", ebuf );
	if( errstr ) {
		printf( "*** %s\n", errstr );
		usage();
		ret=1;
		goto EXIT;
	}
	if( UTL_TSTOPT("?") ) {
		usage();
		goto EXIT;
	}

	/*----------------+
	|  Get arguments  |
	+----------------*/
	for( i=1; i<argc; i++ ) {
		if( *argv[i] != '-' ) {
			if( deviceP == NULL ) {
				deviceP = argv[i];
			}
			else if( boardnameP == NULL ) {
				boardnameP = argv[i];
				break;
			}
		}
	}

	if( !deviceP ) {
		usage();
		return 0;
	}

	dump  = ( UTL_TSTOPT("d") ? 1 : 0 );	/* dump EEPROM? */
	erase = ( UTL_TSTOPT("e") ? 1 : 0 );	/* erase EEPROM? */
	spd   = ( UTL_TSTOPT("s") ? 1 : 0 );		/* lock spd? */

	/* model */
	optp = UTL_TSTOPT("m=");
	if( optp ) {
		sscanf( optp, "%d", &modelno );
		if( (optp[0] == 'Z') || (optp[0] == 'z') ) {
			modelno = ZZ;
		}
	}

	/* SMB device(EEPROM) address */
	optp = UTL_TSTOPT("a=");
	if( optp ) {
		sscanf( optp, "%x", &smbAddr );
	}
	else {
		smbAddr = DEFAULT_ID_PROM_ADDR;		/* 0xAE */
	}

	/* serial number */
	optp = UTL_TSTOPT("n=");
	if( optp ) {
		sscanf(optp, "%d", &serial);
	}
	else {
		serial=0;
	}

	/* file name */
	filenameP = UTL_TSTOPT("f=");

	/* revision of board */
	optp = UTL_TSTOPT("r=");
	if( optp ) {
		revision = 1;
		if( !sscanf(optp,"%2d.%2d.%2d",&rev[0],&rev[1],&rev[2]) ) {
			printf( "\n***ERROR: rev.# %s\n", optp );
			ret=1;
			goto EXIT;
		}
	}

	/* production date */
	optp = UTL_TSTOPT("p=");
	if( optp ) {
		proddate = 1;
		if( !sscanf(optp,"%4d-%2d-%2d", &prodYYYY, &prodMM, &prodDD) ) {
			printf( "\n***ERROR: prod.date %s\n", optp );
			ret=1;
			goto EXIT;
		}
	}
	else {
		GetSystemDate( (u_int16*)&prodYYYY, (u_int8*)&prodDD, (u_int8*)&prodMM );
	}

	/* date of last repair */
	optp = UTL_TSTOPT("l");
	if( optp ) {
		repdate = 1;
		GetSystemDate( (u_int16*)&repYYYY, (u_int8*)&repDD, (u_int8*)&repMM );
	}

	if( dump && erase ) {
		printf( "\n***ERROR: only one of these options (-d OR -e) "
				"are allowed at the same time!\n" );
		usage();
		ret=1;
		goto EXIT;
	}

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
	printf( "\nSMB device %s at address 0x%02x\n", deviceP, smbAddr );
	
	/*---------------------+
	|  Program SPD EEPROM  |
	+---------------------*/
	if( filenameP != NULL ) {
		/* program SMB device */
		err = SmbProgramFile( (u_int8)smbAddr, filenameP );
		if( err == 0 ) {
			printf( "\nSMB device programmed successfully.\n" );
		}
		else if( err == 1 ) {
			printf( "\n***ERROR programming SMB device!\n" );
			ret = err;
		}
		else {
			printf( "\nOperation cancelled.\n" );
		}
		goto CLEANUP;
	}

	/*------------------+
	|  Lock SPD EEPROM  |
	+------------------*/
	if( spd ) {
		/* read byte for validation */
		err = SMB2API_ReadByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS, (u_int8)smbAddr, 0x0, &byte );
		if( err ) {
			PrintError( "SMB2API_ReadByteData", err );
			ret=1;
			goto CLEANUP;
		}
		/* lock SPD EEPROM */
		err = SmbSPDWriteProtect( (u_int8)smbAddr );
		if( err == 0 ) {
			printf( "\nSPD EEPROM locked successfully.\n" );
		}
		else if( err == 1 ) {
			printf( "\n***ERROR while locking EEPROM!\n" );
			ret = err;
		}
		else {
			printf( "\nOperation cancelled.\n" );
		}
		goto CLEANUP;
	}

	/* read current content of EEPROM */
	ret = SmbIdPromRead( (u_int8)smbAddr );
	if( ret ) {
		printf( "\n***ERROR getting EEPROM access\n" );
		goto CLEANUP;
	}

	/*-----------------+
	|  Dump ID EEPROM  |
	+-----------------*/
	if( dump ) {
		/* check EEPROD2 ID and parity */
		if( (G_eeprd2.pd_id == 0xFF) || ( ((G_eeprd2.pd_id >> 4) != EEID_PD2) &&
										((G_eeprd2.pd_id >> 4) != EEID_PD) ) ) {
			printf( "\n***ERROR - Invalid or empty EEPROM!!!\n" );
			printf( "\nDUMP EEPROM (INVALID!!):\n" );
			ret=1;
		}
		else if( CheckParity() != 0 ) {
			printf( "\nCHKSUM ERROR - Invalid EEPROM!!!\n" );
			printf( "\nDUMP EEPROM (INVALID!!):\n" );
			ret=1;
		}
		else {
			printf( "\nDUMP EEPROM:\n" );
			ret=0;
		}

		SmbIdPromDump();
		/* display RAW data */
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
		goto CLEANUP;
	}

	/*------------------+
	|  Erase ID EEPROM  |
	+------------------*/
	if( erase ) {
		printf( "\nATTENTION!!! Do you really want to erase "
				"the board informations from EEPROM?\n"
				"Press 'y' to continue, or any other key to exit: " );
		fflush( stdin );
		if( scanf("%c", &input) ) {
			switch( input ) {
				case 'y':
					printf( "\nERASE EEPROM\n ." );
					memset( &G_eeprd2, 0xFF, sizeof(EEPROD2) );
					ret = SmbIdPromWrite( (u_int8)smbAddr );
					if( ret == 0 ) {
						printf( "\nEEPROM erased successfully.\n" );
					}
					else {
						printf( "\n***ERROR erasing EEPROM!\n" );
					}
					break;
				default:
					printf( "\nOperation cancelled.\n" );
			}
		}
		else {
			ret=1;
		}
		goto CLEANUP;
	}

	/*---------------------+
	|  Initialize EEPROD2  |
	+---------------------*/
	/* check revision number */
	if( revision ) {
		if( (rev[0] > 0xFF) || (rev[1] > 0xFF) || (rev[2] > 0xFF) ) {
			printf( "\n***ERROR: revision number %u.%u.%u wrong!\n",
											rev[0], rev[1], rev[2] );
			ret=1;
			goto CLEANUP;
		}
		else {
			G_eeprd2.pd_revision[0] = (u_int8)(rev[0] & 0x00FF);
			G_eeprd2.pd_revision[1] = (u_int8)(rev[1] & 0x00FF);
			G_eeprd2.pd_revision[2] = (u_int8)(rev[2] & 0x00FF);
		}
		if( (!boardnameP) && (!repdate) ) {
			goto PROGRAM;
		}
	}
	else {
	/* check, if there is already a revision number in EEPROM */
		if( G_eeprd2.pd_revision[0] == 0xFF ) {
			G_eeprd2.pd_revision[0] = (u_int8)rev[0];
			G_eeprd2.pd_revision[1] = (u_int8)rev[1];
			G_eeprd2.pd_revision[2] = (u_int8)rev[2];
		}
	}

	/* get last repair date */
	if( repdate ) {
		G_eeprd2.pd_repdat = SWAPWORD(GetDateFormat( (u_int16)repYYYY,
												(u_int8)repMM, (u_int8)repDD) );
		if( G_eeprd2.pd_repdat == 1 ) {
			ret=1;
			goto CLEANUP;
		}
		if( !boardnameP ) {
			goto PROGRAM;
		}
	}

	/* check boardname */
	if( !boardnameP ) {
		printf( "\n***ERROR: missing boardname!\n" );
		usage();
		ret=1;
		goto CLEANUP;
	}
	else {
		if( strlen(boardnameP) > (sizeof(G_eeprd2.pd_hwName)-1) ) {
			printf( "\n***ERROR: boardname too long!\n" );
			ret=1;
			goto CLEANUP;
		}
		else {
		/* check if letters of boardname are small letters */
			for( i=0; i<sizeof(boardnameP); i++ ) {
				if( boardnameP[i] > 90 ) { /* ASCII value for 'Z', 0x5A */
					boardnameP[i] = (char)toupper( boardnameP[i] );
				}
			}
			strncpy( G_eeprd2.pd_hwName, boardnameP, sizeof(G_eeprd2.pd_hwName) );
			/* truncate string */
			if( G_eeprd2.pd_hwName[sizeof(G_eeprd2.pd_hwName)-1] != '\0' ) {
				G_eeprd2.pd_hwName[sizeof(G_eeprd2.pd_hwName)-1] = '\0';
			}
		}
	}

	/* get production date */
	if( proddate ) {
		G_eeprd2.pd_prodat = SWAPWORD( GetDateFormat( (u_int16)prodYYYY,
									(u_int8)prodMM, (u_int8)prodDD ) );
	}
	else {
	/* check, if there is already a production date in EEPROM */
		if( G_eeprd2.pd_prodat == 0xFFFF ) {
			G_eeprd2.pd_prodat = SWAPWORD( GetDateFormat( (u_int16)prodYYYY,
										(u_int8)prodMM, (u_int8)prodDD ) );
		}
	}
	if( (G_eeprd2.pd_prodat == 1) || (G_eeprd2.pd_prodat == 0xFFFF) ) {
		ret=1;
		goto CLEANUP;
	}

	/* get serial number */
	while(1) {
		if(!serial){
			printf( "\nPlease enter serial number or 'q' to quit:\n--> " );
			fflush( stdout );
			if( GetSerial( &serial ) == 1 ) {	/* exit */
				ret=0;
				printf( "\nOperation cancelled.\n" );
				goto CLEANUP;
			}
		}
		if( (serial > MIN_SERIAL_NO) && (serial < MAX_SERIAL_NO) ) {
			break;
		}
		else {
			printf( "*** %d is not a valid serial number "
					"between 1 and 65534!\n", serial );
			printf( "Please try again with a valid serial number!\n" );
			serial=0;
		} /* end else */
} /* end while(1) */

	G_eeprd2.pd_serial = SWAPLONG(serial);

	/* model number */
	G_eeprd2.pd_model = (u_int8)modelno;

PROGRAM:
	/*-----------------+
	|  Program EEPROM  |
	+-----------------*/
	/* calc parity */
	arr = (u_int8*)&G_eeprd2;
	par = CalcParity( &arr[1], (sizeof(EEPROD2)-1) );
	G_eeprd2.pd_id = (u_int8)( par | (EEID_PD2 << 4) );

	printf( "\nPROGRAMMING EEPROM\n ." );

	ret = SmbIdPromWrite( (u_int8)smbAddr );
	if( ret == 0 ) {
		/* Check parity */
		printf( "\nVERIFY EEPROM: \n" );
		SmbIdPromRead( (u_int8)smbAddr );
		if( CheckParity() == 1 ) {
			printf( "\n***CHECKSUM ERROR: EEPROM parity not valid!\n" );
			ret=1;
		}
		else {
			SmbIdPromDump();
			printf( "\nEEPROM programmed successfully.\n" );
		}
	}
	else {
		printf( "\n***ERROR programming EEPROM!\n" );
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

/********************************** GetSerial *********************************/
/** Routine to ask the user for the serial number of the CPU board.
 *
 *  \param serial	\OUT serial number
 *
 *  \return			success (0) or error (1)
 */
static int32 GetSerial( u_int32 *serial )
{
	char line[LINE_BUF];

	memset( line, '\0', sizeof(line) );

	fflush( stdin );
	if( fgets(line, LINE_BUF, stdin) ) {
		if( strncmp( line, "q", 1 ) == 0 ) {
			return 1;
		}
		else{
			sscanf( line, "%d", serial );
			return 0;
		}
	}
	return 1;
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

/******************************* SmbIdPromWrite *******************************/
/** Routine to write data from EEPROD2 struct to a SMB ID-EEPROM.
 *
 *  \param smbAddr	\IN address of SMB device
 *
 *  \return			success (0) or error (1)
 */
static int32 SmbIdPromWrite( u_int8 smbAddr )
{
	int err, ret=0;
	u_int8 i=0;
	u_int8 *arr;

	arr = (u_int8*)&G_eeprd2;

	for( i=0; i<sizeof(EEPROD2); i++ ) {
		err = SMB2API_WriteByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS, smbAddr, i, arr[i] );
		UOS_Delay(20); /* wait 20ms */
		if( err ) {
			PrintError( "SMB2API_WriteByteData", err );
			ret = 1;
		}
		else {
			printf( "." );
		}
		UOS_Delay(20); /* wait 20ms */
	}
	UOS_Delay(50); /* wait 50ms */

	printf( "\n" );

	return ret;
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
		printf( " Serial#    = 0x%X\n", (SWAPLONG(G_eeprd2.pd_serial)) );
	}
	else {
		printf( " Serial#    = %08u\n", (SWAPLONG(G_eeprd2.pd_serial)) );
	}

	if( (G_eeprd2.pd_revision[0] == 0xFF) || (G_eeprd2.pd_revision[1] == 0xFF)
										|| (G_eeprd2.pd_revision[2] == 0xFF) ) {
		printf( " Revision   = %02X.%02X.%02X\n", G_eeprd2.pd_revision[0],
													G_eeprd2.pd_revision[1],
													G_eeprd2.pd_revision[2] );
	}
	else {
		printf( " Revision   = %02d.%02d.%02d\n", G_eeprd2.pd_revision[0],
													G_eeprd2.pd_revision[1],
													G_eeprd2.pd_revision[2] );
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

/***************************** SmbSPDWriteProtect *****************************/
/** Routine to lock SPD EEPROMs
 *
 *  \param smbAddr	\IN address of SMB device
 *
 *  \return			success (0) or error (1)
 */
static int32 SmbSPDWriteProtect( u_int8 smbAddr )
{
	int  err=0, ret=1;
	char input=0;

	printf( "\nATTENTION!!! Do you really want to lock the SPD EEPROM?\n"
			"Press 'y' to continue or any other key to exit: " );
	fflush( stdin );
	if( scanf("%c", &input) ) {
		switch( input ) {
			case 'y':
				printf( "\nLOCK SPD EEPROM\n" );
				/* write one byte */
				if( smbAddr == SPD_WP_ADDR_A ) {
					err = SMB2API_WriteByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS,
													SPD_WP_ADDR_A, 0x0, 0x12 );
					UOS_Delay(20); /* wait 20ms */
					ret=0;
				}
				else if( smbAddr == SPD_WP_ADDR_B ) {
					err = SMB2API_WriteByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS,
													SPD_WP_ADDR_B, 0x0, 0x14 );
					UOS_Delay(20); /* wait 20ms */
					ret=0;
				}
				else {
					err = SMB2API_WriteByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS,
														  smbAddr, 0x0, 0x14 );
					UOS_Delay(20); /* wait 20ms */
					ret=0;
				}
				if( err ) {
					PrintError( "SMB2API_WriteByteData", err );
					ret=1;
				}
				break;

			default:
				ret=2;
		}
	}
	else {
		ret=1;
	}

	return ret;
}

/******************************* SmbProgramFile ******************************/
/** Routine to program SMB devices e.g. SPD EEPROMs from binary file 
 *
 *  \param smbAddr		\IN address of SMB device
 *  \param filenameP	\IN name of binary file
 *
 *  \return				success (0) or error (1)
 */
static int32 SmbProgramFile( u_int8 smbAddr, char *filenameP )
{
	int    err=0, ret=0;
	char   input=0;
	int    filesize, offs;
	FILE   *fileP = NULL;
	u_int8 *buf=NULL;

	printf( "\nATTENTION!!! Do you really want to program the SMB device?\n"
			"Press 'y' to continue or any other key to exit: " );
	fflush( stdin );
	if( scanf("%c", &input) ) {
		if( input == 'y') {
			printf( "\nPROGRAM SMB DEVICE\n" );
			fileP = fopen( filenameP, "rb" );
			if( fileP == NULL ){
				PrintError( "fopen", err );
				return 1;
			}
			/* Determine size of input file */
			fseek( fileP, 0, SEEK_END );
			filesize = ftell( fileP );
			fseek( fileP, 0, SEEK_SET );
			
			buf = malloc(filesize*sizeof(char));
			if( buf == NULL ) {
				PrintError( "malloc", err );
				return 1;
			}
			if( fread( buf, 1, filesize, fileP ) != (size_t)filesize ){
				PrintError( "fread", err );
				return 1;
			}
			
			for( offs=0; offs<=(filesize-1); offs++ ) {
				err = SMB2API_WriteByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS,
										smbAddr, (u_int8)offs, buf[offs] );
				if( err ) {
					PrintError( "SMB2API_WriteByteData", err );
					ret = 1;
				}
				else {
					printf( "." );
				}
				UOS_Delay(20); /* wait 20ms */
			}
			printf( "\n" );
			for( offs=0; offs<=(filesize-1); offs++ ) {
				err = SMB2API_ReadByteData( SMB2EEPROD2_smbHdl, SMB_FLAGS,
										smbAddr, (u_int8)offs, &buf[offs] );
				if( err ) {
					PrintError( "SMB2API_ReadByteData", err );
					ret = 1;
				}
				else {
					printf(" %02X", buf[offs] );
					if( !((offs+1)%16) ) {
						printf("\n");
					}
				} 
			} /* for */
		} /* if */
		else{
			ret=2;
		}
	}
	else {
		ret=1;
	}

	/* close file handle */
	if( fileP ) {
		fclose(fileP);
	}
	/* free memory */
	if( buf ) {
		free(buf);
	}

	return ret;
}

/******************************* GetDateFormat ********************************/
/** Routine to get the right date format for EEPROD2 structure
 *
 *  \param year		\IN year in decimal format
 *  \param day		\IN day in decimal format
 *  \param month	\IN month in decimal format
 *
 *  \return			formdate
 */
static u_int16 GetDateFormat( u_int16 year, u_int8 month, u_int8 day )
{
	u_int16 formdate=0;

	if( (year < 1990) || (year > 2117) ) {
		printf( "***ERROR wrong date: year = %d\n", year );
		return 1;
	}
	if( (month == 0) || (month > 12) ) {
		printf( "***ERROR wrong date: month = %d\n", month );
		return 1;
	}
	if( (day == 0) || (day > 31) ) {
		printf( "***ERROR wrong date: day = %d\n", day );
		return 1;
	}

	year -= EEPROD2_DATE_YEAR_BIAS;

/*
  - Bits 15..9 (7 bits) contain the year since 1990 in binary format.
				This allows a range from 1990..2117
  - Bits  8..5 (4 bits) contain the month in binary format. (1..12)
  - Bits  4..0 (5 bits) contain the day of month in binary format (1..31)
*/
	/* do the shifting */
	formdate = year & 0x007F;
	formdate <<= 4;

	formdate |= month & 0x0F;
	formdate <<= 5;

	formdate |= day & 0x1F;

	return formdate;
}

/******************************* GetSystemDate ********************************/
/** Routine to get the systemdate
 *
 *  \param year   \OUT year in decimal format
 *  \param day    \OUT day in decimal format
 *  \param month  \OUT month in decimal format
 */
static void GetSystemDate( u_int16* year, u_int8* day, u_int8* month )
{
	char amonth[3], aday[3], ayear[5];
	struct tm *timeP;
	time_t systime;

	/* get current time */
	time( &systime );
	timeP = localtime( &systime );
	/* get month */
	strftime( amonth, sizeof(amonth), "%m", timeP );
	*month = (u_int8)( atoi( amonth ) );
	/* get day */
	strftime( aday, sizeof(aday), "%d", timeP );
	*day = (u_int8)( atoi( aday ) );
	/* get year */
	strftime( ayear, sizeof(ayear), "%Y", timeP );
	*year = (u_int16)( atoi( ayear ) );
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
