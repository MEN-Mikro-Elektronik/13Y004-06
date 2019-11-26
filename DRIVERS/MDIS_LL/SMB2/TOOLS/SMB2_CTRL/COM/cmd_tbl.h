/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  cmd_tbl.h
 *
 *      \author  dieter.pfeuffer@men.de
 *
 *        \brief Command table for dispatching
 *
 *     Switches: -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright 2019, MEN Mikro Elektronik GmbH
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




