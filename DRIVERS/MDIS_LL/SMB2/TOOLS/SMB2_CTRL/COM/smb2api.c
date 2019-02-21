/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2api.c
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2014/02/20 18:21:46 $
 *    $Revision: 1.6 $
 *
 *        \brief SMB2_API functions
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

/*
 * Macro for ignoring the return value of a function (e.g. scanf)
 */
#ifndef IGNORE_RET_VAL
#define IGNORE_RET_VAL(fun) { if (fun); }
#endif

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static void AlertCbFunc( void *cbArg );
static int internFunc( int argc, char **argv );

#define STR_XFER_LEN    20

/**********************************************************************/
/** Ask user for specified parameters
 */
static int32 AskUser(
	unsigned int 	*flags,
	unsigned int *addr,
	u_int8		*cmdAddr,
	u_int8		*byteData,
	u_int16		*wordData,
	u_int8		*readWrite
){
	unsigned int 	tmp;
	int ac=0;
	char *pc=NULL;
	/* satisfy vxWorks compiler.. */
	tmp = internFunc( ac, &pc);
	
	if( flags ){
		if( SMB2CTRL_flag ){
			printf(" flags -> 0x");
			fflush( stdin );
			IGNORE_RET_VAL(scanf("%x", flags));
		}
		else
			*flags = 0;
	}
	if( addr ){
		printf(" device address -> 0x");
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%x", &tmp));
		*addr = (u_int16)tmp;
	}
	if( cmdAddr ){
		printf(" device command or index value -> 0x");
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%x", &tmp));
		*cmdAddr = (u_int8)tmp;
	}
	if( byteData ){
		printf(" byte to write -> 0x");
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%x", &tmp));
		*byteData = (u_int8)tmp;
	}
	if( wordData ){
		printf(" word to write -> 0x");
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%x", &tmp));
		*wordData = (u_int16)tmp;
	}
	if( readWrite ){
		do{
			printf(" read or write (r/w) -> ");
			fflush( stdin );
			IGNORE_RET_VAL(scanf("%c", (char*)&tmp));
			switch( tmp ){
				case 'r': *readWrite = SMB_READ; break;
				case 'w': *readWrite = SMB_WRITE; break;
			}
		} while( *readWrite > 1 );
	}
	return 0;
}

/**********************************************************************/
/** Ask user for specified block parameters
 */
static int32 AskUserBlk(
	u_int8		*length,
	u_int8		*blkData )
{
	u_int8	n;
	u_int8	maxLength = *length;
	int	tmp=0;

	printf(" number of bytes to write (1..%d)", maxLength);
	/* limit user input */
	do{
		printf(" -> ");
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%d", &tmp));
		*length = (u_int8)tmp;
	} while( *length > maxLength );
	
	for( n=0; n < *length; n++ ){
		printf(" blkData[%d] -> 0x", n);
		fflush( stdin );
		IGNORE_RET_VAL(scanf("%x", &tmp));
		blkData[n] = (u_int8)tmp;
	}

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_QuickComm
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_QuickComm()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		rw;

	printf("SMB2CTRL_QuickComm:\n");
	AskUser( &flags, &addr, 0, 0, 0, &rw );

	return SMB2API_QuickComm( SMB2CTRL_smbHdl, flags, addr, rw );
}

