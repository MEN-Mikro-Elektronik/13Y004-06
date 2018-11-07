/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  bmc_api.h
 *
 *      \author  Roman.Schneider@men.de
 *        $Date: 2015/07/12 11:48:18 $
 *    $Revision: 3.3 $
 *
 *     \project  SMB2 Library
 *       \brief  BMC API interface
 *    \switches  
 */
/*
 *---------------------------------------------------------------------------
 * (c) Copyright 2013 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
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
#ifndef _BMC_API_H
#  define _BMC_API_H

#  ifdef __cplusplus
      extern "C" {
#  endif

/*-------------------------------------+
|    DEFINES                           |
+-------------------------------------*/
#define BMC_DEFAULT_ADDR            0x9C

/*! \defgroup SMB_COMMANDS SMBus command opcodes */ /**@{*/

/*! \defgroup FS_BMC_MANAGEMENT Feature Set FS_BMC_MANAGEMENT */ /**@{*/
#define BMC_FIRMWARE_REV            0x80
#define BMC_HW_BOARD                0x8F
#define BMC_FEATURES                0x84

#define BMC_FEATURE_D1_VREP         0x01
#define BMC_FEATURE_D1_EVLOG        0x02
#define BMC_FEATURE_D1_ERRCNT       0x04
#define BMC_FEATURE_D1_RTC          0x08
#define BMC_FEATURE_D1_FUP          0x20
#define BMC_FEATURE_D1_CPCI         0x40
#define BMC_FEATURE_D1_GPIO         0x80

#define BMC_FEATURE_D2_CLUS         0x01

#define BMC_FEATURE_D5_HWB          0x01
#define BMC_FEATURE_D5_RESMD        0x02
#define BMC_FEATURE_D5_EPFMD        0x04
#define BMC_FEATURE_D5_RSIMD        0x08
#define BMC_FEATURE_D5_SRTCR        0x10


/*! \defgroup FS_BOARD_CONSTANT */
#define BMC_BOARD_CONSTANT_XM1      0x0001
#define BMC_BOARD_CONSTANT_MM1      0x0002
#define BMC_BOARD_CONSTANT_XM2      0x0003
#define BMC_BOARD_CONSTANT_F19P     0x0004
#define BMC_BOARD_CONSTANT_G20      0x0005
#define BMC_BOARD_CONSTANT_SC21     0x0006
#define BMC_BOARD_CONSTANT_F11S     0x0007
#define BMC_BOARD_CONSTANT_F21P     0x0008
#define BMC_BOARD_CONSTANT_MM2      0x0009
#define BMC_BOARD_CONSTANT_F75P     0x0010
#define BMC_BOARD_CONSTANT_F76P     0x0011


/**@}*/

/*! \defgroup FS_WATCHDOG Feature Set FS_WATCHDOG */
#define BMC_WDOG_ON                 0x11
#define BMC_WDOG_OFF                0x12
#define BMC_WDOG_OFF_DATA           0x69
#define BMC_WDOG_TRIG               0x13
#define BMC_WDOG_TIME_SET           0x14
#define BMC_WDOG_TIME_GET           0x14
#define BMC_WDOG_STATE_GET          0x17
#define BMC_WDOG_ARM                0x18
#define BMC_WDOG_ARM_STATE          0x19
#define BMC_WDOG_MIN_TIME_SET       0x1A
#define BMC_WDOG_MIN_TIME_GET       0x1A


/*! \defgroup FS_POWER_RESET_CONTROL Feature Set FS_POWER_RESET_CONTROL */
#define BMC_EXT_PWR_FAIL_MODE       0x21
#define BMC_RST_IN_MODE             0x22
#define BMC_SW_RESET                0x31
#define BMC_SW_COLD_RESET           0x32
#define BMC_SW_RTC_RESET            0x35
#define BMC_SW_HALT                 0x36
#define BMC_RST_REASON_GET          0x92
#define BMC_RST_REASON_CLR          0x9F
#define BMC_RST_REASON_CLR_DATA     0x65
/**@}*/

/*! \defgroup FS_VOLTAGE_REPORTING Feature Set FS_VOLTAGE_REPORTING */
#define BMC_VOLT_MAX_IDX            0x8E
#define BMC_VOLTAGE_SET_IDX         0x61
#define BMC_VOLTAGE_GET             0x60

