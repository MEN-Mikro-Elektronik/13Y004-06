/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: smb2_fslimx6.c
 *      Project: CC10
 *
 *       Author: wz
 *        $Date: 2016/01/07 13:29:36 $
 *    $Revision: 1.0 $
 *
 *  Description: system managment bus driver for the fslimx6 I2C controller
 *
 * This code exports one init functions:
 * - SMB_fslimx6_Init - for Freescale i.MX6 I2C controller
 *
 *     Required: -
 *     Switches: SMB_FIXED_HANDLE - don't allocate handle dynamically
 *									instead, use handle from caller
 *
 *-------------------------------[ History ]---------------------------------
 *
 *
 * Revision 1.0  2016/01/07 19:13:41  wzhou
 * Initial Revision
 *
 *
 * cloned from SMB2/COM/smb2_mgt5200.c Rev. 1.10
 *---------------------------------------------------------------------------
 * (c) Copyright 2006..2016 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

/* #define DBG */
 
#include <MEN/men_typs.h>
#include <MEN/oss.h>
#include <MEN/dbg.h>
#include <MEN/maccess.h>
#include <MEN/mdis_err.h>

#define SMB_COMPILE
#include <MEN/smb2.h>

/*-----------------------------------------+
|  TYPEDEFS                                |
+------------------------------------------*/
typedef struct
{
	SMB_ENTRIES		entries;		/**< function entries */

	SMB_COM_HANDLE	smbComHdl;		/**< Common handle entries */

	/* data */
	u_int32			ownSize;		/**< size of memory allocated for this handle */
	u_int32			timeOut;		/**< wait time if SMBus is busy */
	u_int32			mikroDelay;		/**< default 0-OSS_Delay, 1-OSS_MikroDelay */
	VXB_DEVICE_ID	busCtrlId;		/**< VxBus controller ID for I2C bus */
}SMB_HANDLE;

/*-----------------------------------------+
|  DEFINES & CONST                         |
+------------------------------------------*/
#define	DBG_MYLEVEL		smbHdl->smbComHdl.dbgLevel
#define DBH				smbHdl->smbComHdl.dbgHdl
#define OSSH			smbHdl->smbComHdl.osHdl

int _dbgRd( SMB_HANDLE *smbHdl, MACCESS base, u_int32 reg, u_int32 val, int line )
{
	DBGWRT_1((DBH,"I2C_REG_RD @%08p+0x%08x %08x line %d\n", (void*)base, reg, val, line ));
	return( val );
}

void _dbgWr( SMB_HANDLE *smbHdl, MACCESS base, u_int32 reg, u_int32 val, int line )
{
	DBGWRT_1((DBH,"I2C_REG_WR >> @%08p+0x%08x %08x line %d\n", (void*)base, reg, val, line ));
}

/*-----------------------------------------+
|  GLOBALS                                 |
+------------------------------------------*/
/*-----------------------------------------+
|  STATICS                                 |
+------------------------------------------*/

/*-----------------------------------------+
|  PROTOTYPES                              |
+------------------------------------------*/

static char* fslimx6_smbIdent( void );
static u_int32 fslimx6_smbExit( SMB_HANDLE  **smbHdlP );
static int32 fslimx6_i2cXfer( SMB_HANDLE *smbHdl, struct I2CMESSAGE msg[], u_int32 num );

static void delay( SMB_HANDLE  *smbHdl )
{
	if( smbHdl->mikroDelay )
		OSS_MikroDelay( OSSH, 1 );
	else
		OSS_Delay( OSSH, 1 );

}/*delay*/


/* Write Len bytes pointed to by *Data to the device with address
   Addr.
*/
static int fslimx6_i2cWrite
(
	SMB_HANDLE  *smbHdl,
	int Addr,
	u_int8 * Data,
	int Len,
	int Restart
)
{
	int error = SMB_ERR_NO;
	
	DBGWRT_1((DBH,"%s addr %02x len %d restart %d\n", __FUNCTION__, Addr, Len, Restart ));

	if( !Restart )
		fslI2cStart(smbHdl->busCtrlId);
	else 
		fslI2cRestart(smbHdl->busCtrlId);

	/* Output Address (R/W = 0) */
	fslI2cByteWrite(smbHdl->busCtrlId, (u_int8)Addr);

	while (Len--)
	{
		fslI2cByteWrite(smbHdl->busCtrlId, *Data++);
	}/*while*/

CLEANUP:
	return error;
}/* fslimx6_i2cWrite */


