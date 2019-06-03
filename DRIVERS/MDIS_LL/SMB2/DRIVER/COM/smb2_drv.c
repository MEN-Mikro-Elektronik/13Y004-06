/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_drv.c
 *
 *      \author  dieter.pfeuffer@men.de
 *
 *      \brief   Generic low-level driver for devices on SMBus
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
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

#define _NO_LL_HANDLE		/* ll_defs.h: don't define LL_HANDLE struct */

#ifdef SMB2_SW				/* swapped variant */
#	define MAC_MEM_MAPPED
#	define ID_SW
#endif

#include <MEN/men_typs.h>   /* system dependent definitions */
#include <MEN/maccess.h>    /* hw access macros and types */
#include <MEN/dbg.h>        /* debug functions */
#include <MEN/oss.h>        /* oss functions */
#include <MEN/desc.h>       /* descriptor functions */
#include <MEN/mdis_api.h>   /* MDIS global defs */
#include <MEN/mdis_com.h>   /* MDIS common defs */
#include <MEN/mdis_err.h>   /* MDIS error codes */
#include <MEN/ll_defs.h>    /* low-level driver definitions */
#include <MEN/smb2.h>		/* SMB2 definitions */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* general defines */
#define CH_NUMBER			1			/**< Number of device channels */
#define CH_BYTES			2			/**< Number of bytes per channel */
#define USE_IRQ				FALSE		/**< Interrupt required  */
#define ADDRSPACE_COUNT		0			/**< Number of required address spaces */

/* debug defines */
#define DBG_MYLEVEL			llHdl->dbgLevel   /**< Debug level */
#define DBH					llHdl->dbgHdl     /**< Debug handle */

#define MAX_ONLY_DEVS		16
#define MAX_EXCL_DEVS		16

#define TRANSFER( trx )													\
	trx = (SMB2_TRANSFER*)blk->data;									\
	DBGWRT_2((DBH, " code=0x%x, flags=0x%x, addr=0x%x, cmdAddr=0x%x, "	\
		"readWrite=0x%x, byteData/wordData/alertCnt=0x%x\n",			\
		code, trx->flags, trx->addr, trx->cmdAddr,						\
		trx->readWrite, trx->u.wordData));

#define TRANSFER_BLK( trxBlk )											\
	trxBlk = (SMB2_TRANSFER_BLOCK*)blk->data;							\
	DBGWRT_2((DBH, " code=0x%x, flags=0x%x, addr=0x%x, cmdAddr=0x%x, "	\
		"length/writeLen=0x%x, readLen=0x%x, data[0]=0x%x\n",			\
		code, trxBlk->flags, trxBlk->addr, trxBlk->cmdAddr,				\
		trxBlk->u.length, trxBlk->readLen, trxBlk->data[0]));

#define ALERT( trx )												\
	alert = (SMB2_ALERT*)blk->data;									\
	DBGWRT_2((DBH, " code=0x%x, addr=0x%x, sigCode=0x%x\n",			\
		code, alert->addr, alert->sigCode));

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
    int32           memAlloc;		/**< Size allocated for the handle */
    OSS_HANDLE      *osHdl;         /**< OSS handle */
    OSS_IRQ_HANDLE  *irqHdl;        /**< IRQ handle */
    DESC_HANDLE     *descHdl;       /**< DESC handle */
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/**< ID function table */
	/* debug */
    u_int32         dbgLevel;		/**< Debug level */
	DBG_HANDLE      *dbgHdl;        /**< Debug handle */
	/* smb2 specific */
	SMB_HANDLE		*smbH;			/**< ptr to SMB_HANDLE struct */
	u_int32			onlyDevsNbr;				/**< >0: number of SMB devs in onlyDevs[] */
	u_int8			onlyDevs[MAX_ONLY_DEVS];	/**< array of allowed SMB device numbers */
	u_int32			exclDevsNbr;				/**< >0: number of SMB devs in exclDevs[] */
	u_int8			exclDevs[MAX_EXCL_DEVS];	/**< array of excluded SMB device numbers */
} LL_HANDLE;

