/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_ctrl.c
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2014/10/15 13:00:46 $
 *    $Revision: 1.6 $
 *
 *        \brief Tool to access SMBus devices via the SMB2_API
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: smb2_ctrl.c,v $
 * Revision 1.6  2014/10/15 13:00:46  awerner
 * R: Compiler cause warning in scanf if u_int8 or u_int16 is used.
 * M: Use always u_int32 in scanf and cast the variable in SMB2API functions.
 *
 * Revision 1.5  2014/07/03 17:45:52  MRoth
 * R: scripting not possible, user input always necessary
 * M: added command mode interface including dump functionality for SMBus devices
 *
 * Revision 1.4  2009/06/22 11:59:15  dpfeuffer
 * R: MDVE warnings
 * M: added prefix SMB2CTRL to external globals and functions
 *
 * Revision 1.3  2006/05/31 08:22:13  DPfeuffer
 * added commands -id, -list
 *
 * Revision 1.2  2006/03/03 10:52:36  DPfeuffer
 * - main declaration fixed
 *
 * Revision 1.1  2006/02/28 15:57:31  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright by MEN Mikro Elektronik GmbH, Nuremberg, Germany 
 ****************************************************************************/
static const char RCSid[]="$Id: smb2_ctrl.c,v 1.6 2014/10/15 13:00:46 awerner Exp $";

#include "smb2_ctrl.h"
#include "cmd_tbl.h"
#include "ident.h"

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static char *G_dev;
static char *G_cmd;
u_int32     SMB2CTRL_errCount;
u_int32     SMB2CTRL_alertCallCount;
void        *SMB2CTRL_smbHdl;
u_int8      SMB2CTRL_flag;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static int32 Dispatch(void);

/**********************************************************************/
/**  Prints the headline
 */
static void header(void)
{
	printf(
		"===========================================================\n"
		" smb2_ctrl : Tool to access SMBus devices via the SMB2_API \n"
		"                                                           \n"
		" !!! Please be careful when you write to SMBus devices. !!!\n"
		" !!! Otherwise you may destroy important data (e.g. on  !!!\n"
		" !!! EEPROMs) or you may cause damage to the HW.        !!!\n"
		"                                                           \n"
		"===========================================================\n");
}

/**********************************************************************/
/**  Prints the program usage
 */
static void usage(void)
{
		printf( "\nUsage: smb2_ctrl <devName> [<-f>] [<command>] [<opts>]\n");
		printf( "       devName  : device name e.g. smb2_1             \n");
		printf( "       <-f>     : ask for flags                       \n");
		printf( "       --- command mode e.g. for scripting ---        \n");
		printf( "       command  : choose one of the following commands:    \n"
				"                  wb  : WriteByte       rb  : ReadByte     \n"
				"                  wbd : WriteByteData   rbd : ReadByteData \n"
				"                  wwd : WriteWordData   rwd : ReadWordData \n"
				"                  rbk : ReadBlockData   pc  : ProcessCall  \n"
				"                  q   : QuickComm                          \n"
				"                  list: list all available SMB devices     \n"
				"       [-a=hex]   address of SMB2 device\n"
				"       [-o=hex]   offset................................[0]\n"
				"       [-d=hex]   data to write                            \n"
				"       [-n=hex]   number of bytes for memory dump          \n"
				"       [-r]       read access for SMB2CTRL_QuickComm       \n"
				"\n"
				"Calling examples:\n"
				"\n--- User mode (extended functions) ---\n"
				"  smb2_ctrl smb2_1 \n"
				"\n--- Command mode ---\n"
				"- write single byte: \n"
				"    smb2_ctrl smb2_1 wbd -a=0xac -o=5 -d=0x12 \n"
				"- read 0x7f bytes: \n"
				"    smb2_ctrl smb2_1 rbd -a=0xae -n=0x7f \n"
				"\n(c)Copyright 2006 by MEN Mikro Elektronik GmbH\n%s\n", RCSid
		);
}