/**********************************************************************/
/** SMB2CTRL_WriteByte
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_WriteByte()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		byteData;

	printf("SMB2CTRL_WriteByte:\n");
	AskUser( &flags, &addr, 0, &byteData, 0, 0 );

	return SMB2API_WriteByte( SMB2CTRL_smbHdl, flags, addr, byteData );
}

/**********************************************************************/
/** SMB2CTRL_ReadByte
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_ReadByte()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		byteData;
	int32		err;

	printf("SMB2CTRL_ReadByte:\n");
	AskUser( &flags, &addr, 0, 0, 0, 0 );

	err = SMB2API_ReadByte( SMB2CTRL_smbHdl, flags, addr, &byteData );
	if( err )
		return err;

	printf(" read byte: 0x%02x\n", byteData);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_WriteByteData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_WriteByteData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int8		byteData;

	printf("SMB2CTRL_WriteByteData:\n");
	AskUser( &flags, &addr, &cmdAddr, &byteData, 0, 0 );

	return SMB2API_WriteByteData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, byteData );
}

/**********************************************************************/
/** SMB2CTRL_ReadByteData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_ReadByteData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int8		byteData;
	int32		err;

	printf("SMB2CTRL_ReadByteData:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, 0, 0 );

	err = SMB2API_ReadByteData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &byteData );
	if( err )
		return err;

	printf(" read byte: 0x%02x\n", byteData);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_WriteWordData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_WriteWordData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int16		wordData;

	printf("SMB2CTRL_WriteWordData:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, &wordData, 0 );

	return SMB2API_WriteWordData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, wordData );
}

/**********************************************************************/
/** SMB2CTRL_ReadWordData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_ReadWordData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int16		wordData;
	int32		err;

	printf("SMB2CTRL_ReadWordData:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, 0, 0 );

	err = SMB2API_ReadWordData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &wordData );
	if( err )
		return err;

	printf(" read word: 0x%04x\n", wordData);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_WriteBlockData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_WriteBlockData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int8		length=SMB_BLOCK_MAX_BYTES;
	u_int8		blkData[SMB_BLOCK_MAX_BYTES];

	printf("SMB2CTRL_WriteBlockData:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, 0, 0 );
	AskUserBlk( &length, blkData );

	return SMB2API_WriteBlockData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, length, blkData );
}

/**********************************************************************/
/** SMB2CTRL_ReadBlockData
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_ReadBlockData()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int8		length;
	u_int8		blkData[SMB_BLOCK_MAX_BYTES];
	int32		err;

	printf("SMB2CTRL_ReadBlockData:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, 0, 0 );

	err = SMB2API_ReadBlockData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &length, blkData );
	if( err )
		return err;

	UTL_Memdump("Read data", (char*)blkData, length, 1);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_ProcessCall
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_ProcessCall()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int16		wordData;
	int32		err;

	printf("SMB2CTRL_ProcessCall:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, &wordData, 0 );

	err = SMB2API_ProcessCall( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &wordData );
	if( err )
		return err;

	printf(" read word: 0x%04x\n", wordData);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_BlockProcessCall
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_BlockProcessCall()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddr;
	u_int8		readLen;
	u_int8		writeLen=SMB_BLOCK_MAX_BYTES;
	u_int8		writeData[SMB_BLOCK_MAX_BYTES];
	u_int8		readData[SMB_BLOCK_MAX_BYTES];
	int32		err;

	printf("SMB2CTRL_BlockProcessCall:\n");
	AskUser( &flags, &addr, &cmdAddr, 0, 0, 0 );
	AskUserBlk( &writeLen, writeData );

	err = SMB2API_BlockProcessCall( SMB2CTRL_smbHdl, flags, addr, cmdAddr,
			writeLen, writeData, &readLen, readData );
	if( err )
		return err;

	UTL_Memdump("Read data", (char*)readData, readLen, 1);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_I2CXfer
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_I2CXfer()
{
	int	num, n, size;
	u_int8	length;
	char	str[STR_XFER_LEN];
	int32	err=1;
	SMB_I2CMESSAGE *msg=NULL;

	printf("SMB2CTRL_I2CXfer:\n");
	
	printf(" number of I2C messages to transfer -> ");
	fflush( stdin );
	IGNORE_RET_VAL(scanf("%d", &num));

	if ( (num <=0 ) || (num > 1024) ) {
		printf("*** # of messages too high, max. is 1024.\n");
		return -1;
	}

	/* alloc buffer (data buffer limited to 128 bytes) */
	size = num * (sizeof(SMB_I2CMESSAGE) +	I2C_BLOCK_MAX_BYTES);
	if( (msg = (SMB_I2CMESSAGE*)malloc( size )) == NULL)
		goto ERR_EXIT;

	/* create the messages */
	for( n=0; n<num; n++ ){

		printf("--- Message #%d ---\n", n);

		AskUser( (unsigned int*)&msg[n].flags, (unsigned int*)&msg[n].addr, 0, 0, 0, 0 );

		/* set buffer pointer behind the SMB_I2CMESSAGE struct */
		msg[n].buf = (u_int8*)(&msg[n]) + sizeof(SMB_I2CMESSAGE);

		length = I2C_BLOCK_MAX_BYTES; 
		AskUserBlk( &length, msg[n].buf );

		msg[n].len = length;
	}

	err = SMB2API_I2CXfer( SMB2CTRL_smbHdl, msg, num );
	if( err )
		goto ERR_EXIT;

	/* dump messages */
	for( n=0; n<num; n++ ){
		snprintf( str, (size_t)STR_XFER_LEN ,"Message #%d", n);
		UTL_Memdump( str, (char*)(msg[n].buf), msg[n].len, 1);
	}

ERR_EXIT:
	if( msg )
		free(msg);

	return err;
}