/** Double linked List for alerts */
typedef struct
{
	u_int16			addr;						/**< SMBus address */
	u_int32			sigCode;					/**< UOS_SIG_XXX code */
	OSS_SIG_HANDLE  *sigHdl;					/**< signal handle	*/
	LL_HANDLE		*llHdl;						/**< low-level handle */
	u_int32			gotsize;					/**< memory allocated for this node */
}ALERT_NODE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>   /* low-level driver jump table  */
#include <MEN/smb2_drv.h>	/* SMB2 driver header file */

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 SMB2_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
					   MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
					   OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 SMB2_Exit(LL_HANDLE **llHdlP );
static int32 SMB2_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 SMB2_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 SMB2_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code, INT32_OR_64 value32_or_64);
static int32 SMB2_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code, INT32_OR_64 *value32_or_64P);
static int32 SMB2_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 SMB2_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							 int32 *nbrWrBytesP);
static int32 SMB2_Irq(LL_HANDLE *llHdl );
static int32 SMB2_Info(int32 infoType, ... );

static char* Ident( void );
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);

/* SMB2 specific helper functions */
static int32 Smb2SetStat(LL_HANDLE *llHdl, int32 code, INT32_OR_64 value32_or_64);
static int32 Smb2GetStat(LL_HANDLE *llHdl, int32 code, INT32_OR_64 *value32_or_64P);
static void Smb2AlertCb( void *cbArg );
static int32 IsDevExcluded( LL_HANDLE *llHdl, u_int16 addr );


/****************************** SMB2_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT Pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
    void LL_GetEntry( LL_ENTRY* drvP )
#else
    void SMB2_GetEntry( LL_ENTRY* drvP )
#endif /* _ONE_NAMESPACE_PER_DRIVER_ */
{
    drvP->init        = SMB2_Init;
    drvP->exit        = SMB2_Exit;
    drvP->read        = SMB2_Read;
    drvP->write       = SMB2_Write;
    drvP->blockRead   = SMB2_BlockRead;
    drvP->blockWrite  = SMB2_BlockWrite;
    drvP->setStat     = SMB2_SetStat;
    drvP->getStat     = SMB2_GetStat;
    drvP->irq         = SMB2_Irq;
    drvP->info        = SMB2_Info;
}