/**@}*/

/*! \defgroup FS_EVENT_LOG Feature Set FS_EVENT_LOG */
#define BMC_EVLOG_STAT              0x40
#define BMC_EVLOG_WRITE             0x41
#define BMC_EVLOG_READ              0x42
#define BMC_EVLOG_WRITE_IDX         0x43
/**@}*/

/*! \defgroup FS_STATUS_OUTPUTS Feature Set FS_STATUS_OUTPUTS */
#define BMC_STATUS_OUTPUT_SET       0xA0
#define BMC_STATUS_OUTPUT_GET       0xA0
/**@}*/

/*! \defgroup FS_LIFE_TIME_REPORTING  Feature Set FS_LIFE_TIME_REPORTING */
#define BMC_PWRCYCL_CNT             0x93
#define BMC_OP_HRS_CNT              0x94
/**@}*/

/*! \defgroup FS_CLUSTER_SUPPORT  Feature Set FS_CLUSTER_SUPPORT */
#define BMC_CLUS_CHAN_GET           0xF0
#define BMC_CLUS_CHAN_SET           0xF1
/**@}*/


/**@}*/


/*-------------------------------------+
|    TYPEDEFS                          |
+-------------------------------------*/
struct _BMC_BOARD_TYPE
{
	u_int16 boardCode;
	char *boardName;
} bmcBrdTypeTbl[] =
{
	{ BMC_BOARD_CONSTANT_XM1,	"XM1" },
	{ BMC_BOARD_CONSTANT_MM1,	"MM1" },
	{ BMC_BOARD_CONSTANT_XM2,	"XM2" },
	{ BMC_BOARD_CONSTANT_F19P,	"F19P" },
	{ BMC_BOARD_CONSTANT_G20,	"G20" },
	{ BMC_BOARD_CONSTANT_SC21,	"SC21" },
	{ BMC_BOARD_CONSTANT_F11S,	"F11S" },
	{ BMC_BOARD_CONSTANT_F21P,	"F21P" },
	{ BMC_BOARD_CONSTANT_MM2,	"MM2" },
	{ BMC_BOARD_CONSTANT_F75P,	"F75P" },
	{ BMC_BOARD_CONSTANT_F76P,	"F76P" },
};
#define BMC_NBR_OF_BRDTYPES sizeof(bmcBrdTypeTbl)/sizeof(struct _BMC_BOARD_TYPE)





struct _BMC_FEATURES
{
	u_int8 mask;
	char *name;
} bmcFeatureTbl[8][8] =
{
	/* Data 1 */
	{
		{ BMC_FEATURE_D1_VREP,		"VREP" },
		{ BMC_FEATURE_D1_EVLOG,		"EVLOG" },
		{ BMC_FEATURE_D1_ERRCNT,	"ERRCNT" },
		{ BMC_FEATURE_D1_RTC,		"RTC" },
		{ BMC_FEATURE_D1_FUP,		"FUP" },
		{ BMC_FEATURE_D1_CPCI,		"CPCI" },
		{ BMC_FEATURE_D1_GPIO,		"GPIO" },
/* ToDo		{ 0x00,	"" }, */
	},
	/* Data 2 */
	{
		{ BMC_FEATURE_D2_CLUS,	"CLUS" },
	},
	/* Data 3 */
	{
		{ 0x00, "" },
	},
	/* Data 4 */
	{
		{ 0x00, "" },
	},
	/* Data 5 */
	{
		{ BMC_FEATURE_D5_HWB,	"HWB" },
		{ BMC_FEATURE_D5_RESMD,	"RESMD" },
	 	{ BMC_FEATURE_D5_EPFMD,	"EPFMD" },
		{ BMC_FEATURE_D5_RSIMD,	"RSIMD" },
		{ BMC_FEATURE_D5_SRTCR,	"SRTCR" },
	},
	/* Data 6 */
	{
		{ 0x00, "" },
	},
	/* Data 7 */
	{
		{ 0x00, "" },
	},
	/* Data 8 */
	{
		{ 0x00, "" },
	},
};
#define BMC_FEATURE_NBR		8
#define BMC_NBR_OF_FEATURES	sizeof(bmcFeatureTbl)/sizeof(struct _BMC_FEATURES)



#  ifdef __cplusplus
      }
#  endif

#endif /*_BMC_API_H */


