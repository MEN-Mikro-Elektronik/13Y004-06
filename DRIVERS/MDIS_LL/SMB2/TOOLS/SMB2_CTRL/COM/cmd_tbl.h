/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  cmd_tbl.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/06/22 11:59:20 $
 *    $Revision: 1.4 $
 *
 *        \brief Command table for dispatching
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: cmd_tbl.h,v $
 * Revision 1.4  2009/06/22 11:59:20  dpfeuffer
 * R: MDVE warnings
 * M: added prefix SMB2CTRL to external globals and functions
 *
 * Revision 1.3  2006/05/31 08:22:17  DPfeuffer
 * added commands -id, -list
 *
 * Revision 1.2  2006/03/03 10:52:40  DPfeuffer
 * - fixes for VxWorks compiler
 *
 * Revision 1.1  2006/02/28 15:57:35  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright by MEN Mikro Elektronik GmbH, Nuremberg, Germany 
 ****************************************************************************/

#ifndef _CMDTBL_H
#define _CMDTBL_H

#ifdef __cplusplus
      extern "C" {
#endif

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* command table for dispatching */
struct _COMMAND_TABLE
{
    char* command;
    int32 (*dispFunc)(void);
} SMB2CTRL_commands[] =
{   /* command      function */
	{"h",			SMB2CTRL_CmdHelp},
	/* SMB2API */
	{"q",			SMB2CTRL_QuickComm},
	{"wb",			SMB2CTRL_WriteByte},
	{"rb",			SMB2CTRL_ReadByte},
	{"wbd",			SMB2CTRL_WriteByteData},
	{"rbd",			SMB2CTRL_ReadByteData},
	{"wwd",			SMB2CTRL_WriteWordData},
	{"rwd",			SMB2CTRL_ReadWordData},
	{"wbk",			SMB2CTRL_WriteBlockData},
	{"rbk",			SMB2CTRL_ReadBlockData},
	{"pc",			SMB2CTRL_ProcessCall},
	{"bpc",			SMB2CTRL_BlockProcessCall},
	{"ar",			SMB2CTRL_AlertResponse},
	{"aci",			SMB2CTRL_AlertCbInstall},
	{"acis",		SMB2CTRL_AlertCbInstallSig},
	{"acr",			SMB2CTRL_AlertCbRemove},
	{"i2c",			SMB2CTRL_I2CXfer},
	{"ers",			SMB2CTRL_Errstring},
	{"id",			SMB2CTRL_Ident},
	{"list",		SMB2CTRL_List},
	{"mt",			SMB2CTRL_Mtest},
};

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/* number of commands */
#define NUMBER_OF_COMMANDS\
    sizeof(SMB2CTRL_commands)/sizeof(struct _COMMAND_TABLE)

#ifdef __cplusplus
   }
#endif

#endif /* _CMDTBL_H */




