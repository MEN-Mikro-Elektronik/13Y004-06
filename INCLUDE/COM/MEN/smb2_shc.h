/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  smb2_shc.h
 *
 *      \author  andreas.werner@men.de
 *        $Date: 2015/02/24 17:26:56 $
 *    $Revision: 3.2 $
 *
 *     \project  SMB2 SHC
 *       \brief  Shelf controller interface routines
 *
 *    \switches  -
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 2014-2019, MEN Mikro Elektronik GmbH
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
 
#ifndef _SMB2_SHC_H
# define _SMB2_SHC_H

#ifdef __cplusplus
	extern "C" {
#endif

#include <MEN/men_typs.h>
#include <MEN/mdis_err.h>

/*--------------------+
|   SHC Error Codes   |
+--------------------*/

#ifdef ERR_DEV
# define SHC_OFFS (ERR_DEV+0xf0)
#else
# define SHC_OFFS 0
#endif /*ERR_DEV*/

#define SMB2_SHC_ERR_NO        (0x00)            /** No Error */
#define SMB2_SHC_ID_NA         (SHC_OFFS + 0x1)  /** ID not available */
#define SMB2_SHC_ERR_LENGTH    (SHC_OFFS + 0x2)  /** Wrong length */

/*--------------------+
|   SHC Power Supply  |
+--------------------*/

/** Shelf Controller Power Supply IDs */
enum SHC_PSU_NR {
	SHC_PSU1 = 0,
	SHC_PSU2,
	SHC_PSU3
};

/** Shelf Controller PSU Status information */
struct shc_psu {
	/**
	 *  Flag to indicate if the selected PSU is available
	 */
	u_int8 isPresent;
	/**
	 *  Flag to indicate if the External Power is present
	 */
	u_int8 isEPwrPresent;
	/**
	 *  Flag to indicate if there is an internal error
	 */
	u_int8 intFailure;
};

/*--------------------+
|   SHC FANs          |
+--------------------*/

/** Shelf Controller FAN IDs */
enum SHC_FAN_NR {
	SHC_FAN1 = 0,
	SHC_FAN2,
	SHC_FAN3
};

/** Shelf Controller FAN state */
enum SHC_FAN_STAT {
	SHC_FAN_OK = 0,
	SHC_FAN_FAIL
};

/** Shelf Controller FAN Status information */
struct shc_fan {
	/**
	 *  Flag to indicate if the selected FAN is available
	 */
	u_int8 isPresent;
	/**
	 *  Current speed of the selected FAN (RPM)
	 */
	u_int16 speedRpm;
	/**
	 *  Current state of the selected FAN
	 */
	enum SHC_FAN_STAT state;
};

/*--------------------+
|  SHC Voltage Level  |
+--------------------*/

/** Shelf Controller PWR_MON IDs */
enum SHC_PWR_MON_ID {
	SHC_PWR_MON_1 = 0,
	SHC_PWR_MON_2,
	SHC_PWR_MON_3,
	SHC_PWR_MON_4
};

/*--------------------+
|   SHC UPS           |
+--------------------*/

/** Shelf Controller UPS IDs */
enum SHC_UPS_NR {
	SHC_UPS1 = 0,
	SHC_UPS2
};

/** Shelf Controller UPS Status information */
struct shc_ups {
	/**
	 *  Flag to indicate if the selected UPS is available
	 */
	u_int8 isPresent;
	/**
	 *  Flag to indicate if the External Power is present
	 */
	u_int8 provPWR;
	/**
	 *  Flag to indicate if there is an internal error
	 */
	u_int8 intFailure;
	/**
	 *  Charging level in %
	 */
	u_int8 chrg_lvl;
};

/*----------------------------+
|   SHC Configuration Data    |
+----------------------------*/

/** Shelf Controller Slot label */
enum SHC_SLOT_STATE {
	SHC_SLOT_EMPTY = 0,
	SHC_SLOT_PSU,
	SHC_SLOT_UPS
};

/** Shelf Controller Configuration Data */
struct shc_configdata {
	/**
	 *  Slot 2 indication
	 */
	enum SHC_SLOT_STATE pwrSlot2;
	/**
	 *  Slot 3 indication
	 */
	enum SHC_SLOT_STATE pwrSlot3;
	/**
	 *  UPS Minimum Start Charge in %
	 */
	u_int8 upsMinStartCharge;
	/**
	 *  UPS Minimum Run Charge in %
	 */
	u_int8 upsMinRunCharge;
	/**
	 *  Low Temperature Warning Limit in K
	 */
	u_int16 tempWarnLow;
	/**
	 *  Upper Temperature Warning Limit in K
	 */
	u_int16 tempWarnHigh;
	/**
	 *  Low Temperature Run Limit in K
	 */
	u_int16 tempRunLow;
	/**
	 *  High Temperature Run Limit in K
	 */
	u_int16 tempRunHigh;
	/**
	 *  Number of Fans
	 */
	u_int8 fanNum;
	/**
	 *  Fan Min Duty Cycle in %
	 */
	u_int8 fanDuCyMin;
	/**
	 *  Fan Start Temperature in K
	 */
	u_int16 fanTempStart;
	/**
	 *  Fan Max Speed Temperature in K
	 */
	u_int16 fanTempMax;
	/**
	 *  State Machine Id can be 1, 2 or 3
	 */
	u_int8 StateMachineID;
};

/*--------------------+
|   SHC Firmware      |
+--------------------*/

/** Shelf Controller Firmware Version Data */
struct shc_fwversion {
	/**
	 * Errorcode (0x00 = OK, 0xFF = Error)
	 */
	u_int8 errcode;
	/**
	 * Major Revision
	 */
	u_int8 maj_revision;
	/**
	 * Minor Revision
	 */
	u_int8 min_revision;
	/**
	 * Maintenance Revision
	 */
	u_int8 mtnce_revision;
	/** 
	 * Build number
	 */
	u_int16 build_nbr;
	/**
	 * Flag if firmware is verified or not
	 */
	u_int8 veri_flag;
};

extern int32 __MAPILIB SMB2SHC_ShutDown();
extern int32 __MAPILIB SMB2SHC_PowerOff();
extern int32 __MAPILIB SMB2SHC_Exit(void);
extern char* __MAPILIB SMB2SHC_Ident(void);
extern int32 __MAPILIB SMB2SHC_Init(char *deviceP);
extern int32 __MAPILIB SMB2SHC_GetTemperature(u_int16 *tempK);
extern int32 __MAPILIB SMB2SHC_SetTemperature(u_int16 tempK);
extern char* __MAPILIB SMB2SHC_Errstring(u_int32 errCode, char *strBuf);
extern int32 __MAPILIB SMB2SHC_GetFirm_Ver(struct shc_fwversion *fw_version);
extern int32 __MAPILIB SMB2SHC_GetConf_Data(struct shc_configdata *configdata);
extern int32 __MAPILIB SMB2SHC_GetPSU_State(enum SHC_PSU_NR psu_nr, struct shc_psu *shc_psu);
extern int32 __MAPILIB SMB2SHC_GetFAN_State(enum SHC_FAN_NR fan_nr, struct shc_fan *shc_fan);
extern int32 __MAPILIB SMB2SHC_GetUPS_State(enum SHC_UPS_NR ups_nr, struct shc_ups *shc_ups);
extern int32 __MAPILIB SMB2SHC_GetVoltLevel(enum SHC_PWR_MON_ID pwr_mon_nr, u_int16 *volt_value);

#ifdef __cplusplus
	}
#endif

#endif /*_SMB2_SHC_H*/
