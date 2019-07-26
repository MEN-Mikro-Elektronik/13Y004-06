/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  smb2_bmc_api.c
 *
 *      \author  quoc.bui@men.de
 *
 *       \brief  API functions to access the MEN BMC
 *
 *    \switches  -
 *
 *
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <MEN/men_typs.h>
#include <MEN/smb2_bmc_api.h>
#include <MEN/smb2_api.h>
#include <MEN/mdis_err.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* BMC Management Commands */
#define BMC_GET_FW_REV			0x80
#define BMC_SET_HW_BRD			0x8F
#define BMC_GET_HW_BRD			0x8F
#define BMC_GET_FEAT			0x84

/* WDOG Commands */
#define BMC_WDOG_ON				0x11
#define BMC_WDOG_OFF			0x12
#define BMC_WDOG_TRIG			0x13
#define BMC_WDOG_TIME_SET		0x14
#define BMC_WDOG_TIME_GET		0x14
#define BMC_WDOG_STATE_GET		0x17
#define BMC_WDOG_ARM			0x18
#define BMC_WDOG_ARM_STATE		0x19
#define BMC_WDOG_MIN_TIME_SET	0x1A
#define BMC_WDOG_MIN_TIME_GET	0x1A

/* Power Reset Control Commands */
#define BMC_RESUME_MODE_SET			0x20
#define BMC_RESUME_MODE_GET			0x20
#define BMC_EXT_PWR_FAIL_MODE_SET	0x21
#define BMC_EXT_PWR_FAIL_MODE_GET	0x21
#define BMC_RESET_IN_MODE_SET		0x22
#define BMC_RESET_IN_MODE_GET		0x22
#define BMC_EXT_PS_ON_MODE_SET		0x23
#define BMC_EXT_PS_ON_MODE_GET		0x23
#define BMC_SW_RESET				0x31
#define BMC_SW_COLD_RESET			0x32
#define BMC_SW_RTC_RESET			0x35
#define BMC_SW_HALT					0x36
#define BMC_PWRBTN					0x33
#define BMC_PWRBTN_OVRD				0x34
#define BMC_RST_REASON_GET			0x92
#define BMC_RST_REASON_CLR			0x9F

/* Voltage Reporting Commands */
#define BMC_VOLT_MAX_NUM		0x8E
#define BMC_VOLT_SET_IDX		0x61
#define BMC_VOLTAGE_GET			0x60

/* Life Time Reporting Commands */
#define BMC_PWRCYCLE_CNT		0x93
#define BMC_OP_HRS_CNT			0x94

/* Event Log Commands */
#define BMC_EVLOG_STAT			0x40
#define BMC_EVLOG_WRITE			0x41
#define BMC_EVLOG_READ_IDX		0x43
#define BMC_EVLOG_READ			0x42

/* Error Counter Commands */
#define BMC_ERRCNT_MAX			0x8D
#define BMC_ERRCNT_CLR			0x7F
#define BMC_ERRCNT_SET_IDX		0x71
#define BMC_ERRCNT_GET			0x70

/* Status Output Commands */
#define BMC_STAT_OUT_SET		0xA0
#define BMC_STAT_OUT_GET		0xA0

/* RTC Commands */
#define BMC_RTC_SET				0x50
#define BMC_RTC_GET				0x51

/* Backplane CPCI Commands */
#define BMC_CPCI_BRDMODE		0x8B
#define BMC_CPCI_SLOTADDR		0x8C

/* GPIO Commands */
#define BMC_GPO_CAPS			0xE0
#define BMC_GPO_SET				0xE1
#define BMC_GPO_GET				0xE2
#define BMC_GPI_CAPS			0xE8
#define BMC_GPI_GET				0xE9

/* CB30C Specific Commands */
#define BMC_PWR_LOG_SET			0x44
#define BMC_PWR_LOG_GET			0x44
#define BMC_STAT_FRM_TRIG		0xB3
#define BMC_GET_STAT_FRM		0xB4

#define BMC_SMBADDR				0x9C
#define BMC_SMBFLAGS			0x00

#define GPIO_SUPPORT			0x80
#define CPCI_SUPPORT			0x40
#define FUP_SUPPORT				0x20
#define RTC_SUPPORT				0x08
#define ERRCNT_SUPPORT			0x04
#define EVLOG_SUPPORT			0x02
#define VREP_SUPPORT			0x01
#define CLUS_SUPPORT			0x01
#define SRTCR_SUPPORT			0x10
#define RSIMD_SUPPORT			0x08
#define EPFMD_SUPPORT			0x04
#define RESMD_SUPPORT			0x02
#define HWB_SUPPORT				0x01

#define WDOG_OFF				0x69
#define RST_REASON_CLR			0x65
#define ERRCNT_CLR				0x66
#define STATUSOUT_ERR			0x80
#define STATUSOUT_USR_OUTPUTS	0x7C
#define STATUSOUT_HOTSWAP		0x02
#define STATUSOUT_STA			0x01
#define GPIO_ERR				0x80
#define RTC_BATTERY_LOW			0x01

