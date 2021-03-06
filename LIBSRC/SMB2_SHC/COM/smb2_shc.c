/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_shc.c
 *
 *      \author  quoc.bui@men.de
 *
 *       \brief  API functions to access the SMB2 Shelf Controller
 *
 *    \switches  -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright 2014-2019, MEN Mikro Elektronik GmbH
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
 * You should have received a copy of the GNU Lesser General License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <MEN/men_typs.h>
#include <MEN/smb2_shc.h>
#include <MEN/smb2_api.h>
#include <MEN/mdis_err.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* Access opcodes for SMB */
#define SHC_PSU_GET_OPCODE   0x01
#define SHC_TEMP_OPCODE      0x02
#define SHC_FAN_GET_OPCODE   0x03
#define SHC_VOLT_GET_OPCODE  0x04
#define SHC_UPS_GET_OPCODE   0x05
#define SHC_CONF_GET_OPCODE  0x06
#define SHC_TEMP_SET_OPCODE  0x07
#define SHC_PERS_PWRBTN_GET_OPCODE 0x08
#define SHC_PERS_PWRBTN_SET_OPCODE 0x09
#define SHC_SH_DOWN_OPCODE   0x10
#define SHC_PWR_OFF_OPCODE   0x11
#define SHC_PWRCYCLE_DUR_SET_OPCODE 0x12
#define SHC_FVER_GET_OPCODE  0x80

/* data lengths */
#define SHC_PSU_GET_LENGTH   0x03
#define SHC_TEMP_SET_LENGTH  0x03
#define SHC_DUR_SET_LENGTH  0x03
#define SHC_FAN_GET_LENGTH   0x09
#define SHC_VOLT_GET_LENGTH  0x08
#define SHC_UPS_GET_LENGTH   0x04
#define SHC_CONF_GET_LENGTH_v416  0x13
#define SHC_CONF_GET_LENGTH_v417  0x17
#define SHC_FVER_GET_LENGTH  0x07

#define BIT_IS_PRESENT       0x01
#define BIT_EX_PWR           0x02
#define BIT_PROV_PWR         0x02
#define BIT_ERR              0x04

#define BIT_FAN_IS_PRESENT   0x01
#define BIT_FAN_STAT         0x02

#define TO_FAN_STAT_BYTE(fan_id) (fan_id * 3)
#define TO_FAN_RPM_LSB(fan_nr)   (TO_FAN_STAT_BYTE(fan_nr) + 1)
#define TO_FAN_RPM_MSB(fan_nr)   (TO_FAN_STAT_BYTE(fan_nr) + 2)

#define SET_TEMP_ENABLE      0x01

#define SHC_SMBADDR          0xea
#define SHC_SMBFLAGS         0x00

/*-----------------------------------------+
|  GLOBALS                                 |
+-----------------------------------------*/
static void *g_SMB2SHC_smbHdl;
static struct shc_fwversion g_firm_version;

#define SHC_V416_OR_BELOW (g_firm_version.maj_revision <= 4 && g_firm_version.min_revision <= 16)
#define SHC_AT_LEAST_V417 (!SHC_V416_OR_BELOW)

/**
 * \defgroup _SMB2_SHC SMB2_SHC
 *  The SMB2_SHC API provides access functions to communicate
 *  with the Shelf Controller from user mode applications.
 *  The API uses the SMB2_API for the SMBUS access functions.
 *  @{
 */


/****************************************************************************/
/** Return ident string of the SMB2SHC API
 *
 *  \return    ident string
 *
 */
char* __MAPILIB SMB2SHC_Ident(void)
{
	return( (char*) IdentString );
}