/**********************************************************************/
/** Main function.
 *
 *  \param argc		number of arguments
 *  \param argv		pointer to arguments
 *
 *  \return 0=ok, or error number
 */
int main(int argc, char* argv[])
{
	int32   ret=0, err=0;
	u_int32 smbAddr=0x0;
	u_int32 flags=0x0;
	u_int32 wordData=0;
	u_int32 byteData=0;
	u_int32 offs=0;
	int32   num=0, i=0;
	u_int8  readwrite;
	u_int8  length = SMB_BLOCK_MAX_BYTES;
	u_int8  blkData[SMB_BLOCK_MAX_BYTES];
	char    *errstr=NULL, ebuf[100];
	char    *optP=NULL;

	/* init globals */
	SMB2CTRL_flag = 0;
	SMB2CTRL_errCount = 0;
	SMB2CTRL_alertCallCount = 0;
	G_dev = NULL;
	G_cmd = NULL;

	/*------------------+
	|  Check arguments  |
	+------------------*/
	errstr = UTL_ILLIOPT( "?fra=o=d=n=F=", ebuf );
	if( errstr ) {
		printf( "*** %s\n", errstr );
		usage();
		return 1;
	}
	if( UTL_TSTOPT("?") ) {
		header();
		usage();
		return 0;
	}

	/*----------------+
	|  Get arguments  |
	+----------------*/
	for( i=1; i<argc; i++ ) {
		if( *argv[i] != '-' ) {
			if( G_dev == NULL ) {
				G_dev = argv[i];
			}
			else if( G_cmd == NULL ) {
				G_cmd = argv[i];
				break;
			}
		}
	}
	if( !G_dev ) {
		header();
		usage();
		return 0;
	}

	/*--------------------+
	|  init library       |
	+--------------------*/
	err = SMB2API_Init( G_dev, &SMB2CTRL_smbHdl );
	if( err ){
		SMB2CTRL_PrintStatus(err);
		ret=1;
		goto EXIT;
	}

	/*--------------------------+
	| User mode                 |
	+--------------------------*/
	if( G_cmd == NULL ) {
		
		SMB2CTRL_flag  = ( UTL_TSTOPT("f") ? 1 : 0 );    /* ask for SMB flag */
		
		header();
		
		SMB2CTRL_CmdHelp();
	
		/* enter command dispatcher */
		Dispatch();

	}
	/*--------------------------+
	| Command mode              |
	+--------------------------*/
	else{
		/* SMB device address */
		if( (optP = UTL_TSTOPT("a=")) ) {
			sscanf( optP, "%x", &smbAddr );
		}
		if( ((!smbAddr) && (strncmp(G_cmd, "list", 4))) ) {
			printf( "***ERROR: missing SMB address!\n" );
			goto CLEANUP;
		}

		/* SMB flags */
		if( (optP = UTL_TSTOPT("F=")) )
			sscanf( optP, "%x", &flags );
		else
			flags = 0x0;

		/*--------------------+
		| check SMB2 commands |
		+--------------------*/
		/* Dump SMBus */
		if( !(strncmp(G_cmd, "list", 4)) ) {
			SMB2CTRL_List();
		}
		/* Write Byte */
		else if( !(strncmp(G_cmd, "wb", 3)) ) {
			if( (optP = UTL_TSTOPT("d=")) ) {
				sscanf( optP, "%x", &byteData );
			}
			else {
				printf( "\n***ERROR: missing data!\n" );
				usage();
				ret = 1;
				goto CLEANUP;
			}
			err = SMB2API_WriteByte( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)byteData );
			SMB2CTRL_PrintStatus(err);
			if( err ){
				ret=1;
				goto CLEANUP;
			}
		}
		/* Read Byte */
		else if( !(strncmp(G_cmd, "rb", 3)) ) {
			err = SMB2API_ReadByte( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8*)&byteData );
			if( err ){
				SMB2CTRL_PrintStatus(err);
				ret=1;
				goto CLEANUP;
			}
			printf("%02x\n", byteData);
		}
		/* Write Byte Data */
		else if( !(strncmp(G_cmd, "wbd", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			if( (optP = UTL_TSTOPT("d=")) ) {
				sscanf( optP, "%x", &byteData );
			}
			else {
				printf( "\n***ERROR: missing data!\n" );
				usage();
				ret = 1;
				goto CLEANUP;
			}
			err = SMB2API_WriteByteData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, (u_int8)byteData );
			SMB2CTRL_PrintStatus(err);
			if( err ){
				ret=1;
				goto CLEANUP;
			}
		}
		/* Read Byte Data */
		else if( !(strncmp(G_cmd, "rbd", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			if( (optP = UTL_TSTOPT("n=")) ) {
				sscanf( optP, "%x", &num );
			}
			else{
				num=1;
			}
			for( i=0; i<num; i++ ) {
				err = SMB2API_ReadByteData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, ((u_int8)offs+(u_int8)i), (u_int8*)&byteData );
				if( err ){
					SMB2CTRL_PrintStatus(err);
					ret=1;
					goto CLEANUP;
				}
				printf( "%02x ", byteData );
				if( !((i+1)%16) ) {
					printf("\n");
				}
			}
			printf( "\n" );
		}
		/* Write Word Data */
		else if( !(strncmp(G_cmd, "wwd", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			if( (optP = UTL_TSTOPT("d=")) ) {
				sscanf( optP, "%x", &wordData );
			}
			else {
				printf( "\n***ERROR: missing data!\n" );
				usage();
				ret = 1;
				goto CLEANUP;
			}
			err = SMB2API_WriteWordData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, (u_int16)wordData );
			SMB2CTRL_PrintStatus(err);
			if( err ){
				ret=1;
				goto CLEANUP;
			}
		}
		/* Read Word Data */
		else if( !(strncmp(G_cmd, "rwd", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			if( (optP = UTL_TSTOPT("n=")) ) {
				sscanf( optP, "%x", &num );
			}
			else{
				num=1;
			}
			for( i=0; i<num*2; i+=2 ) {
				err = SMB2API_ReadWordData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, ((u_int8)offs+(u_int8)i), (u_int16*)&wordData );
				if( err ){
					SMB2CTRL_PrintStatus(err);
					ret=1;
					goto CLEANUP;
				}
				printf( "%04x ", wordData );
				if( !((i+2)%16) ) {
					printf("\n");
				}
			}
			printf( "\n" );
		}
		/* Read Block Data */
		else if( !(strncmp(G_cmd, "rbk", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			err = SMB2API_ReadBlockData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, &length, blkData );
			if( err ){
				SMB2CTRL_PrintStatus(err);
				ret=1;
				goto CLEANUP;
			}
			UTL_Memdump("Read data", (char*)blkData, length, 1);
		}
		/* QuickComm */
		else if( !(strncmp(G_cmd, "q", 1)) ) {
			readwrite = ( UTL_TSTOPT("r") ? SMB_READ : SMB_WRITE ); /* read(1) or write(0) access */
			err = SMB2API_QuickComm( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, readwrite );
			SMB2CTRL_PrintStatus(err);
			if( err ){
				ret=1;
				goto CLEANUP;
			}
		}
		/* Process Call */
		else if( !(strncmp(G_cmd, "pc", 2)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			if( (optP = UTL_TSTOPT("d=")) ) {
				sscanf( optP, "%x", &wordData );
			}
			else {
				printf( "\n***ERROR: missing data!\n" );
				usage();
				ret = 1;
				goto CLEANUP;
			}
			err = SMB2API_ProcessCall( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, (u_int16*)&wordData );
			if( err ){
				SMB2CTRL_PrintStatus(err);
				ret=1;
				goto CLEANUP;
			}
			printf("%04x\n", wordData);
		}
		/* Default */
		else {
			printf("\n***ERROR: SMB2 command not supported!\n");
			usage();
		}
	}

CLEANUP:
	/*--------------------+
	|  exit library       |
	+--------------------*/
	if( (err = SMB2API_Exit( &SMB2CTRL_smbHdl )) ){
		SMB2CTRL_PrintStatus(err);
		ret = 1;
	}
	
EXIT:
	printf("\nExiting program...\n");
	printf("Sum of all occured error(s): %d\n", SMB2CTRL_errCount);

	return ret;
}

/**********************************************************************/
/** Print command help.
 *
 *  \return 0,-1=ok, or error number
 */
extern int32 SMB2CTRL_CmdHelp( void )
{
	printf("\n"
		   "Commands\n"
		   "========\n"
		   " -h     : HELP (these command list)                      \n"
		   " -e     : EXIT                                           \n"
    	   "__________SMB2API___________\n"
		   " -q     : SMB2CTRL_QuickComm                                      \n"
		   " -wb    : SMB2CTRL_WriteByte          -rb    : SMB2CTRL_ReadByte           \n"
		   " -wbd   : SMB2CTRL_WriteByteData      -rbd   : SMB2CTRL_ReadByteData       \n"
		   " -wwd   : SMB2CTRL_WriteWordData      -rwd   : SMB2CTRL_ReadWordData       \n"
		   " -wbk   : SMB2CTRL_WriteBlockData     -rbk   : SMB2CTRL_ReadBlockData      \n"
		   " -pc    : SMB2CTRL_ProcessCall        -bpc   : SMB2CTRL_BlockProcessCall   \n"
		   " -ar    : SMB2CTRL_AlertResponse\n"
		   " -aci   : SMB2CTRL_AlertCbInstall     -acis  : SMB2CTRL_AlertCbInstallSig  \n"
		   " -acr   : SMB2CTRL_AlertCbRemove\n"
		   " -i2c   : SMB2CTRL_I2CXfer\n"
		   " -ers   : SMB2CTRL_Errstring          -id    : SMB2CTRL_Ident              \n"
    	   "__________TOOLS___________\n"
		   " -list  : SMB2CTRL_List all available SMB devices\n"
		   " -mt    : Simple memory test (e.g. for EEPROMS)\n"
		   "\n"
           );

    return -1;
}

/**********************************************************************/
/** Command dispatcher.
 *
 *  \return 0=ok, or error number
 */
static int32 Dispatch( void )
{
    int32   ii;             
	char	cmd[10];
	u_int8	found;

    while(1) {
		found = FALSE;
		
		printf("\ndev=%s ==> cmd: -", G_dev);

		fflush( stdin );
		scanf("%s",cmd);

		/* exit program */
		if (strcmp(cmd, "e") == 0) {
			return 0;
		}
		
		/* find the entered command */
		for(ii = 0; ii < NUMBER_OF_COMMANDS; ii++) {
			
			/* command found */
			if (strcmp(cmd, SMB2CTRL_commands[ii].command) == 0) {
				found = TRUE;

				/* dispatch command */
				SMB2CTRL_PrintStatus( SMB2CTRL_commands[ii].dispFunc() );	
				break;
			}
		}
		if( !found )
			printf("\007*** Unknown command\n"); 
	}
}

/**********************************************************************/
/** Print errCode.
 *
 *  \param errCode		error code
 */
extern void SMB2CTRL_PrintStatus( int32 errCode )
{
    static char errMsg[512];

	switch( errCode ){
	case -1:
		return;
	case 0:
		printf(" ------------------------- SUCCESS -------------------------\n" );
		break;
	default:
		printf(" ************************** ERROR **************************\n" );
		printf("\007 %s\n", SMB2API_Errstring( errCode, errMsg ) );
		SMB2CTRL_errCount++;
	}
}


 
