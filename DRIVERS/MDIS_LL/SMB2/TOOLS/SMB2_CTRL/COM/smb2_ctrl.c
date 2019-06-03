/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_ctrl.c
 *
 *      \author  dieter.pfeuffer@men.de
 *
 *        \brief Tool to access SMBus devices via the SMB2_API
 *
 *     Switches: -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright (c) 2019, MEN Mikro Elektronik GmbH
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

#include "smb2_ctrl.h"
#include "cmd_tbl.h"

/*
 * Macro for ignoring the return value of a function (e.g. scanf)
 */
#ifndef IGNORE_RET_VAL
#define IGNORE_RET_VAL(fun) { if (fun); }
#endif

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static const char IdentString[]=MENT_XSTR(MAK_REVISION);
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

#define CMD_LEN 		10
#define INPUT_BUF_LEN	1024  /* much larger than cmd buf */


/**********************************************************************/
/**  Prints the program usage
 */
static void usage(void)
{
	printf( "\n"
		"Usage   : smb2_ctrl <devName> [<-f>] [<command>] [<opts>]     \n"
		"Function: Access SMBus devices                                \n"
		"Options:\n"
		"    devName : SMB2 device name e.g. smb2_1                    \n"
		"+---------------------------------------------------------+\n"
		"| If you specify a command, the tool operates in Command  |\n"
		"| Mode (e.g. for scripting). Otherwise, the tool provides |\n"
		"| a Command Line Interface with extended functionality.   |\n"
		"+---------------------------------------------------------+\n"
		"Command Mode:\n"
		"    list : List SMB devices that accepts ReadByte commands.   \n"
		"           A few SMB devices accepts ReadByte commands only   \n"
		"           after other commands or not at all.                \n"
				"                  wb  : WriteByte       rb  : ReadByte     \n"
				"                  wbd : WriteByteData   rbd : ReadByteData \n"
				"                  wwd : WriteWordData   rwd : ReadWordData \n"
		"    rbk  : ReadBlockData       wbk  : WriteBlockData          \n"
		"    pc   : ProcessCall         q    : QuickComm               \n"
		"Command Mode Options:\n"
		"    [-r]      : read access for QuickComm                     \n"
		"    [-F=hex]  : set SMB flags (default=0x00)                  \n"
		"    [-a=hex]  : address of SMB2 device                        \n"
		"    [-o=hex]  : offset (default=0)                            \n"
		"    [-d=hex]  : data to write (non-block access - wbd/wwd)    \n"
		"    [hex ..]  : data to write (block access - wbk)            \n"
		"    [-n=hex]  : number of bytes for memory dump               \n"
		"CLI Mode Options:\n"
		"    <-f>      : ask for flags                                 \n"
				"Calling examples:\n"
		"  _____Command Mode_____                                      \n"
		"  - write byte:\n"
				"    smb2_ctrl smb2_1 wbd -a=0xac -o=5 -d=0x12 \n"
		"  - read 128 bytes:\n"
		"    smb2_ctrl smb2_1 rbd -a=0xae -n=0x80\n"
		"  - block write 3 bytes to offset 0x5:\n"
		"    smb2_ctrl smb2_1 wbk -a=0xae -o=0x05 0xaa 0xff 0xee\n"
		"  _____CLI Mode_____                                          \n"
		"  - execute CLI Mode:\n"
		"    smb2_ctrl smb2_1\n\n"
		"+------------------------------------------------------+\n"
		"!! Please be careful when you write to SMBus devices. !!\n"
		"!! Otherwise you may destroy important data (e.g. on  !!\n"
		"!! EEPROMs) or you may cause damage to the HW.        !!\n"
		"+------------------------------------------------------+\n");
    printf("\nCopyright (c) 2019, MEN Mikro Elektronik GmbH\n%s\n\n", IdentString)
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
	int32   smbAddr=0x0;
	u_int32 flags=0x0;
	u_int32 wordData=0;
	u_int32 byteData=0;
	u_int32 offs=0;
	u_int32	tmp=0;
	int32   i=0;
	int32   num=0;
	u_int8  readwrite;
	u_int8  length=0;
	u_int8  max_length = SMB_BLOCK_MAX_BYTES;
	u_int8  blkData[SMB_BLOCK_MAX_BYTES]={0};
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
		
		SMB2CTRL_CmdHelp();
	
		/* enter command dispatcher */
		Dispatch();

	}
	/*--------------------------+
	| Command mode              |
	+--------------------------*/
	else{
		if( UTL_TSTOPT("f") ) {
			printf( "*** You are in Command Mode! Please use param. -F= if you need to use flags!\n" );
			usage();
			goto CLEANUP;
		}
		
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
		/* Write Block Data */
		else if( !(strncmp(G_cmd, "wbk", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}			
			for( i=3; i<argc; i++ ) {
				if( *argv[i] != '-' ) {
					sscanf( argv[i], "%x", &tmp );
					blkData[length] = (u_int8)tmp;
					printf( "blkData[%d]=0x%02x\n",	length, blkData[length] );
					length++;
					if( length>(SMB_BLOCK_MAX_BYTES) ) {
						break;
					}
				}
			}
			err = SMB2API_WriteBlockData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, length, blkData );

			if( err ){
				SMB2CTRL_PrintStatus(err);
				ret=1;
				goto CLEANUP;
			}
		}
		/* Read Block Data */
		else if( !(strncmp(G_cmd, "rbk", 3)) ) {
			if( (optP = UTL_TSTOPT("o=")) ) {
				sscanf( optP, "%x", &offs );
			}
			err = SMB2API_ReadBlockData( SMB2CTRL_smbHdl, (u_int8)flags, (u_int8)smbAddr, (u_int8)offs, &max_length, blkData );
			if( err ){
				SMB2CTRL_PrintStatus(err);
				ret=1;
				goto CLEANUP;
			}
			UTL_Memdump("Read data", (char*)blkData, max_length, 1);
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
	printf("Sum of all occured error(s): %u\n", SMB2CTRL_errCount);

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
		   " -list  : List SMB devices that accepts ReadByte commands.\n"
		   "          A few SMB devices accepts ReadByte commands only\n"
		   "          after other commands or not at all.             \n"
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
	char	cmd[CMD_LEN];
	char	cmdtmp[INPUT_BUF_LEN];
	u_int8	found;

    while(1) {
		found = FALSE;
		
		printf("\ndev=%s ==> cmd: -", G_dev);

		fflush( stdin );
		
		IGNORE_RET_VAL(scanf("%s",cmdtmp));
		strncpy(cmd, cmdtmp, CMD_LEN);
		cmd[CMD_LEN-1] = 0; /* klocwork id5163 */

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