/****************************************************************************/
/** Convert SMB2SHC, SMB2 and MDIS error codes to strings
*
* SMB2SHC_Errstring() creates an error message for error \a errCode and
* returns a pointer to the generated string with the following
* format:
*
*  \param errCode    \IN  error code from SMB2 function
*  \param strBuf     \OUT filled with error message (should have space for
*                         512 characters, including '\\0')
*  \return           \a   strBuf
*/
char* __MAPILIB SMB2SHC_Errstring(u_int32 errCode, char *strBuf)
{
	u_int32 i=0;
	char    *smbErr=NULL;

	/* SMB_SHC error table */
	static struct _ERR_STR
	{
		unsigned int errCode;
		char         *errString;
	} errStrTable[] =
	{
		/* max string size indicator  |1---------------------------------------------50| */
		/* no error */
		{ SMB2_SHC_ERR_NO			,		"(no error)" },
		{ SMB2_SHC_ID_NA			,		"ID number is not available" },
		{ SMB2_SHC_ERR_LENGTH			,		"Data has wrong length" },
		{ SMB2_SHC_ERR_FEATURE_UNAVAILABLE	,		"Feature is unavailable in this SHC revision" },
		/* max string size indicator  |1---------------------------------------------50| */
	};

	#define NBR_OF_ERR sizeof(errStrTable)/sizeof(struct _ERR_STR)

	/*----------------------+
	|  SMB2 SHC error?      |
	+----------------------*/
	if ((errCode >= (ERR_DEV + 0xf0)) && (errCode < ERR_END)) {

		/* search the error string */
		for (i=0; i<NBR_OF_ERR; i++) {
			if (errCode == (int32)errStrTable[i].errCode) {
				smbErr = errStrTable[i].errString;
			}
		}

		/* known SMB2_SHC error */
		if (smbErr) {
			sprintf(strBuf, "ERROR (SMB2_SHC) 0x%04x: %s", errCode, smbErr);
		}
		/* unknown SMB2_SHC error? */
		else {
			sprintf(strBuf, "ERROR (SMB2_SHC) 0x%04x: Unknown SMB2_SHC error", errCode);
		}
	}
	/*----------------------+
	|  MDIS or SMB2 error?  |
	+----------------------*/
	else {
		SMB2API_Errstring(errCode, strBuf);
	}

	return (strBuf);
}


/****************************************************************************/
/** Initialization of the shelf controller library
 *
 *  \param     deviceP    \IN  MDIS device name
 *  \return    0 on success or error code
 *
 *  \sa SMB2SHC_Exit
 */
int32 __MAPILIB SMB2SHC_Init(char *deviceP)
{
	int err = SMB2API_Init(deviceP, &g_SMB2SHC_smbHdl);
	if (err) {
		return err;
	}
	err = SMB2SHC_GetFirm_Ver(&g_firm_version);
	if (err) {
		return err;
	}
        return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** Deinitialization of the shelf controller library
 *
 *  \return     0 on success or error code
 *
 *  \sa SMB2SHC_Init
 */
int32 __MAPILIB SMB2SHC_Exit()
{
	if (g_SMB2SHC_smbHdl) {
		return (SMB2API_Exit(&g_SMB2SHC_smbHdl));
	}

	return (0);
}


/****************************************************************************/
/** Read the inner temperature of the shelf controller
 *
 *  The temperature will be read in Kelvin
 *
 *  \param     tempK    \OUT  Temperature (Kelvin) read from the device
 *  \return    0 on success or error code
 *
 *  \sa SMB2SHC_SetTemperature, SMB2SHC_GetTemperatureOverrideStatus
 */
int32 __MAPILIB SMB2SHC_GetTemperature(u_int16 *tempK)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_TEMP_SET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_TEMP_SET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	if (blkData[0] != SET_TEMP_ENABLE){
		return SMB2API_ReadWordData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
									SHC_TEMP_OPCODE, tempK);
	}
	else{
		*tempK = ((u_int16)blkData[2] << 8);    /* MSB */
		*tempK = *tempK | (u_int16)blkData[1];  /* LSB */
	}
	return SMB2_SHC_ERR_NO;
}

/****************************************************************************/
/** Get the status of the temperature override
 *
 *  Set status to 1 if override enabled, 0 otherwise.
 *
 *  \return    error code or SMB2_SHC_ERR_NO if no error
 *
 *  \sa SMB2SHC_GetTemperature, SMB2SHC_SetTemperature
 */
int32 __MAPILIB SMB2SHC_GetTemperatureOverrideStatus(u_int16 *status) {
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	int err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
		      SHC_TEMP_SET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_TEMP_SET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	*status = blkData[0] == SET_TEMP_ENABLE ? 1 : 0;
	return SMB2_SHC_ERR_NO;
}