/******************************** SMB2_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes the SMB2 device with the definitions made
 * in the descriptor.
 *
 * The function decodes \ref descriptor_entries "these descriptor entries"
 * in addition to the general descriptor keys.
 *
 *  \param descP      \IN  Pointer to descriptor data
 *  \param osHdl      \IN  OSS handle
 *  \param ma         \IN  HW access handle
 *  \param devSemHdl  \IN  Device semaphore handle
 *  \param irqHdl     \IN  IRQ handle
 *  \param llHdlP     \OUT Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 SMB2_Init(
    DESC_SPEC       *descP,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE       **llHdlP
)
{
    LL_HANDLE	*llHdl = NULL;
    u_int32		gotsize, smbBusNbr;
    int32		error;
    u_int32		value;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
    if((llHdl = (LL_HANDLE*)OSS_MemGet(
    				osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
    llHdl->memAlloc   = gotsize;
    llHdl->osHdl      = osHdl;
    llHdl->irqHdl     = irqHdl;

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    if((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return( Cleanup(llHdl,error) );

    /* DEBUG_LEVEL_DESC */
    if((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    if((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    DBGWRT_1((DBH, "LL - SMB2_Init\n"));

    /* SMB_BUSNBR (required) */
    if((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&smbBusNbr, "SMB_BUSNBR")))
		return( Cleanup(llHdl,error) );

    /* SMB_DEVS_ONLY (not possible with SMB_DEVS_EXCLUDE) */
	llHdl->onlyDevsNbr = MAX_ONLY_DEVS;
    if((error = DESC_GetBinary(llHdl->descHdl, (u_int8*)"", 0,
					llHdl->onlyDevs, &llHdl->onlyDevsNbr, "SMB_DEVS_ONLY")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* SMB_DEVS_EXCLUDE (not possible with SMB_DEVS_ONLY) */
	llHdl->exclDevsNbr = MAX_EXCL_DEVS;
    if((error = DESC_GetBinary(llHdl->descHdl, (u_int8*)"", 0,
					llHdl->exclDevs, &llHdl->exclDevsNbr, "SMB_DEVS_EXCLUDE")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	/* SMB_DEVS_ONLY and SMB_DEVS_EXCLUDE specified? */
	if( llHdl->onlyDevsNbr && llHdl->exclDevsNbr ){
		DBGWRT_ERR((DBH," *** LL - SMB2_Init: descriptor keys SMB_DEVS_EXCLUDE "
			"AND SMB_DEVS_ONLY specified\n"));
		return( Cleanup(llHdl,ERR_LL_DESC_PARAM) );
	}

    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
	if((error = OSS_GetSmbHdl( llHdl->osHdl, smbBusNbr, (void**)&llHdl->smbH) ))
		return( Cleanup(llHdl,error) );

	*llHdlP = llHdl;	/* set low-level driver handle */

	return(ERR_SUCCESS);
}

/****************************** SMB2_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes the SMB2 device.
 *
 *  \param llHdlP      \IN  Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 SMB2_Exit(
   LL_HANDLE    **llHdlP
)
{
    LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

    DBGWRT_1((DBH, "LL - SMB2_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/

    /*------------------------------+
    |  clean up memory               |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl,error);

	return(error);
}

/****************************** SMB2_Read ************************************/
/** Read a value from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param valueP     \OUT Read value
 *
 *  \return           \c 0 On success or error code
 */
static int32 SMB2_Read(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 *valueP
)
{
    DBGWRT_1((DBH, "LL - SMB2_Read: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** SMB2_Write ***********************************/
/** Write a value to the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param value      \IN  Read value
 *
 *  \return           \c ERR_LL_ILL_FUNC
 */
static int32 SMB2_Write(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 value
)
{
    DBGWRT_1((DBH, "LL - SMB2_Write: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** SMB2_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl  	     \IN  Low-level handle
 *  \param code          \IN  \ref getstat_setstat_codes "status code"
 *  \param ch            \IN  Current channel
 *  \param value32_or_64 \IN  Data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 On success or error code
 */
static int32 SMB2_SetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64 value32_or_64
)
{
	int32 error = ERR_SUCCESS;
	int32 value = (int32)value32_or_64;	/* 32bit value */

    DBGWRT_1((DBH, "LL - SMB2_SetStat: ch=%d code=0x%04x value=0x%x\n",
			  ch,code,value));

    switch(code) {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            llHdl->dbgLevel = value;
            break;
        /*--------------------------+
        |  channel direction        |
        +--------------------------*/
        case M_LL_CH_DIR:
			if( value != M_CH_IN )
				error = ERR_LL_ILL_DIR;
            break;
        /*--------------------------+
        |  SMB2_Xxx                 |
        +--------------------------*/
        default:
			error = Smb2SetStat( llHdl, code, value32_or_64 );
    }

	return(error);
}

/****************************** SMB2_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl          \IN  Low-level handle
 *  \param code           \IN  \ref getstat_setstat_codes "status code"
 *  \param ch             \IN  Current channel
 *  \param value32_or_64P \IN  Pointer to block data structure (M_SG_BLOCK) for
 *                             block status codes
 *  \param value32_or_64P \OUT Data pointer or pointer to block data structure
 *                             (M_SG_BLOCK) for block status codes
 *
 *  \return               \c 0 On success or error code
 */
static int32 SMB2_GetStat(
    LL_HANDLE *llHdl,
	int32  code,
    int32  ch,
    INT32_OR_64 *value32_or_64P
)
{
	int32 error = ERR_SUCCESS;
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value  */
	INT32_OR_64	*value64P = value32_or_64P;		/* stores 32/64bit pointer */

    DBGWRT_1((DBH, "LL - SMB2_GetStat: ch=%d code=0x%04x\n",
			  ch,code));

    switch(code)
    {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            *valueP = llHdl->dbgLevel;
            break;
        /*--------------------------+
        |  number of channels       |
        +--------------------------*/
        case M_LL_CH_NUMBER:
            *valueP = CH_NUMBER;
            break;
        /*--------------------------+
        |  channel direction        |
        +--------------------------*/
        case M_LL_CH_DIR:
            *valueP = M_CH_IN;
            break;
        /*--------------------------+
        |  channel length [bits]    |
        +--------------------------*/
        case M_LL_CH_LEN:
            *valueP = 32;
            break;
        /*--------------------------+
        |  channel type info        |
        +--------------------------*/
        case M_LL_CH_TYP:
            *valueP = M_CH_ANALOG;
            break;
        /*--------------------------+
        |   ident table pointer     |
        |   (treat as non-block!)   |
        +--------------------------*/
        case M_MK_BLK_REV_ID:
           *value64P = (INT32_OR_64)&llHdl->idFuncTbl;
           break;
        /*--------------------------+
        |  SMB2_Xxx                 |
        +--------------------------*/
        default:
			error = Smb2GetStat( llHdl, code, value32_or_64P );
    }

	return(error);
}

/******************************* SMB2_BlockRead ******************************/
/** Read a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl       \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrRdBytesP \OUT Number of read bytes
 *
 *  \return            \c 0 On success or error code
 */
static int32 SMB2_BlockRead(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrRdBytesP
)
{
    DBGWRT_1((DBH, "LL - SMB2_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}

/****************************** SMB2_BlockWrite ******************************/
/** Write a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrWrBytesP \OUT Number of written bytes
 *
 *  \return            \c ERR_LL_ILL_FUNC
 */
static int32 SMB2_BlockWrite(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
    DBGWRT_1((DBH, "LL - SMB2_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}


/****************************** SMB2_Irq ************************************/
/** Interrupt service routine - unused
 *
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \return LL_IRQ_DEVICE	IRQ caused by device
 *          LL_IRQ_DEV_NOT  IRQ not caused by device
 *          LL_IRQ_UNKNOWN  Unknown
 */
static int32 SMB2_Irq(
   LL_HANDLE *llHdl
)
{
	return(LL_IRQ_DEV_NOT);
}

/****************************** SMB2_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      Hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   Number of required address spaces
 *  LL_INFO_ADDRSPACE         Address space information
 *  LL_INFO_IRQ               Interrupt required
 *  LL_INFO_LOCKMODE          Process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE code returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType	   \IN  Info code
 *  \param ...         \IN  Argument(s)
 *
 *  \return            \c 0 On success or error code
 */
static int32 SMB2_Info(
   int32  infoType,
   ...
)
{
    int32   error = ERR_SUCCESS;
    va_list argptr;

    va_start(argptr, infoType );

    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes ORed)   |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			/* none address space required */
			error = ERR_LL_ILL_PARAM;
			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_CALL;
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
}

/*******************************  Ident  ***********************************/
/** Return ident string
 *
 *  \return            Pointer to ident string
 */
static char* Ident( void )
{
    return( (char*) IdentString );
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *	\warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param retCode    \IN  Return value
 *
 *  \return           \IN   retCode
 */
static int32 Cleanup(
   LL_HANDLE    *llHdl,
   int32        retCode
)
{
    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if(llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
}

/********************************* Smb2SetStat *******************************/
/** Handle SMB2_ setstat code
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl  	     \IN  Low-level handle
 *  \param code          \IN  \ref getstat_setstat_codes "status code"
 *  \param value32_or_64 \IN  Data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 On success or error code
 */
static int32 Smb2SetStat(
	LL_HANDLE	*llHdl,
	int32		code,
	INT32_OR_64 value32_or_64 )
{
	int32				error = SMB_ERR_NOT_SUPPORTED;
	SMB2_TRANSFER		*trx = NULL;
	SMB2_TRANSFER_BLOCK	*trxBlk = NULL;
	M_SG_BLOCK			*blk = (M_SG_BLOCK*)value32_or_64;		/* stores block struct pointer */

	switch(code){

	case SMB2_BLK_QUICK_COMM:
		TRANSFER( trx )
		DBGWRT_2((DBH, " QuickComm\n"));
		if( !llHdl->smbH->QuickComm )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->QuickComm( llHdl->smbH,
				trx->flags, trx->addr, trx->readWrite )) )
			goto ERR_EXIT;
		break;

	case SMB2_BLK_WRITE_BYTE:
		TRANSFER( trx )
		DBGWRT_2((DBH, " WriteByte\n"));
		if( !llHdl->smbH->WriteByte )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->WriteByte( llHdl->smbH,
			trx->flags, trx->addr, trx->u.byteData )) )
			goto ERR_EXIT;
		break;

	case SMB2_BLK_WRITE_BYTE_DATA:
		TRANSFER( trx )
		DBGWRT_2((DBH, " WriteByteData\n"));
		if( !llHdl->smbH->WriteByteData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->WriteByteData( llHdl->smbH,
			trx->flags, trx->addr, trx->cmdAddr, trx->u.byteData )) )
			goto ERR_EXIT;
		break;

	case SMB2_BLK_WRITE_WORD_DATA:
		TRANSFER( trx )
		DBGWRT_2((DBH, " WriteWordData\n"));
		if( !llHdl->smbH->WriteWordData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->WriteWordData( llHdl->smbH,
			trx->flags, trx->addr, trx->cmdAddr, trx->u.wordData )) )
			goto ERR_EXIT;
		break;

	case SMB2_BLK_WRITE_BLOCK_DATA:
		TRANSFER_BLK( trxBlk )
		DBGWRT_2((DBH, " WriteBlockData\n"));
		if( !llHdl->smbH->WriteBlockData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trxBlk->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->WriteBlockData( llHdl->smbH,
			trxBlk->flags, trxBlk->addr, trxBlk->cmdAddr,
			trxBlk->u.length, trxBlk->data )) )
			goto ERR_EXIT;
		break;

	case SMB2_BLK_ALERT_CB_INSTALL:
	{
		SMB2_ALERT	*alert = NULL;
		ALERT_NODE	*alertNode;
		u_int32 gotsize;

		ALERT( alert )
		DBGWRT_2((DBH, " AlertCbInstall\n"));
		if( !llHdl->smbH->AlertCbInstall )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, alert->addr )) )
			goto ERR_EXIT;

		/* create new alert node */
		if( (alertNode = (ALERT_NODE*)OSS_MemGet( llHdl->osHdl, sizeof(ALERT_NODE),
						&gotsize )) == NULL ){
			error = SMB_ERR_NO_MEM;
			goto ERR_EXIT;
		}

		/* init node */
		alertNode->addr = alert->addr;
		alertNode->sigCode = alert->sigCode;
		alertNode->gotsize = gotsize;
		alertNode->llHdl = llHdl;

		/* install signal */
		if ((error = (OSS_SigCreate(llHdl->osHdl, alert->sigCode,
						&alertNode->sigHdl)))){
			OSS_MemFree( llHdl->osHdl, (void*)alertNode, alertNode->gotsize );
			goto ERR_EXIT;
		}

		/* SMB2 alert install */
		if( (error = llHdl->smbH->AlertCbInstall( llHdl->smbH, alert->addr,
						Smb2AlertCb, (void*)alertNode )) ){
			OSS_SigRemove( llHdl->osHdl, &alertNode->sigHdl );
			OSS_MemFree( llHdl->osHdl, (void*)alertNode, alertNode->gotsize );
			goto ERR_EXIT;
		}
	}
		break;

	case SMB2_BLK_ALERT_CB_REMOVE:
	{
		SMB2_ALERT	*alert = NULL;
		ALERT_NODE	*alertNode;

		ALERT( alert )
		DBGWRT_2((DBH, " AlertCbRemove\n"));
		if( !llHdl->smbH->AlertCbRemove )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, alert->addr )) )
			goto ERR_EXIT;

		/* SMB2 alert remove */
		if( (error = llHdl->smbH->AlertCbRemove( llHdl->smbH, alert->addr,
						(void**)&alertNode )) ){
			goto ERR_EXIT;
		}

		/* remove signal */
		OSS_SigRemove(llHdl->osHdl, &alertNode->sigHdl);

		/* free node */
		OSS_MemFree( llHdl->osHdl, (void*)alertNode, alertNode->gotsize );
	}
		break;

	default:
		return ERR_LL_UNK_CODE;
	}

	return(0);

ERR_EXIT:
	return error;
}

/********************************* Smb2GetStat *******************************/
/** Handle SMB2_ getstat code
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl          \IN  Low-level handle
 *  \param code           \IN  \ref getstat_setstat_codes "status code"
 *  \param value32_or_64P \IN  Pointer to block data structure (M_SG_BLOCK) for
 *                             block status codes
 *  \param value32_or_64P \OUT Data pointer or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *
 *  \return           \c 0 On success or error code
 */
static int32 Smb2GetStat(
	LL_HANDLE	*llHdl,
	int32		code,
	INT32_OR_64 *value32_or_64P )
{
	int32 error = SMB_ERR_NOT_SUPPORTED;
	M_SG_BLOCK *blk = (M_SG_BLOCK*)value32_or_64P; /* stores block struct pointer */
	SMB2_TRANSFER *trx = NULL;
	SMB2_TRANSFER_BLOCK	*trxBlk = NULL;

	switch(code){

	case SMB2_BLK_READ_BYTE:
		TRANSFER( trx )
		if( !llHdl->smbH->ReadByte )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->ReadByte( llHdl->smbH,
				trx->flags, trx->addr, &trx->u.byteData )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " ReadByte: byteData=0x%02x\n", trx->u.byteData));
		break;

	case SMB2_BLK_READ_BYTE_DATA:
		TRANSFER( trx )
		if( !llHdl->smbH->ReadByteData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->ReadByteData( llHdl->smbH,
				trx->flags, trx->addr, trx->cmdAddr, &trx->u.byteData )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " ReadByteData: byteData=0x%02x\n", trx->u.byteData));
		break;

	case SMB2_BLK_READ_WORD_DATA:
		TRANSFER( trx )
		if( !llHdl->smbH->ReadWordData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->ReadWordData( llHdl->smbH,
				trx->flags, trx->addr, trx->cmdAddr, &trx->u.wordData )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " ReadWordData: wordData=0x%04x\n", trx->u.wordData));
		break;

	case SMB2_BLK_READ_BLOCK_DATA:
		TRANSFER_BLK( trxBlk )
		if( !llHdl->smbH->ReadBlockData )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trxBlk->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->ReadBlockData( llHdl->smbH,
				trxBlk->flags, trxBlk->addr, trxBlk->cmdAddr,
				&trxBlk->u.length, trxBlk->data )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " ReadBlockData: length=0x%02x, data[0]=0x%02x\n",
			trxBlk->u.length, trxBlk->data[0]));
		break;

	case SMB2_BLK_PROCESS_CALL:
		TRANSFER( trx )
		if( !llHdl->smbH->ProcessCall )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->ProcessCall( llHdl->smbH,
				trx->flags, trx->addr, trx->cmdAddr, &trx->u.wordData )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " ProcessCall: wordData=0x%04x\n", trx->u.wordData));
		break;

	case SMB2_BLK_BLOCK_PROCESS_CALL:
		TRANSFER_BLK( trxBlk )
		if( !llHdl->smbH->BlockProcessCall )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trxBlk->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->BlockProcessCall( llHdl->smbH,
				trxBlk->flags, trxBlk->addr, trxBlk->cmdAddr,
				trxBlk->u.writeLen, trxBlk->data,
				&trxBlk->readLen, trxBlk->data + trxBlk->u.writeLen)) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " BlockProcessCall: readLen=0x%02x, readData[0]=0x%02x\n",
			trxBlk->readLen, trxBlk->data + trxBlk->u.writeLen));
		break;

	case SMB2_BLK_ALERT_RESPONSE:
		TRANSFER( trx )
		if( !llHdl->smbH->AlertResponse )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, trx->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->AlertResponse( llHdl->smbH,
				trx->flags, trx->addr, &trx->u.alertCnt )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " AlertResponse: alertCnt=0x%x\n", trx->u.alertCnt));
		break;

	case SMB2_BLK_I2C_XFER:
	{
		SMB_I2CMESSAGE *i2cMsg = (SMB_I2CMESSAGE*)blk->data;
		DBGWRT_2((DBH, " code=0x%x, flags=0x%x, addr=0x%x, len=0x%x, buf[0]=0x%x\n",
			code, i2cMsg->flags, i2cMsg->addr, i2cMsg->len,	i2cMsg->buf[0]));
		if( !llHdl->smbH->I2CXfer )
			goto ERR_EXIT;
		if( (error = IsDevExcluded( llHdl, i2cMsg->addr )) )
			goto ERR_EXIT;
		if( (error = llHdl->smbH->I2CXfer( llHdl->smbH, i2cMsg, 1 )) )
			goto ERR_EXIT;
		DBGWRT_2((DBH, " I2cXfer: buf[0]=0x%x\n", i2cMsg->buf[0]));
		break;
	}

	default:
		return ERR_LL_UNK_CODE;
	}

	return(0);
