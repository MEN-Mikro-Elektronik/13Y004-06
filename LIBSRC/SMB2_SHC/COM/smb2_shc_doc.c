/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_shc_doc.c
 *
 *      \author  andreas.werner@men.de
 *
 *      \brief   User documentation for SMB2_SHC (Shelf Controller API)
 *
 *     Required: -
 *
 *     \switches -
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2014-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*! \mainpage

  This document describes the SMB2SHC API to access the MEN Shelf Controller.
\n
  The Shelf Controller also called SHC can be controlled using the I2C/SMBus\n
  interface.\n
  The SMB2SHC API uses the SMBus interface from the SMB2_API which\n
  provides e.g. ReadByteData and ReadBlockData to access the SMBus.\n

\n
  The main features of the Shelf controller API are:\n
		- Get the power supply status SMB2SHC_GetPSU_State()
		- Get the system temperature  SMB2SHC_GetTemperature()
		- Set the ambient temperature SMB2SHC_SetTemperature()
		- Get the fan status          SMB2SHC_GetFAN_State()
		- Get the voltage levels      SMB2SHC_GetVoltLevel()
		- Get the UPS charging state  SMB2SHC_GetUPS_State()
		- Get the configuration data  SMB2SHC_GetConf_Data()
		- ...

\n
  <b>Calling example of a SMB2SHC function:</b>
  \verbatim
  err = SMB2SHC_GetTemperature(u_int16 *value); \endverbatim

\n

*/
/** \example smb2_shc_ctrl.c */
/*! \page smb2shcdummy MEN logo

 \menimages

*/