/**********************************************************************/
/** SMB2CTRL_Errstring
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_Errstring()
{
	int		errCode;
    static char errMsg[512];

	printf("SMB2CTRL_Errstring:\n");

	printf(" error code -> 0x");
	fflush( stdin );
	IGNORE_RET_VAL(scanf("%x", &errCode));

	printf(" %s\n", SMB2API_Errstring( errCode, errMsg ));

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_Ident
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_Ident()
{
	printf("SMB2CTRL_Ident:\n");

	printf("%s\n", SMB2API_Ident() );

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_AlertResponse
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_AlertResponse()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int16		alertCnt;
	int32		err;

	printf("SMB2CTRL_AlertResponse:\n");
	AskUser( &flags, &addr, 0, 0, 0, 0 );

	err = SMB2API_AlertResponse( SMB2CTRL_smbHdl, flags, addr, &alertCnt );
	if( err )
		return err;

	printf(" received alerts: %d\n", alertCnt);

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_AlertCbInstall
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_AlertCbInstall()
{
	unsigned int 	addr=0;

	printf("SMB2CTRL_AlertCbInstall:\n");
	AskUser( 0, &addr, 0, 0, 0, 0 );
	printf(" alertCallCount=%d\n", (int)SMB2CTRL_alertCallCount);

	return SMB2API_AlertCbInstall( SMB2CTRL_smbHdl, addr, AlertCbFunc,
				(void*)(INT32_OR_64)addr );

	return 0;
}

/**********************************************************************/
/** SMB2CTRL_AlertCbInstallSig
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_AlertCbInstallSig()
{
	unsigned int	addr=0;
	unsigned int	sigCode	= 0;

	printf("SMB2CTRL_AlertCbInstallSig:\n");
	AskUser( 0, &addr, 0, 0, 0, 0 );

	printf(" UOS_SIG signal code to use for alert -> 0x");
	fflush( stdin );
	IGNORE_RET_VAL(scanf("%x", &sigCode));
	
	printf(" alertCallCount=%d\n", (int)SMB2CTRL_alertCallCount);

	return SMB2API_AlertCbInstallSig( SMB2CTRL_smbHdl, addr, AlertCbFunc,
									  (void*)(INT32_OR_64)addr, sigCode );
	return 0;
}

/**********************************************************************/
/** SMB2CTRL_AlertCbRemove
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_AlertCbRemove()
{
	unsigned int	addr=0;
	int		cbArg;
	int32		err;

	printf("SMB2CTRL_AlertCbRemove:\n");
	AskUser( 0, &addr, 0, 0, 0, 0 );

	err = SMB2API_AlertCbRemove( SMB2CTRL_smbHdl, addr, (void**)&cbArg );
	if( err )
		return err;

	printf(" cbArg=0x%x (should be the address)\n", cbArg);
	printf(" alertCallCount=%d\n", (int)SMB2CTRL_alertCallCount);

	return 0;
}

/**********************************************************************/
/** AlertCbFunc
 *
 *  \return 0=ok, or error number
 */
static void AlertCbFunc( void *cbArg )
{
	SMB2CTRL_alertCallCount++;
	printf(">>> AlertCbFunc called with cbArg=%p, alertCallCount=%d\n", cbArg, (int)SMB2CTRL_alertCallCount );
}