/****************************************************************************/
/** Write the ambient temperature to the shelf controller (for one minute)
 *
 *  The temperature will be set in Kelvin
 *
 *  \param     tempK    \IN  Temperature (Kelvin) set to the device
 *  \return    0 on success or error code
 *
 *  \sa SMB2SHC_GetTemperatureOverrideStatus, SMB2SHC_GetTemperature
 */
int32 __MAPILIB SMB2SHC_SetTemperature(u_int16 tempK)
{
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	blkData[0] = (u_int8)SET_TEMP_ENABLE; 
	blkData[1] = (u_int8)tempK;      /* LSB */
	blkData[2] = (u_int8)(tempK>>8); /* MSB */
	
	return SMB2API_WriteBlockData( g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
									SHC_TEMP_SET_OPCODE, SHC_TEMP_SET_LENGTH, blkData);
}

/****************************************************************************/
/** Get the Power Supply Report of the selected PSU
 *
 *  \param     psu_nr     \IN   PSU ID number
 *  \param     shc_psu    \OUT  status of the selected PSU
 *  \return    0 on success or error code
 *
 */
int32 __MAPILIB SMB2SHC_GetPSU_State(enum SHC_PSU_NR psu_nr, struct shc_psu *shc_psu)
{
	int err;
	u_int8 length;
	u_int8 psu_data;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_PSU_GET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_PSU_GET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	if (psu_nr < SHC_PSU1 || psu_nr > SHC_PSU3)
		return SMB2_SHC_ID_NA;

	psu_data = blkData[psu_nr];
	shc_psu->intFailure    = psu_data & BIT_ERR;
	shc_psu->isPresent     = psu_data & BIT_IS_PRESENT;
	shc_psu->isEPwrPresent = psu_data & BIT_EX_PWR;

	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** Get the FAN status of the selected FAN.
 *
 *  \param     fan_nr     \IN   FAN ID number
 *  \param     shc_fan    \OUT  status of the selected FAN
 *  \return    0 on success or error code
 *
 */
int32 __MAPILIB SMB2SHC_GetFAN_State(enum SHC_FAN_NR fan_nr, struct shc_fan *shc_fan)
{
	int err;
	u_int8 length;
	u_int8 statByte, rpmLSB, rpmMSB;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_FAN_GET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_FAN_GET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	if (fan_nr < SHC_FAN1 || fan_nr > SHC_FAN3)
		return SMB2_SHC_ID_NA;

	statByte = TO_FAN_STAT_BYTE(fan_nr);
	rpmLSB = TO_FAN_RPM_LSB(fan_nr);
	rpmMSB = TO_FAN_RPM_MSB(fan_nr);

	shc_fan->isPresent = blkData[statByte] & BIT_FAN_IS_PRESENT;
	shc_fan->state     = blkData[statByte] & BIT_FAN_STAT;
	shc_fan->speedRpm  = (u_int16)blkData[rpmLSB] | ( (u_int16)blkData[rpmMSB] << 8);

	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** Get the voltage levels on power monitor inputs
*
*  \param     pwr_mon_nr    \IN   power monitor number
*  \param     volt_value    \OUT  voltage level
*  \return    0 on success or error code
*
*/
int32 __MAPILIB SMB2SHC_GetVoltLevel(enum SHC_PWR_MON_ID pwr_mon_nr, u_int16 *volt_value)
{
	int err;
	u_int8 length;
	u_int16 volt_data;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_VOLT_GET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_VOLT_GET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	if (pwr_mon_nr < SHC_PWR_MON_1 || pwr_mon_nr > SHC_PWR_MON_4)
		return SMB2_SHC_ID_NA;

	volt_data = blkData[pwr_mon_nr * 2];
	volt_data += ((u_int16)blkData[pwr_mon_nr * 2 + 1] << 8);
	*volt_value = volt_data;

	return SMB2_SHC_ERR_NO;
}

/****************************************************************************/
/** Set power cycle duration in milliseconds
 *
 *  \param     duration    \IN  desired delay in milliseconds
 *  \return    SMB2_SHC_ERR_NO on success or error code
 *
*/
int32 __MAPILIB SMB2SHC_SetPowerCycleDuration(u_int16 duration) {
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	if (SHC_V416_OR_BELOW) {
		return SMB2_SHC_ERR_FEATURE_UNAVAILABLE;
	}
	/* Set blkData[0] to 0 to only set powercycle duration.
	 * Set blkData[0] to 1 to enable immediate shutdown. */
	blkData[0] = 0;
	blkData[1] = (u_int8) duration;         /* LSB */
	blkData[2] = (u_int8) (duration >> 8);  /* MSB */

	return SMB2API_WriteBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
			SHC_PWRCYCLE_DUR_SET_OPCODE, SHC_DUR_SET_LENGTH, blkData);
}

/****************************************************************************/
/** Set the status of the persistent power button
 *
 *  \param     status	   \IN   1 to set button ON, 0 otherwise
 *  \return    SMB2_SHC_ERR_NO on success or error code
 *
 *  \sa SMB2SHC_GetPersistentPowerbuttonStatus
*/
int32 __MAPILIB SMB2SHC_SetPersistentPowerbuttonStatus(u_int32 status) {
	int err;
	if (SHC_V416_OR_BELOW) {
		return SMB2_SHC_ERR_FEATURE_UNAVAILABLE;
	}
	err = SMB2API_WriteByteData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
		SHC_PERS_PWRBTN_SET_OPCODE, status == 1 ? 1 : 0);
	if (err) {
		return err;
	}
	return SMB2_SHC_ERR_NO;
}