#define BMC_GET_FW_REV_LENGTH	0x07
#define BMC_GET_FEAT_LENGTH		0x09
#define SW_RESET_LENGTH			0x04
#define SW_COLD_RESET_LENGTH	0x04
#define SW_RTC_RESET_LENGTH		0x04
#define SW_HALT_LENGTH			0x04
#define RST_REASON_LENGTH		0x08
#define VOLT_REPORT_LENGTH		0x09
#define PWRCYCL_CNT_LENGTH		0x04
#define OP_TIME_LENGTH			0x04
#define EVLOG_STAT_LENGTH		0x06
#define EVLOG_WRITE_LENGTH		0x06
#define EVLOG_READ_LENGTH		0x0D
#define ERRCNT_LENGTH			0x03
#define RTC_SET_LENGTH			0x07
#define RTC_GET_LENGTH			0x09
#define BMC_STAT_FRM_LENGTH		0x09

#define ERROR_CODE				0xFF			

/*-----------------------------------------+
|  GLOBALS                                 |
+-----------------------------------------*/
void *SMB2BMC_smbHdl;

/**
 * \defgroup _SMB2_BMC SMB2_BMC
 *  The SMB2_BMC_API provides access functions to communicate
 *  with the MEN BMC from user mode applications.
 *  The API uses the SMB2_API for the SMBUS access functions.
 *  @{
 */


/****************************************************************************/
/** Return ident string of the SMB2BMC API
 *
 *  \return    ident string
 *
 *  \sa SMB2BMC_Ident
 */
char* __MAPILIB SMB2BMC_Ident(void)
{
	return( (char*) IdentString );
}


/****************************************************************************/
/** Convert SMB2BMC, SMB2 and MDIS error codes to strings
*
* SMB2BMC_Errstring() creates an error message for error \a errCode and
* returns a pointer to the generated string with the following
* format:
*
*  \param errCode    \IN  error code from SMB2 function
*  \param strBuf     \OUT filled with error message (should have space for
*                         512 characters, including '\\0')
*  \return           \a   strBuf
*/
char* __MAPILIB SMB2BMC_Errstring(u_int32 errCode, char *strBuf)
{
	u_int32 i=0;
	char    *smbErr=NULL;

	/* SMB2_BMC error table */
	static struct _ERR_STR
	{
		unsigned int errCode;
		char         *errString;
	} errStrTable[] =
	{
		/* max string size indicator  |1---------------------------------------------50| */
		/* no error */
		{ SMB2_BMC_ERR_NO		,		"(no error)" },
		{ SMB2_BMC_ERR_LENGTH	,		"Data has wrong length" },
		{ SMB2_BMC_ERROR		,		"Error Code 0xFF" },
		{ SMB2_BMC_ERR_INPUT    ,		"Invalid Input"},
		/* max string size indicator  |1---------------------------------------------50| */
	};

	#define NBR_OF_ERR sizeof(errStrTable) / sizeof(struct _ERR_STR)

	/*----------------------+
	|  SMB2 BMC error?      |
	+----------------------*/
	if ((errCode >= (ERR_DEV + 0xe0)) && (errCode < ERR_END)) {

		/* search the error string */
		for (i=0; i<NBR_OF_ERR; i++) {
			if (errCode == (int32)errStrTable[i].errCode) {
				smbErr = errStrTable[i].errString;
			}
		}

		/* known SMB2_BMC_API error */
		if (smbErr) {
			sprintf(strBuf, "ERROR (SMB2_BMC_API) 0x%04x: %s", (unsigned int)errCode, smbErr);
		}
		/* unknown SMB2_BMC_API error? */
		else {
			sprintf(strBuf, "ERROR (SMB2_BMC_API) 0x%04x: Unknown SMB2_BMC_API error", (unsigned int)errCode);
		}
	}
	/*----------------------+
	|  MDIS or SMB2 error?  |
	+----------------------*/
	else {
		SMB2API_Errstring(errCode, strBuf);
	}

	return strBuf;
}


/****************************************************************************/
/** Initialization of the BMC API library
 *
 *  \param     deviceP    \IN  MDIS device name
 *  \return    0 on success or error code
 *
 *  \sa SMB2BMC_Init
 */
int32 __MAPILIB SMB2BMC_Init(char *deviceP)
{
	return SMB2API_Init(deviceP, &SMB2BMC_smbHdl);
}


/****************************************************************************/
/** Deinitialization of the BMC API library
 *
 *  \return     0 on success or error code
 *
 *  \sa SMB2BMC_Exit
 */
int32 __MAPILIB SMB2BMC_Exit()
{
	if (SMB2BMC_smbHdl)
		return (SMB2API_Exit(&SMB2BMC_smbHdl));

	return 0;
}