/* Read Len bytes to the location pointed to by *Data from the device
   with address Addr.
*/
static int fslimx6_i2cRead
(
	SMB_HANDLE  *smbHdl,
	int Addr,
	u_int8 * Data,
	int Len,
	int Restart
)
{
	int error = SMB_ERR_NO;
	volatile u_int32 dummy;

	DBGWRT_1((DBH,"%s addr %02x len %d restart %d\n", __FUNCTION__, Addr, Len, Restart ));

	if( !Restart )
		fslI2cStart(smbHdl->busCtrlId);
	else 
		fslI2cRestart(smbHdl->busCtrlId);

	/* Output Address (R/W = 1) */
	fslI2cByteWrite(smbHdl->busCtrlId, (u_int8)(Addr | 0x01));

	while (Len)
	{
		fslI2cByteRead(smbHdl->busCtrlId, Data++);
		Len--;					/* Decrement count */
	}/*while*/
	
CLEANUP:
	return error;
}/* fslimx6_i2cRead */


/*******************************  fslimx6_smbIdent  *************************
 *
 *  Description:  Return ident string
 *
 *---------------------------------------------------------------------------
 *  Input......:  -
 *  Output.....:  return  pointer to ident string
 *  Globals....:  -
 ****************************************************************************/
static char *fslimx6_smbIdent( void )	/* nodoc */
{
    return( "SMB - SMB library: smb2_fslimx6.c Revision 1.0  2016/01/07" );
}/* fslimx6_smbIdent */


#ifdef	SMB_FIXED_HANDLE
u_int32 SMB_fslimx6_HandleSize(void)
{
	return sizeof( SMB_HANDLE );
}
u_int32 SMB_MPC85XX_HandleSize(void)
{
	return sizeof( SMB_HANDLE );
}
#endif


/******************************** fslimx6_smbUseOssDelay *****************************/
/** Setup used OSS_Delay or OSS_MikroDelay.
 *
 *  \param   smbHdl     	valid SMB handle
 *	\param   useOssDelay    0 | 1
 *
 *  \return    0 | error code
 *
 *  Globals....:  -
 ****************************************************************************/
static int32 fslimx6_smbUseOssDelay( SMB_HANDLE *smbHdl, int useOssDelay )
{
	smbHdl->mikroDelay = !useOssDelay;
    return( SMB_ERR_NO );
}


/****************************** SMB_fslimx6_Init *****************************
 *
 *  Description:  Initializes the Freescale I2C controller lib
 *
 *  Note: I2C pins must have been assigned by caller
 *
 *---------------------------------------------------------------------------
 *  Input......:  desc		descriptor
 *                osHdl     OS specific handle
 *                smbHdlP	pointer to variable where the handle will be stored
 *
 *  Output.....:  return    0 | error code
 *				  *smbHdlP	valid handle | NULL
 *
 *  Globals....:  -
 ****************************************************************************/
u_int32 SMB_FSLIMX6_Init
(
	SMB_DESC_FSLIMX6	*desc,
    OSS_HANDLE			*osHdl,
	void				**smbHdlP
)
{
    u_int32     error  = SMB_ERR_NO;
    SMB_HANDLE  *smbHdl;
    u_int32		gotSize=0;

#ifndef	SMB_FIXED_HANDLE
	*smbHdlP = NULL;
	smbHdl   = (SMB_HANDLE*) OSS_MemGet( osHdl, sizeof(SMB_HANDLE), &gotSize );
	if( smbHdl == NULL )
	{
		error = SMB_ERR_NO_MEM;
		goto CLEANUP;
	}/*if*/
	*smbHdlP = smbHdl;

#else
    smbHdl = *smbHdlP;
#endif

    /* init the structure */
    OSS_MemFill( osHdl, gotSize, (char*)smbHdl, 0 );

    DBGINIT((NULL,&smbHdl->smbComHdl.dbgHdl));
    
    smbHdl->busCtrlId 		= vxbInstByNameFind("fslI2c", desc->unit);
    smbHdl->smbComHdl.dbgLevel		= desc->dbgLevel;
    smbHdl->timeOut		  	= desc->timeOut;
    smbHdl->mikroDelay		= desc->mikroDelay;
    smbHdl->ownSize  		= gotSize;
    smbHdl->smbComHdl.osHdl 	= (OSS_HANDLE*) osHdl;
	smbHdl->smbComHdl.busyWait 	= smbHdl->timeOut;
	smbHdl->entries.Capability = SMB_FUNC_I2C | SMB_FUNC_SMBUS_EMUL ;

	DBGWRT_1((DBH,"%s baseAddr %08p smbHdl %08p\n", __FUNCTION__, smbHdl->baseAddr, smbHdl ));
	if( (error = SMB_COM_Init(smbHdl)) )
	{
		goto CLEANUP;
	}

	smbHdl->entries.Exit		= (int32 (*)(void**))fslimx6_smbExit;
	smbHdl->entries.SmbXfer		= NULL;
	smbHdl->entries.I2CXfer		= (int32 (*)( void *smbHdl, struct I2CMESSAGE msg[],
											  u_int32 num))fslimx6_i2cXfer;
	smbHdl->entries.Ident		= (char* (*)(void))fslimx6_smbIdent;
	smbHdl->entries.UseOssDelay	= (int32 (*)(void *,int))fslimx6_smbUseOssDelay;

#ifndef	SMB_FIXED_HANDLE
  CLEANUP:
#endif

	DBGWRT_1((DBH,"SMB_fslimx6_Init exit (error=0x%08x)\n", error));

    return( error );
}/*SMB_fslimx6_Init*/