/****************************************************************************/
/** Get the status of the persistent power button
 *
 *  \param     status           \OUT  1 if button ON, 0 otherwise
 *  \return    SMB2_SHC_ERR_NO on success or error code
 *
 *  \sa SMB2SHC_SetPersistentPowerbuttonStatus
*/
int32 __MAPILIB SMB2SHC_GetPersistentPowerbuttonStatus(u_int8 *status) {
	int err;
	if (SHC_V416_OR_BELOW) {
		return SMB2_SHC_ERR_FEATURE_UNAVAILABLE;
	}
	err = SMB2API_ReadByteData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
		SHC_PERS_PWRBTN_GET_OPCODE, status);
	if (err) {
		return err;
	}
	return SMB2_SHC_ERR_NO;
}

/****************************************************************************/
/** Get the Uninterruptible Power Supply report of the selected UPS
*
*  \param     ups_nr           \IN   UPS ID number
*  \param     shc_ups_state    \OUT  status of the selected UPS
*  \return    0 on success or error code
*
*/
int32 __MAPILIB SMB2SHC_GetUPS_State(enum SHC_UPS_NR ups_nr, struct shc_ups *shc_ups_state)
{
	int err;
	u_int8 length;
	u_int8 ups_status, ups_charging_lvl;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_UPS_GET_OPCODE, &length, blkData);
	if (err)
		return err;

	if (length != SHC_UPS_GET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	if (ups_nr < SHC_UPS1 || ups_nr > SHC_UPS2)
		return SMB2_SHC_ID_NA;

	ups_status       = blkData[ups_nr * 2];
	ups_charging_lvl = blkData[ups_nr * 2 + 1];

	shc_ups_state->intFailure = ups_status & BIT_ERR;
	shc_ups_state->isPresent  = ups_status & BIT_IS_PRESENT;
	shc_ups_state->provPWR    = ups_status & BIT_PROV_PWR;
	shc_ups_state->chrg_lvl   = ups_charging_lvl;

	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** CPU indicates shutdown
*
*  \return    0 on success or error code
*
*  \sa SMB2SHC_PowerOff
*/
int32 __MAPILIB SMB2SHC_ShutDown()
{
	int err;

	err = SMB2API_WriteByte(g_SMB2SHC_smbHdl, SHC_SMBFLAGS,
							SHC_SMBADDR, SHC_SH_DOWN_OPCODE);
	if (err)
		return err;

	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** CPU indicates that it wants to shut off the power supply.
*
*  \return    0 on success or error code
*
*  \sa SMB2SHC_ShutDown
*/
int32 __MAPILIB SMB2SHC_PowerOff()
{
	int err;

	err = SMB2API_WriteByte(g_SMB2SHC_smbHdl, SHC_SMBFLAGS,
							SHC_SMBADDR, SHC_PWR_OFF_OPCODE);
	if (err)
		return err;

	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** Get the Configuration Data.
*
*  \param     configdata    \OUT  contains all the confiuration data
*  \return    0 on success or error code
*
*/
int32 __MAPILIB SMB2SHC_GetConf_Data(struct shc_configdata *configdata)
{
	int err;
	u_int8 length;
	u_int16 config_data16;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_CONF_GET_OPCODE, &length, blkData);
	if (err) {
		return err;
	}

	if (!((length == SHC_CONF_GET_LENGTH_v416 && SHC_V416_OR_BELOW) ||
	      (length == SHC_CONF_GET_LENGTH_v417 && SHC_AT_LEAST_V417))) {
		return SMB2_SHC_ERR_LENGTH;
	}

	configdata->pwrSlot2 = blkData[0];
	configdata->pwrSlot3 = blkData[1];

	configdata->upsMinStartCharge = blkData[2];
	configdata->upsMinRunCharge = blkData[3];

	config_data16 = ((u_int16)blkData[5] << 8);
	config_data16 += blkData[4];
	configdata->tempWarnLow = config_data16;

	config_data16 = ((u_int16)blkData[7] << 8);
	config_data16 += blkData[6];
	configdata->tempWarnHigh = config_data16;

	config_data16 = ((u_int16)blkData[9] << 8);
	config_data16 += blkData[8];
	configdata->tempRunLow = config_data16;

	config_data16 = ((u_int16)blkData[11] << 8);
	config_data16 += blkData[10];
	configdata->tempRunHigh = config_data16;

	if (SHC_V416_OR_BELOW) {
		configdata->persistentPwrbtnEnabled = 0;
		configdata->usePBRST = 1;

		configdata->fanNum = blkData[12];
		configdata->fanDuCyMin = blkData[13];

		config_data16 = ((u_int16)blkData[15] << 8);
		config_data16 += blkData[14];
		configdata->fanTempStart = config_data16;

		config_data16 = ((u_int16)blkData[17] << 8);
		config_data16 += blkData[16];
		configdata->fanTempMax = config_data16;

		configdata->voltMonMask = 15;
		configdata->i2cAddress = 0x75;

		configdata->StateMachineID = blkData[18];
	}
	else {
		configdata->persistentPwrbtnEnabled = blkData[12];
		configdata->usePBRST = blkData[13];

		configdata->fanNum = blkData[14];
		configdata->fanDuCyMin = blkData[15];

		config_data16 = ((u_int16)blkData[17] << 8);
		config_data16 += blkData[16];
		configdata->fanTempStart = config_data16;

		config_data16 = ((u_int16)blkData[19] << 8);
		config_data16 += blkData[18];
		configdata->fanTempMax = config_data16;

		configdata->voltMonMask = blkData[20] & 0x0f;

		configdata->i2cAddress = blkData[21];

		configdata->StateMachineID = blkData[22];
	}
	return SMB2_SHC_ERR_NO;
}


/****************************************************************************/
/** Get the firmware version.
*
*  \param     fw_version    \OUT  contains firmware version informations
*  \return    0 on success or error code
*
*/
int32 __MAPILIB SMB2SHC_GetFirm_Ver(struct shc_fwversion *fw_version)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(g_SMB2SHC_smbHdl, SHC_SMBFLAGS, SHC_SMBADDR,
								SHC_FVER_GET_OPCODE, &length, blkData);

	if (err)
		return err;

	if (length != SHC_FVER_GET_LENGTH)
		return SMB2_SHC_ERR_LENGTH;

	fw_version->errcode = blkData[0];

	fw_version->maj_revision = blkData[1];
	fw_version->min_revision = blkData[2];

	fw_version->mtnce_revision = blkData[3];
	fw_version->build_nbr = (u_int16)blkData[4] << 8;
	fw_version->build_nbr += blkData[5];

	fw_version->veri_flag = blkData[6];

	return SMB2_SHC_ERR_NO;
}