/****************************************************************************/
/** Get the firmware version.
*
*  \param     fw_version    \OUT  contains firmware version informations
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GetFirm_Ver
*/
int32 __MAPILIB SMB2BMC_GetFirm_Ver(struct bmc_fwversion *fw_version)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
				    BMC_GET_FW_REV, &length, blkData);

	if (err)
		return err;

	if (length != BMC_GET_FW_REV_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	fw_version->maj_revision = blkData[1];
	fw_version->min_revision = blkData[2];

	fw_version->mtnce_revision = blkData[3];
	fw_version->build_nbr = (u_int16)blkData[4] << 8;
	fw_version->build_nbr += blkData[5];

	fw_version->veri_flag = blkData[6];

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set the hardware board.
*
*  \param     s_board    \IN   contains hardware board information
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Set_HW_Brd
*/
int32 __MAPILIB SMB2BMC_Set_HW_Brd(u_int16 s_board)
{
	int err;
		
	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR, 
								BMC_SET_HW_BRD, s_board);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get the hardware board.
*
*  \param     g_board    \OUT  contains hardware board information
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_HW_Brd
*/
int32 __MAPILIB SMB2BMC_Get_HW_Brd(u_int16 *g_board)
{
	int err;

	err = SMB2API_ReadWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GET_HW_BRD, g_board);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get the features.
*
*  \param     features    \OUT  contains features supported
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_Features
*/
int32 __MAPILIB SMB2BMC_Get_Features(struct bmc_features *features)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GET_FEAT, &length, blkData);

	if (err)
		return err;

	if (length != BMC_GET_FEAT_LENGTH)
		return SMB2_BMC_ERR_LENGTH;
	
	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	features->gpio_support		= blkData[1] & GPIO_SUPPORT;
	features->cpci_support		= blkData[1] & CPCI_SUPPORT;
	features->fup_support 		= blkData[1] & FUP_SUPPORT;
	features->rtc_support 		= blkData[1] & RTC_SUPPORT;
	features->errcnt_support 	= blkData[1] & ERRCNT_SUPPORT;
	features->evlog_support  	= blkData[1] & EVLOG_SUPPORT;
	features->vrep_support 		= blkData[1] & VREP_SUPPORT;
	
	features->clus_support		= blkData[2] & CLUS_SUPPORT;
	
	features->srtcr_support  	= blkData[5] & SRTCR_SUPPORT;
	features->rsimd_support  	= blkData[5] & RSIMD_SUPPORT;
	features->epfmd_support  	= blkData[5] & EPFMD_SUPPORT;
	features->resmd_support  	= blkData[5] & RESMD_SUPPORT;
	features->hwb_support  		= blkData[5] & HWB_SUPPORT;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Enable WDOG.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_enable