/**********************************************************************/
/** Simple memory test
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_Mtest()
{
	unsigned int	flags=0;
	unsigned int	addr=0;
	u_int8		cmdAddrStart=0, cmdAddr=0;
	int32		err=0;
	u_int8		bytes, words, n;
	u_int8		bytePat, byteRead;
	u_int16		wordPat, wordRead;
	u_int32		errCount;
	int		tmp=0;
	char		errMsg[512];

	printf("Simple memory test:\n");

	AskUser( &flags, &addr, 0, 0, 0, 0 );

	printf(" offset to begin (index value, 0x00..0xff) -> 0x");
	fflush( stdin );
	IGNORE_RET_VAL(scanf("%x", &tmp));
	cmdAddrStart = (u_int8)tmp;

	printf(" number of bytes to transfer (0x00..0xff) -> 0x");
	fflush( stdin );
	IGNORE_RET_VAL(scanf("%x", &tmp));
	bytes = (u_int8)tmp;
	words = bytes/2;

	/*-----------------------------------------+
	|  Write/SMB2CTRL_ReadByteData test                 |
	+-----------------------------------------*/
	errCount=0;
	printf("\n - SMB2CTRL_WriteByteData( hdl, flags, 0x%x, 0x%x..0x%x, byte ):\n",
		addr, cmdAddrStart, cmdAddrStart+bytes);
	printf("   byte:");
	for( n=0; n<bytes; n++ ){
		bytePat = n+1;
		printf(" 0x%02x", bytePat);
		cmdAddr = cmdAddrStart + n;
		err = SMB2API_WriteByteData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, bytePat );
		if( err ){
			printf("\n\007 *** failed: SMB2CTRL_WriteByteData( hdl, flags, 0x%x, 0x%x, 0x%x )\n",
				addr, cmdAddr, bytePat);			
			printf(" %s\n", SMB2API_Errstring( err, errMsg ));
			errCount++;
			break;
		}
		UOS_Delay(20);
	}

	printf("\n - SMB2CTRL_ReadByteData( hdl, flags, 0x%x, 0x%x..0x%x, byte ), verify:\n",
		addr, cmdAddrStart, cmdAddrStart+bytes);
	for( n=0; n<bytes; n++ ){
		bytePat = n+1;
		cmdAddr = cmdAddrStart + n;
		err = SMB2API_ReadByteData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &byteRead );
		if( err ){
			printf(" *** SMB2CTRL_ReadByteData( hdl, flags, 0x%x, 0x%x, &byte ) failed\n",
				addr, cmdAddr);
			printf(" %s\n", SMB2API_Errstring( err, errMsg ));
			errCount++;
			break;
		}
		if( bytePat != byteRead ){
			printf(" *** index=0x%02x: written byte 0x%02x != read byte 0x%02x\n",
				cmdAddr, bytePat, byteRead);
			errCount++;
		}
	}

	if( errCount ){
		printf(" => *** Write/SMB2CTRL_ReadByteData test failed\n");
		SMB2CTRL_errCount++;
	}
	else {
		printf(" => Write/SMB2CTRL_ReadByteData test passed\n");
	}

	/*-----------------------------------------+
	|  Write/SMB2CTRL_ReadWordData test        |
	+-----------------------------------------*/
	errCount=0;
	printf("\n - SMB2CTRL_WriteWordData( hdl, flags, 0x%x, 0x%x..0x%x, word ):\n",
		addr, cmdAddrStart, cmdAddrStart+words);
	printf("   word:");
	for( n=0; n<words; n+=2 ){
		bytePat = n+1;
		wordPat = (0xff00 & (((u_int16)(bytePat))<<8)) |
				  (0x00ff & (u_int16)(~bytePat));
		printf(" 0x%04x", wordPat);
		cmdAddr = cmdAddrStart + n;
		err = SMB2API_WriteWordData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, wordPat );
		if( err ){
			printf("\n\007 *** failed: SMB2CTRL_WriteWordData( hdl, flags, 0x%x, 0x%x, 0x%x )\n",
				addr, cmdAddr, wordPat);
			printf(" %s\n", SMB2API_Errstring( err, errMsg ));
			errCount++;
			break;
		}
		UOS_Delay(20);
	}

	printf("\n - SMB2CTRL_ReadWordData( hdl, flags, 0x%x, 0x%x..0x%x, word ), verify:\n",
		addr, cmdAddrStart, cmdAddrStart+words);
	for( n=0; n<words; n+=2 ){
		bytePat = n+1;
		wordPat = (0xff00 & (((u_int16)(bytePat))<<8)) |
				  (0x00ff & (u_int16)(~bytePat));
		cmdAddr = cmdAddrStart + n;
		err = SMB2API_ReadWordData( SMB2CTRL_smbHdl, flags, addr, cmdAddr, &wordRead );
		if( err ){
			printf(" *** SMB2CTRL_ReadWordData( hdl, flags, 0x%x, 0x%x, &word ) failed\n",
				addr, cmdAddr);
			printf(" %s\n", SMB2API_Errstring( err, errMsg ));
			errCount++;
			break;
		}
		if( wordPat != wordRead ){
			printf(" *** index=0x%02x: written word 0x%04x != read word 0x%04x\n",
				cmdAddr, wordPat, wordRead);
			errCount++;
		}
	}

	if( errCount ){
		printf(" => *** Write/SMB2CTRL_ReadWordData test failed\n");
		SMB2CTRL_errCount++;
	}
	else {
		printf(" => Write/SMB2CTRL_ReadWordData test passed\n");
	}

	return -1;
}

/**********************************************************************/
/** SMB2CTRL_List all available SMB devices
 *
 *  \return 0=ok, or error number
 */
extern int32 SMB2CTRL_List()
{
	u_int16 addr;
	u_int8  byte;
	int32   err;

	printf("SMB2CTRL_List all available SMB devices:\n");

	for( addr=0x0; addr<0xff; addr+=2 ) {
		err = SMB2API_ReadByte( SMB2CTRL_smbHdl, 0, addr, &byte );
		/* device present? */ 
		if( (err != SMB_ERR_ADDR) && (err != SMB_ERR_NO_DEVICE) ) {
			printf("- 0x%02x", addr);
			switch( err ){
				case SMB_ERR_NO:
					printf(" (read-byte=0x%x)\n", byte);
					break;
				case SMB_ERR_ADDR_EXCLUDED:
					printf(" (device not checked - excluded in descriptor)\n");
					break;
				default:
				{
					static char errMsg[512];
					printf(" %s\n", SMB2API_Errstring( err, errMsg ));
				}
			} /* switch */
		}
	}/* for */

	return -1;
}

/* dummy to satisfy compiler, unused */
static int internFunc( int argc, char **argv )
{
	return 0;
}