/*******************************  fslimx6_smbExit ***************************
 *
 *  Description:  Deinitializes this library and SMB controller.
 *
 *---------------------------------------------------------------------------
 *  Input......:  smbHdlP	pointer to variable where the handle is stored
 *
 *  Output.....:  return    always 0
 *				  *smbHdlP	NULL
 *
 *  Globals....:  -
 ****************************************************************************/
static u_int32 fslimx6_smbExit
(
	SMB_HANDLE **smbHdlP
)
{
    u_int32 error  = 0;
    SMB_HANDLE  *smbHdl;

    smbHdl = *smbHdlP;

    fslI2cStop(smbHdl->busCtrlId);
    fslI2cRelease(smbHdl->busCtrlId);
    
	/* deinitialize common interface */
	if(smbHdl->smbComHdl.ExitCom)
		smbHdl->smbComHdl.ExitCom(smbHdl);

#ifndef	SMB_FIXED_HANDLE
    OSS_MemFree( OSSH, smbHdl, smbHdl->ownSize );
    *smbHdlP = NULL;
#endif

    return( error );
}/* fslimx6_smbExit */


/******************************** fslimx6_i2cXfer *****************************/
/** Read/Write data from a device using the I2C protocol
 *
 *  \param   smbHdl     valid SMB handle
 *	\param   msg[]      array of i2c messages to handle
 *	\param   num        number of messages in msg []
 *
 *  \return    0 | error code
 *
 *  Globals....:  -
 ****************************************************************************/
static int32 fslimx6_i2cXfer
(
	SMB_HANDLE *smbHdl,
	struct I2CMESSAGE msg[],
	u_int32 num
)
{
	struct I2CMESSAGE *pmsg;
	u_int32 i;
	int32 error = SMB_ERR_NO;

	DBGWRT_1((DBH,">>> %s: smbHdl %08p\n", __FUNCTION__, smbHdl ));
	
	for( i = 0; i < num; i++ ) {
		pmsg = &msg[i];
		DBGWRT_3((DBH,"Doing %s %d bytes to 0x%02x - %d of %d messages\n",
				 pmsg->flags & I2C_M_RD ? "read" : "write",
				 pmsg->len, pmsg->addr, i + 1, num));

		if( pmsg->flags & I2C_M_RD )
		{		
			error = fslimx6_i2cRead(smbHdl, pmsg->addr, pmsg->buf, pmsg->len, i);
			if( error )
			{
				DBGWRT_ERR((DBH,"*** %s: fslimx6_i2cRead\n", __FUNCTION__ ));
				break;
			}
		}
		else
		{
			error = fslimx6_i2cWrite(smbHdl, pmsg->addr, pmsg->buf, pmsg->len, i);
			if( error )
			{
				DBGWRT_ERR((DBH,"*** %s: fslimx6_i2cWrite\n", __FUNCTION__ ));
				break;
			}
		}
	}

	/* STOP */
	fslI2cStop(smbHdl->busCtrlId);
	fslI2cRelease(smbHdl->busCtrlId);
	
CLEANUP:
	DBGWRT_1((DBH,"<<< %s: smbHdl %08p error %08x\n", __FUNCTION__, smbHdl, error ));
	return error;
} /* fslimx6_i2cXfer */