*/
int32 __MAPILIB SMB2BMC_WDOG_enable(void)
{
	int err;

	err = SMB2API_WriteByte(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
							BMC_WDOG_ON);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Disable WDOG.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_disable
*/
int32 __MAPILIB SMB2BMC_WDOG_disable(void)
{
	int err;

	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_OFF, WDOG_OFF);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Trigger WDOG.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_trig
*/
int32 __MAPILIB SMB2BMC_WDOG_trig(void)
{
	int err;

	err = SMB2API_WriteByte(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
							BMC_WDOG_TRIG);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set upper limit of trigger time window.
*
*  \param     wd_max_tout    \IN  contains upper limit of trigger time window
*								  unit: 100 ms
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_TimeSet
*/
int32 __MAPILIB SMB2BMC_WDOG_TimeSet(u_int16 wd_max_tout)
{
	int err;

	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_TIME_SET, wd_max_tout);

	if (err)
		return err;
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get upper limit of trigger time window.
*
*  \param     wd_max_tout    \OUT  contains upper limit of trigger time window
*								   unit: 100 ms
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_TimeGet
*/
int32 __MAPILIB SMB2BMC_WDOG_TimeGet(u_int16 *wd_max_tout)
{
	int err;

	err = SMB2API_ReadWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_TIME_GET, wd_max_tout);

	if (err)
		return err;

	if (*wd_max_tout == 0xFFFF)
		return SMB2_BMC_ERROR;
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get Watchdog State.
*
*  \param     wd_state    \OUT  contains WDOG state
*								0x00: off
*								0x01: on
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_GetState
*/
int32 __MAPILIB SMB2BMC_WDOG_GetState(u_int8 *wd_state)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_STATE_GET, wd_state);

	if (err)
		return err;
	
	if (*wd_state == 0xFF)
		return SMB2_BMC_ERROR;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Arm Watchdog (Main CPU) and BIOS timeouts.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_Arm
*/
int32 __MAPILIB SMB2BMC_WDOG_Arm(void)
{
	int err;

	err = SMB2API_WriteByte(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
							BMC_WDOG_ARM);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get WDOG Arming State.
*
*  \param     arm_state    \OUT  contains arm state
*								 0x00: not armed
*								 0x01: armed
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_GetArmState
*/
int32 __MAPILIB SMB2BMC_WDOG_GetArmState(u_int8 *arm_state)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_ARM_STATE, arm_state);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set lower limit of trigger time window.
*
*  \param     wd_min_tout    \IN  contains lower limit of trigger time window
*								  unit: 10 ms
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_MinTimeSet
*/
int32 __MAPILIB SMB2BMC_WDOG_MinTimeSet(u_int16 wd_min_tout)
{
	int err;

	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_MIN_TIME_SET, wd_min_tout);

	if (err)
		return err;
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get lower limit of trigger time window.
*
*  \param     wd_min_tout    \OUT  contains lower limit of trigger time window
*								   unit: 10 ms
*  \return    0 on success or error code
*
*  \sa SMB2BMC_WDOG_MinTimeGet
*/
int32 __MAPILIB SMB2BMC_WDOG_MinTimeGet(u_int16 *wd_min_tout)
{
	int err;

	err = SMB2API_ReadWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_WDOG_MIN_TIME_GET, wd_min_tout);

	if (err)
		return err;

	if (*wd_min_tout == 0xFFFF)
		return SMB2_BMC_ERROR;
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set Power Resume Mode.
*
*  \param     res_mode    \IN  contains resume mode
*							   0x00: off
*							   0x01: on
*							   0x02: former
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ResumeModeSet
*/
int32 __MAPILIB SMB2BMC_ResumeModeSet(u_int8 res_mode)
{
	int err;

	if ((res_mode < 0x00) || (res_mode > 0x02))
		return SMB2_BMC_ERR_INPUT;
	
	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RESUME_MODE_SET, res_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}


/****************************************************************************/
/** Get Power Resume Mode.
*
*  \param     res_mode    \OUT  contains resume mode
*								0x00: off
*								0x01: on
*								0x02: former
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ResumeModeGet
*/
int32 __MAPILIB SMB2BMC_ResumeModeGet(u_int8 *res_mode)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RESUME_MODE_GET, res_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set external Power Supply Failure Mode.
*
*  \param     ext_pwr_fail_mode    \IN  contains external power supply failure mode
*										0x00: ignore
*										0x01: treat as error
*										0xFF: ERROR
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ExtPwrFailModeSet
*/
int32 __MAPILIB SMB2BMC_ExtPwrFailModeSet(u_int8 ext_pwr_fail_mode)
{
	int err;

	if ((ext_pwr_fail_mode < 0) || (ext_pwr_fail_mode > 1))
		return SMB2_BMC_ERR_INPUT;
	
	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_EXT_PWR_FAIL_MODE_SET, ext_pwr_fail_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get external Power Supply Failure Mode.
*
*  \param     ext_pwr_fail_mode    \OUT  contains external power supply failure mode
*										0x00: ignore
*										0x01: treat as error
*										0xFF: ERROR
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ExtPwrFailModeGet
*/
int32 __MAPILIB SMB2BMC_ExtPwrFailModeGet(u_int8 *ext_pwr_fail_mode)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_EXT_PWR_FAIL_MODE_GET, ext_pwr_fail_mode);

	if (err)
		return err;
	
	if (*ext_pwr_fail_mode == ERROR_CODE)
		return SMB2_BMC_ERROR;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set RESET_IN Mode.
*
*  \param     reset_in_mode    \IN  contains RESET_IN mode
*									0x00: resets enabled
*									0x01: resets masked
*									0xFF: ERROR
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ResetInModeSet
*/
int32 __MAPILIB SMB2BMC_ResetInModeSet(u_int8 reset_in_mode)
{
	int err;
	
	if ((reset_in_mode < 0) || (reset_in_mode > 1))
		return SMB2_BMC_ERR_INPUT;
	
	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RESET_IN_MODE_SET, reset_in_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get RESET_IN Mode.
*
*  \param     reset_in_mode    \OUT  contains RESET_IN mode
*									0x00: resets enabled
*									0x01: resets masked
*									0xFF: ERROR
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ResetInModeGet
*/
int32 __MAPILIB SMB2BMC_ResetInModeGet(u_int8 *reset_in_mode)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RESET_IN_MODE_GET, reset_in_mode);

	if (err)
		return err;
	
	if (*reset_in_mode == ERROR_CODE)
		return SMB2_BMC_ERROR;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set EXT_PS_ON Mode.
*
*  \param     param1    \OUT  contains lower limit of trigger time window
*  \return    0 on success or error code
*
*  \sa SMB2BMC_
*/

/****************************************************************************/
/** Get EXT_PS_ON Mode.
*
*  \param     param1    \OUT  contains lower limit of trigger time window
*  \return    0 on success or error code
*
*  \sa SMB2BMC_
*/

/****************************************************************************/
/** Initiate Software Reset.
*
*  \param     reset_cause    \IN  contains the reset cause
*  \return    0 on success or error code
*
*  \sa SMB2BMC_SW_Reset
*/
int32 __MAPILIB SMB2BMC_SW_Reset(u_int16 reset_cause)
{
	int err;
	u_int8 reset_cause_lsb;
	u_int8 reset_cause_msb;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	reset_cause_lsb = (u_int8)reset_cause;
	reset_cause_msb = (u_int8)(reset_cause >> 8);
	
	blkData[0] = 0xa0;
	blkData[1] = 0xde;
	blkData[2] = reset_cause_lsb;
	blkData[3] = reset_cause_msb;

	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_SW_RESET, SW_RESET_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Initiate Cold Reset.
*
*  \param     reset_cause    \IN  contains the reset cause
*  \return    0 on success or error code
*
*  \sa SMB2BMC_SW_ColdReset
*/
int32 __MAPILIB SMB2BMC_SW_ColdReset(u_int16 reset_cause)
{
	int err;
	u_int8 reset_cause_lsb;
	u_int8 reset_cause_msb;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	reset_cause_lsb = (u_int8)reset_cause;
	reset_cause_msb = (u_int8)(reset_cause >> 8);
	
	blkData[0] = 0xa3;
	blkData[1] = 0xde;
	blkData[2] = reset_cause_lsb;
	blkData[3] = reset_cause_msb;

	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_SW_COLD_RESET, SW_COLD_RESET_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Initiate Cold Reset combined with RTC Reset.
*
*  \param     reset_cause    \IN  contains the reset cause
*  \return    0 on success or error code
*
*  \sa SMB2BMC_SW_RTC_Reset
*/
int32 __MAPILIB SMB2BMC_SW_RTC_Reset(u_int16 reset_cause)
{
	int err;
	u_int8 reset_cause_lsb;
	u_int8 reset_cause_msb;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	reset_cause_lsb = (u_int8)reset_cause;
	reset_cause_msb = (u_int8)(reset_cause >> 8);
	
	blkData[0] = 0xa5;
	blkData[1] = 0xde;
	blkData[2] = reset_cause_lsb;
	blkData[3] = reset_cause_msb;

	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_SW_RTC_RESET, SW_RTC_RESET_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Initiate Halt.
*
*  \param     reset_cause    \IN  contains the reset cause
*  \return    0 on success or error code
*
*  \sa SMB2BMC_SW_Halt
*/
int32 __MAPILIB SMB2BMC_SW_Halt(u_int16 reset_cause)
{
	int err;
	u_int8 reset_cause_lsb;
	u_int8 reset_cause_msb;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	reset_cause_lsb = (u_int8)reset_cause;
	reset_cause_msb = (u_int8)(reset_cause >> 8);
	
	blkData[0] = 0x77;
	blkData[1] = 0xde;
	blkData[2] = reset_cause_lsb;
	blkData[3] = reset_cause_msb;
	
	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_SW_HALT, SW_HALT_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Perform Power Button Press.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_PWRBTN_Press
*/

/****************************************************************************/
/** Perform Power Button Override.
*
*  \param     wd_min_tout    \OUT  contains lower limit of trigger time window
*  \return    0 on success or error code
*
*  \sa SMB2BMC_PWRBTN_Override
*/

/****************************************************************************/
/** Get last reset reason.
*
*  \param     reset_reason    \OUT  contains the reason for a reset
*  \return    0 on success or error code
*
*  \sa SMB2BMC_RstReasonGet
*/
int32 __MAPILIB SMB2BMC_RstReasonGet(struct bmc_rst_reason *reset_reason)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RST_REASON_GET, &length, blkData);

	if (err)
		return err;
	
	if (length != RST_REASON_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	reset_reason->procID = blkData[1];
	reset_reason->ev_code = ((u_int16)blkData[3] << 8) + blkData[2];
	reset_reason->ev_info1 = blkData[4];
	reset_reason->ev_info2 = blkData[5];
	reset_reason->ev_info3 = blkData[6];
	reset_reason->ev_info4 = blkData[7];
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Clear last reset reason.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_RstReasonCLR
*/
int32 __MAPILIB SMB2BMC_RstReasonCLR(void)
{
	int err;
	
	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RST_REASON_CLR, RST_REASON_CLR);

	if (err)
		return err;
		
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get Number of Voltages measured by BMC.
*
*  \param     volt_max_num    \OUT  number of voltages measured by BMC
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Volt_Max_Num
*/
int32 __MAPILIB SMB2BMC_Volt_Max_Num(u_int8 *volt_max_num)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_VOLT_MAX_NUM, volt_max_num);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get actual value and configured value of one voltage.
*
*  \param     volt_idx       \IN   index to read from
*  \param     volt_report    \OUT  voltage report
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Volt_Get
*/
int32 __MAPILIB SMB2BMC_Volt_Get(u_int8 volt_idx, struct bmc_voltage_report *volt_report)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_VOLT_SET_IDX, volt_idx);

	if (err)
		return err;
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_VOLTAGE_GET, &length, blkData);

	if (err)
		return err;
	
	if (length != VOLT_REPORT_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	volt_report->actual_volt = ((u_int16)blkData[2] << 8)+ blkData[1];
	volt_report->nominal_volt = ((u_int16)blkData[4] << 8) + blkData[3];
	volt_report->low_lim_volt = ((u_int16)blkData[6] << 8) + blkData[5];
	volt_report->up_lim_volt = ((u_int16)blkData[8] << 8) + blkData[7];
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get Power Cycle Counter.
*
*  \param     pwr_cycles    \OUT  power cycle counter
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_PwrCycleCnt
*/
int32 __MAPILIB SMB2BMC_Get_PwrCycleCnt(u_int32 *pwr_cycles)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_PWRCYCLE_CNT, &length, blkData);

	if (err)
		return err;
	
	if (length != PWRCYCL_CNT_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	*pwr_cycles = (u_int32)(blkData[3] << 24);
	*pwr_cycles += (u_int32)(blkData[2] << 16);
	*pwr_cycles += (u_int32)(blkData[1] << 8);
	*pwr_cycles += (u_int32)(blkData[0]);
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get Operating Hours Counter.
*
*  \param     op_time    \OUT  operating hours counter
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_OpHoursCnt
*/
int32 __MAPILIB SMB2BMC_Get_OpHoursCnt(u_int32 *op_time)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_OP_HRS_CNT, &length, blkData);

	if (err)
		return err;
	
	if (length != OP_TIME_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	*op_time = (u_int32)(blkData[3] << 24);
	*op_time += (u_int32)(blkData[2] << 16);
	*op_time += (u_int32)(blkData[1] << 8);
	*op_time += (u_int32)(blkData[0]);
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get event log status.
*
*  \param     evlog_stat    \OUT  event log status
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_EventLog_Status
*/
int32 __MAPILIB SMB2BMC_Get_EventLog_Status(struct bmc_evlog_status *evlog_stat)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_EVLOG_STAT, &length, blkData);

	if (err)
		return err;
	
	if (length != EVLOG_STAT_LENGTH)
		return SMB2_BMC_ERR_LENGTH;
	
	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	evlog_stat->rtcts = blkData[1] & 0x01;
	evlog_stat->max_entries = blkData[2];
	evlog_stat->max_entries += (u_int16)blkData[3] << 8;
	evlog_stat->act_entries = blkData[4];
	evlog_stat->act_entries += (u_int16)blkData[5] << 8;
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Add event to event log.
*
*  \param     ev_code     \IN  event code
*  \param     ev_info1    \IN  event info 1
*  \param     ev_info2    \IN  event info 2
*  \param     ev_info3    \IN  event info 3
*  \param     ev_info4    \IN  event info 4
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Add_Event
*/
int32 __MAPILIB SMB2BMC_Add_Event(u_int16 ev_code, u_int8 ev_info1, u_int8 ev_info2,
									u_int8 ev_info3, u_int8 ev_info4)
{
	int err;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	blkData[0] = (u_int8)ev_code;
	blkData[1] = (u_int8)(ev_code >> 8);
	blkData[2] = ev_info1;
	blkData[3] = ev_info2;
	blkData[4] = ev_info3;
	blkData[5] = ev_info4;

	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_EVLOG_WRITE, EVLOG_WRITE_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Read one event from the event log.
*
*  \param     evlog_idx       \IN   index to read from
*  \param     event_report    \OUT  event report
*  \return    0 on success or error code
*
*  \sa SMB2BMC_EventLog_Read
*/
int32 __MAPILIB SMB2BMC_EventLog_Read(u_int16 evlog_idx, 
									  struct bmc_event_report *event_report)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR, 
								BMC_EVLOG_READ_IDX, evlog_idx);

	if (err)
		return err;
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_EVLOG_READ, &length, blkData);

	if (err)
		return err;
	
	if (length != EVLOG_READ_LENGTH)
		return SMB2_BMC_ERR_LENGTH;
	
	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;

	event_report->tstamp = ((u_int64)blkData[5] & 0x7F) << 32;
	event_report->tstamp += (u_int64)blkData[4] << 24;
	event_report->tstamp += (u_int64)blkData[3] << 16;
	event_report->tstamp += (u_int64)blkData[2] << 8;
	event_report->tstamp += (u_int64)blkData[1]; 
	
	event_report->rtcv = blkData[5] & 0x80;
	event_report->procID = blkData[6];
	event_report->ev_code = ((u_int16)blkData[8] << 8) + blkData[7];
	event_report->ev_info1 = blkData[9];
	event_report->ev_info2 = blkData[10];
	event_report->ev_info3 = blkData[11];
	event_report->ev_info4 = blkData[12];
	
	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get number of error counters supported by this BMC.
*
*  \param     errcnt_max_idx    \OUT  number of error counters supported by this BMC
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ErrCnt_MaxIDX
*/
int32 __MAPILIB SMB2BMC_ErrCnt_MaxIDX(u_int8 *errcnt_max_idx)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_ERRCNT_MAX, errcnt_max_idx);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Clear all Error Counters.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_ErrCnt_Clear
*/
int32 __MAPILIB SMB2BMC_ErrCnt_Clear(void)
{
	int err;

	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_ERRCNT_CLR, ERRCNT_CLR);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get Error Counter by Index.