ERR_EXIT:
	return error;
}

/********************************* Smb2AlertCb *******************************/
/** Alert callback function
 *
 *  Sends the installed signal for the occured alert.
 *
 *  \param cbArg      \IN  alert callback argument (ALERT_NODE)
 */
static void Smb2AlertCb( void *cbArg )
{
	ALERT_NODE	*alertNode = (ALERT_NODE*)cbArg;
	LL_HANDLE	*llHdl = alertNode->llHdl;

	DBGWRT_1((DBH, "LL - Smb2AlertCb: addr=0x%x sigCode=0x%x\n",
			  alertNode->addr, alertNode->sigCode));

	OSS_SigSend( llHdl->osHdl, alertNode->sigHdl );
}

/********************************* IsDevExcluded *******************************/
/** Check if SMB device is excluded
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param addr		  \IN  address of SMB device to check
 *
 *  \return           \c 0: not excluded
 */
static int32 IsDevExcluded(
	LL_HANDLE	*llHdl,
	u_int16		addr )
{
	u_int16 i;

	/* all devices allowed */
	if( (0 == llHdl->onlyDevsNbr) &&
		(0 == llHdl->exclDevsNbr) ){
		return 0;
	}
	/* only devices specified in SMB_DEVS_ONLY desc-key allowed */
	else if ( llHdl->onlyDevsNbr ){
		for( i=0; i<llHdl->onlyDevsNbr; i++ ){
			if( addr == llHdl->onlyDevs[i] )
				return 0;
		}
		return SMB_ERR_ADDR_EXCLUDED;
	}
	/* only devices not specified in SMB_DEVS_EXCLUDE desc-key allowed */
	else {
		/* SMB device in exclDevs array? */
		for( i=0; i<llHdl->exclDevsNbr; i++ ){
			if( addr == llHdl->exclDevs[i] )
				return SMB_ERR_ADDR_EXCLUDED;
		}
		return 0;
	}
}