*
*  \param     errcnt_idx   \IN  index to read from
*  \param     error_cnt    \OUT  error counter by index
*  \return    0 on success or error code
*
*  \sa SMB2BMC_Get_ErrCnt
*/
int32 __MAPILIB SMB2BMC_Get_ErrCnt(u_int8 errcnt_idx, u_int16 *error_cnt)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_ERRCNT_SET_IDX, errcnt_idx);

	if (err)
		return err;
	
	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_ERRCNT_GET, &length, blkData);

	if (err)
		return err;

	if (length != ERRCNT_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	*error_cnt = ((u_int16)blkData[2] << 8) + blkData[1];

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set status outputs.
*
*  \param     on_off        \IN  0=off, >0=on
*  \param     status_out    \IN  selects status output
*  \return    0 on success or error code
*
*  \sa SMB2BMC_StatusOutput_Set
*/
int32 __MAPILIB SMB2BMC_StatusOutput_Set(enum STATUS_OUTPUT status_out, u_int8 on_off)
{
	int err;
	u_int16 status_output_data=0;

	status_output_data = (1 << (status_out + 8)) | ((on_off?1:0) << status_out);
	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
		BMC_STAT_OUT_SET, status_output_data);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get status outputs.
*
*  \param     status           \OUT  0x00   : off
*									 != 0x00: on
*  \param     status_output    \OUT  status outputs
*  \return    0 on success or error code
*
*  \sa SMB2BMC_StatusOutput_Get
*/
int32 __MAPILIB SMB2BMC_StatusOutput_Get(enum STATUS_OUTPUT status_out, u_int8 *status)
{
	int err;
	u_int8 status_buffer;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_STAT_OUT_GET, &status_buffer);

	if (err)
		return err;

	if(status_buffer & STATUSOUT_ERR)
		return SMB2_BMC_ERROR;
	
	*status = status_buffer & (0x01 << status_out);

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Set RTC.
*
*  \param     year   \IN  year
*  \param     month  \IN  month
*  \param     day    \IN  day
*  \param     hrs    \IN  hours
*  \param     min    \IN  minutes
*  \param     sec    \IN  seconds
*  \return    0 on success or error code
*
*  \sa SMB2BMC_RTC_Set
*/
int32 __MAPILIB SMB2BMC_RTC_Set(u_int16 year, u_int8 month, u_int8 mday, 
								u_int8 hrs, u_int8 min, u_int8 sec )
{
	int err;
	u_int8 year_lsb;
	u_int8 year_msb;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];
	
	if (!((0 < month) && (month <= 12)))
		return SMB2_BMC_ERR_INPUT;
	if (!((0 < mday) && (mday <= 31)))
		return SMB2_BMC_ERR_INPUT;
	if (!((0 <= hrs) && (hrs <= 24)))
		return SMB2_BMC_ERR_INPUT;
	if (!((0 <= min) && (min <= 60)))
		return SMB2_BMC_ERR_INPUT;
	if (!((0 <= sec) && (sec <= 60)))
		return SMB2_BMC_ERR_INPUT;
	
	year_lsb = (u_int8)year;
	year_msb = (u_int8)(year >> 8);
	
	blkData[0] = year_lsb;
	blkData[1] = year_msb;
	blkData[2] = month;
	blkData[3] = mday;
	blkData[4] = hrs;
	blkData[5] = min;
	blkData[6] = sec;
	
	err = SMB2API_WriteBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								 BMC_RTC_SET, RTC_SET_LENGTH, blkData);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get RTC.
*
*  \param     rtc   \OUT  rtc
*  \return    0 on success or error code
*
*  \sa SMB2BMC_RTC_Get
*/
int32 __MAPILIB SMB2BMC_RTC_Get(struct bmc_rtc *rtc)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_RTC_GET, &length, blkData);

	if (err)
		return err;

	if (length != RTC_GET_LENGTH)
		return SMB2_BMC_ERR_LENGTH;

	if(blkData[0] == ERROR_CODE)
		return SMB2_BMC_ERROR;
	
	rtc->year = ((u_int16)blkData[2] << 8) + blkData[1];
	rtc->month = blkData[3];
	rtc->mday = blkData[4];
	rtc->hours = blkData[5];
	rtc->minutes = blkData[6];
	rtc->seconds = blkData[7];
	rtc->battery = blkData[8] & RTC_BATTERY_LOW;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get CPCI Board Mode.
*
*  \param     cpci_mode   \OUT  CPCI Board Mode
*  \return    0 on success or error code
*
*  \sa SMB2BMC_CPCI_BrdMode
*/
int32 __MAPILIB SMB2BMC_CPCI_BrdMode(u_int8 *cpci_mode)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_CPCI_BRDMODE, cpci_mode);

	if (err)
		return err;
	
	if (*cpci_mode == ERROR_CODE)
		return SMB2_BMC_ERROR;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get CPCI peripheral slot address.
*
*  \param     cpci_slotaddr   \OUT  CPCI peripheral slot address
*  \return    0 on success or error code
*
*  \sa SMB2BMC_CPCI_SlotAddr
*/
int32 __MAPILIB SMB2BMC_CPCI_SlotAddr(u_int8 *cpci_slotaddr)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_CPCI_SLOTADDR, cpci_slotaddr);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Report which GPOs are supported.
*
*  \param     gpo           \IN   selects GPO
*  \param     gpo_support   \OUT  '0': not supported
*								  '1': supported
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GPO_Caps
*/
int32 __MAPILIB SMB2BMC_GPO_Caps(enum GPO gpo, u_int8 *gpo_support)
{
	int err;
	u_int8 gpo_caps;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GPO_CAPS, &gpo_caps);

	if (err)
		return err;
	
	if (gpo_caps & GPIO_ERR)
		return SMB2_BMC_ERROR;
	
	*gpo_support = gpo_caps & (0x01 << gpo);

	return SMB2_BMC_ERR_NO;
}


/****************************************************************************/
/** Set general purpose outputs.
*
*  \param     on_off   \IN  0=off, >0=on
*  \param     gpo      \IN  selects GPO
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GPO_Set
*/
int32 __MAPILIB SMB2BMC_GPO_Set(enum GPO gpo, u_int8 on_off)
{
	int err;
	u_int16 gpo_data;
	
	gpo_data = (1 << (gpo + 8)) | ((on_off ? 1 : 0) << gpo);
	err = SMB2API_WriteWordData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
		BMC_GPO_SET, gpo_data);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}


/****************************************************************************/
/** Get general purpose outputs.
*
*  \param     gpo      \IN  selects gpo
*  \param     status   \IN  '0': LOW level
*							'1': HIGH level
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GPO_Get
*/
int32 __MAPILIB SMB2BMC_GPO_Get(enum GPO gpo, u_int8 *status)
{
	int err;
	u_int8 gpo_level;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GPO_GET, &gpo_level);

	if (err)
		return err;
	
	if (gpo_level & GPIO_ERR)
		return SMB2_BMC_ERROR;

	*status = gpo_level & (0x01 << gpo);
	
	return SMB2_BMC_ERR_NO;
}


/****************************************************************************/
/** Report which GPIs are supported.
*
*  \param     gpi           \IN  selects GPI
*  \param     gpi_support   \OUT  '0': not supported
*								 '1': supported
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GPI_Caps
*/
int32 __MAPILIB SMB2BMC_GPI_Caps(enum GPI gpi, u_int8 *gpi_support)
{
	int err;
	u_int8 gpi_caps;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GPI_CAPS, &gpi_caps);

	if (err)
		return err;
	
	if (gpi_caps & GPIO_ERR)
		return SMB2_BMC_ERROR;
	
	*gpi_support = gpi_caps & (0x01 << gpi);

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get general purpose inputs.
*
*  \param     gpi      \IN  selects GPI
*  \param     status   \IN  '0': LOW level
*							'1': HIGH level
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GPI_Get
*/
int32 __MAPILIB SMB2BMC_GPI_Get(enum GPI gpi, u_int8 *status)
{
	int err;
	u_int8 gpi_levels;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GPI_GET, &gpi_levels);

	if (err)
		return err;
	
	if (gpi_levels & GPIO_ERR)
		return SMB2_BMC_ERROR;
	
	*status = gpi_levels & (0x01 << gpi);

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Enable/Disable power on/off event logging.
*
*  \param     pwr_log_mode   \IN  power on/off event logging mode
*  \return    0 on success or error code
*
*  \sa SMB2BMC_PWR_SetEvLog
*/
int32 __MAPILIB SMB2BMC_PWR_SetEvLog(u_int8 pwr_log_mode)
{
	int err;
	
	if ((pwr_log_mode < 0) || (pwr_log_mode > 1))
		return SMB2_BMC_ERR_INPUT;
	
	err = SMB2API_WriteByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_PWR_LOG_SET, pwr_log_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get current setting of power on/off event logging.
*
*  \param     pwr_log_mode   \IN  power on/off event logging mode
*  \return    0 on success or error code
*
*  \sa SMB2BMC_PWR_GetEvLog
*/
int32 __MAPILIB SMB2BMC_PWR_GetEvLog(u_int8 *pwr_log_mode)
{
	int err;

	err = SMB2API_ReadByteData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_PWR_LOG_GET, pwr_log_mode);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Trigger a new SUPV Status Frame Transfer.
*
*  \return    0 on success or error code
*
*  \sa SMB2BMC_StatusFrame_trigger
*/
int32 __MAPILIB SMB2BMC_StatusFrame_trigger(void)
{
	int err;

	err = SMB2API_WriteByte(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
							BMC_STAT_FRM_TRIG);

	if (err)
		return err;

	return SMB2_BMC_ERR_NO;
}

/****************************************************************************/
/** Get last from SUPV received status frame.
*
*  \param     status_frame    \OUT  contains SUPV Status Frame
*  \return    0 on success or error code
*
*  \sa SMB2BMC_GetStatusFrame
*/
int32 __MAPILIB SMB2BMC_GetStatusFrame(struct bmc_status_frame *status_frame)
{
	int err;
	u_int8 length;
	u_int8 blkData[SMB_BLOCK_MAX_BYTES];

	err = SMB2API_ReadBlockData(SMB2BMC_smbHdl, BMC_SMBFLAGS, BMC_SMBADDR,
								BMC_GET_STAT_FRM, &length, blkData);

	if (err)
		return err;

	if (length != BMC_STAT_FRM_LENGTH)
		return SMB2_BMC_ERR_LENGTH;
	
	status_frame->wd_itout = blkData[0] & 0x40;
	status_frame->isw_st = blkData[0] & 0x20;
	status_frame->fpga_rdy = blkData[0] & 0x10;
	status_frame->state = blkData[0] & 0x0F;
	
	status_frame->fault_cause = blkData[1] & 0x7F;
	
	status_frame->maj_rev = (blkData[2] >> 4) & 0x07;
	status_frame->min_rev = blkData[2] & 0x0F;

	status_frame->ov_shdn = blkData[3] & 0x40;
	status_frame->fpga_mon_en = blkData[3] & 0x20;
	status_frame->restart = blkData[3] & 0x10;
	status_frame->test_mode = blkData[3] & 0x08;
	status_frame->nom_ss_clk = blkData[3] & 0x07;
	
	status_frame->wd_tout_ul = (blkData[4] >> 4) & 0x07;
	status_frame->wd_tout_ll = blkData[4] & 0x08;
	status_frame->wd_en = blkData[4] & 0x04;
	status_frame->wd_init_tout = blkData[4] & 0x03;
	
	status_frame->supv_id = blkData[5] & 0x7F;
	status_frame->supv_id += ((u_int16)blkData[6] << 8) & 0x1F00;
	
	status_frame->life_sign_bit = blkData[6] & 0x40;
	
	status_frame->supv_id_build_no = blkData[7] & 0x7F;
	
	status_frame->crc = blkData[8] & 0xFF;

	return SMB2_BMC_ERR_NO;
}








