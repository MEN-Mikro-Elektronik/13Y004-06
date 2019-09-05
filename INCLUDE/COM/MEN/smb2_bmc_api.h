/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  smb2_bmc_api.h
 *
 *      \author  andreas.werner@men.de
 *
 *     \project  SMB2 BMC
 *       \brief  SMB2 BMC API interface routines
 *
 *    \switches  -
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
 * You should have received a copy of the GNU Lesser General License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SMB2_BMC_API_H
# define _SMB2_BMB_API_H

#ifdef __cplusplus
	extern "C" {
#endif

#include <MEN/men_typs.h>
#include <MEN/mdis_err.h>

/*--------------------+
|   BMC Error Codes   |
+--------------------*/

#ifdef ERR_DEV
# define BMC_OFFS (ERR_DEV+0xe0)
#else
# define BMC_OFFS 0
#endif /*ERR_DEV*/

#define SMB2_BMC_ERR_NO        (0x00)            /** No Error */
#define SMB2_BMC_ERR_LENGTH    (BMC_OFFS + 0x1)  /** Wrong length */
#define SMB2_BMC_ERROR         (BMC_OFFS + 0x2)  /** Error Code 0xFF */
#define SMB2_BMC_ERR_INPUT     (BMC_OFFS + 0x3)  /** Input Error */

/*--------------------+
|   BMC Enumerations  |
+--------------------*/
enum STATUS_OUTPUT {STA = 0, HTSWP, USR1, USR2, USR3, USR4, USR5};
enum GPO {GPO_0 = 0, GPO_1, GPO_2, GPO_3, GPO_4, GPO_5, GPO_6};
enum GPI {GPI_0 = 0, GPI_1, GPI_2, GPI_3, GPI_4, GPI_5, GPI_6};

/*--------------------+
|   BMC Firmware      |
+--------------------*/

/** BMC Firmware Version Data */
struct bmc_fwversion {
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

/** BMC Features */
struct bmc_features {
	/**
	 * Feature Set FS_GPIO supported for (gpio_support != 0)
	 * Feature Set FS_GPIO not supported for (gpio_support == 0)
	 */
	u_int8 gpio_support;
	/**
	 * Feature Set FS_BACKPLANE_CPCI supported for (cpci_support != 0)
	 * Feature Set FS_BACKPLANE_CPCI not supported for (cpci_support == 0)
	 */
	u_int8 cpci_support;
	/**
	 * Feature Set FS_FIRMWARE_UPDATE supported for (fup_support != 0)
	 * Feature Set FS_FIRMWARE_UPDATE not supported for (fup_support == 0)
	 */
	u_int8 fup_support;
	/**
	 * Feature Set FS_RTC supported for (rtc_support != 0)
	 * Feature Set FS_RTC not supported for (rtc_support == 0)
	 */
	u_int8 rtc_support;
	/**
	 * Feature Set Data FS_ERROR_COUNTER supported for (errcnt_support != 0)
	 * Feature Set Data FS_ERROR_COUNTER not supported for (errcnt_support == 0)
	 */
	u_int8 errcnt_support;
	/**
	 * Feature Set FS_EVENT_LOG supported for (evlog_support != 0)
	 * Feature Set FS_EVENT_LOG not supported for (evlog_support == 0)
	 */
	u_int8 evlog_support;
	/**
	 * Feature Set FS_VOLTAGE_REPORTING supported for (vrep_support != 0)
	 * Feature Set FS_VOLTAGE_REPORTING not supported for (vrep_support == 0)
	 */
	u_int8 vrep_support;
	/**
	 * Features Set FS_CLUSTER_REPORT supported for (clus_support != 0)
	 * Features Set FS_CLUSTER_REPORT not supported for (clus_support == 0)
	 */
	u_int8 clus_support;
	/**
	 * Command SW_RTC_RESET supported for (srtcr_support != 0)
	 * Command SW_RTC_RESET not supported for (srtcr_support == 0)
	 */
	u_int8 srtcr_support;
	/**
	 * Command RESET_IN_MODE_SET/GET supported for (rsimd_support != 0)
	 * Command RESET_IN_MODE_SET/GET not supported for (rsimd_support == 0)
	 */
	u_int8 rsimd_support;
	/**
	 * Command EXT_PWR_FAIL_MODE_SET supported for (epfmd_support != 0)
	 * Command EXT_PWR_FAIL_MODE_SET not supported for (epfmd_support == 0)
	 */
	u_int8 epfmd_support;
	/**
	 * Command RESUME_MODE_SET/GET supported for (resmd_support != 0)
	 * Command RESUME_MODE_SET/GET not supported for (resmd_support == 0)
	 */
	u_int8 resmd_support;
	/**
	 * Command HW_BOARD_GET/SET supported for (hwb_support != 0)
	 * Command HW_BOARD_GET/SET not supported for (hwb_support == 0)
	 */
	u_int8 hwb_support;
};

/** BMC Reset Reason */
struct bmc_rst_reason{
	/**
	 * Processor ID that caused the event 
	 *(0=not processor specific, 1..n=processor ID)
	 */
	u_int8 procID;
	/**
	 * Event code, see BMC-API-Spec.
	 */
	u_int16 ev_code;
	/**
	 * Event code specific, additional info, see BMC-API-Spec
	 */
	u_int8 ev_info1;
	u_int8 ev_info2;
	u_int8 ev_info3;
	u_int8 ev_info4;
};

/** BMC Voltage Report */
struct bmc_voltage_report{
	/**
	 * Actual voltage value
	 */
	u_int16 actual_volt;
	/**
	 * Nominal voltage value
	 */
	u_int16 nominal_volt;
	/**
	 * Lowest measured value of this voltage since BMC power on.
	 */
	u_int16 low_lim_volt;
	/**
	 * Highest measured value of this voltage since BMC Power on.
	 */
	u_int16 up_lim_volt;
};

/** BMC Event Log Status */
struct bmc_evlog_status{
	/**
	 * Timestamps are derived from RTC for (rtcts == 1)
	 * Timestamps are derived from operation time counter for (rtcts == 0)
	 */
	u_int8 rtcts;
	/**
	 * Max. number of entries the event buffer can hold
	 */
	u_int16 max_entries;
	/**
	 * Actual number of entries in the event buffer
	 */
	u_int16 act_entries;
};

/** BMC Event Log Report */
struct bmc_event_report{
	/**
	 * If derived from RTC: time in seconds since 1.1.2000 00:00h
	 * If derived from operation timer: operation time in seconds
	 */
	u_int64 tstamp;
	/**
	 * RTC time valid for (rtcv != 0)
	 * RTC time invalid for (rtcv == 0)
	 */
	u_int8 rtcv;
	/**
	 * Processor ID that caused the event
	 * (0=not processor specific, 1..n=processor ID)
	 */
	u_int8 procID;
	/**
	 * Event code
	 */
	u_int16 ev_code;
	
	/**
	 * Event code specific, additional info
	 */
	u_int8 ev_info1;
	/**
	 * Event code specific, additional info
	 */
	u_int8 ev_info2;
	/**
	 * Event code specific, additional info
	 */
	u_int8 ev_info3;
	/**
	 * Event code specific, additional info
	 */
	u_int8 ev_info4;
};

struct bmc_rtc {
	/**
	 * Absolute year (e.g. 2011 = 0x7DB)
	 */
	u_int16 year;
	/**
	 * 1 = January, 12 = December
	 */
	u_int8 month;
	/**
	 * Day of month (1..31)
	 */
	u_int8 mday;
	/**
	 * 0..23
	 */
	u_int8 hours;
	/**
	 * 0..59
	 */
	u_int8 minutes;
	/**
	 * ,..59
	 */
	u_int8 seconds;
	/**
	 * 0 = OK, 1 = RTC stopped due to battery low
	 */
	u_int8 battery;
};

/** BMC Status Frame */
struct bmc_status_frame {
	/**
	 * Watchdog is in state WD_INIT or IDLE for (wd_itout != 0)
	 * Watchdog is in state WD_RUN for (wd_itout == 0)
	 */
	u_int8 wd_itout;
	/**
	 * MFB powered for (isw_st != 0)
	 * MFB not powered for (isw_st == 0)
	 */
	u_int8 isw_st;
	/**
	 * FPGA finished configuration loading for (fpga_rdy != 0)
	 * FPGA has not (yet) finished configuration loading for (fpga_rdy == 0)
	 */
	u_int8 fpga_rdy;
	/**
	 * Indicates the current state of the MAIN_FAIL_FSM
	 */
	u_int8 state;
	/**
	 * Indicates the highest priority failure pending
	 */
	u_int8 fault_cause;
	/**
	 * SUPVCOR major revision
	 */
	u_int8 maj_rev;
	/**
	 * SUPVCOR minor revision
	 */
	u_int8 min_rev;
	/**
	 * Overvoltage condition leads to SAFE_SHUTDOWN state for (ov_shdn != 0)
	 * Overvoltage condition leads to SAFE_DISABLED state for (ov_shdn == 0)
	 */
	u_int8 ov_shdn;
	/**
	 * FPGA monitor enabled for (fpga_mon_en != 0)
	 * FPGA monitor disabled for (fpga_mon_en == 0)
	 */
	u_int8 fpga_mon_en;
	/**
	 * SUPVCOR restarts after fatal error for (restart != 0)
	 * SUPVCOR does not restart after fatal error for (restart == 0)
	 */
	u_int8 restart;
	/**
	 * Test mode active for (test_mode != 0)
	 * Test mode inactive for (test_mode == 0)
	 */
	u_int8 test_mode;
	/**
	 * Configures nominal frequency of SS_CLK that is supervised by CLK_MON
	 */
	u_int8 nom_ss_clk;
	/**
	 * Upper limit of WDOG window
	 */
	u_int8 wd_tout_ul;
	/**
	 * Lower limit of WDOG window
	 */
	u_int8 wd_tout_ll;
	/**
	 * Configures watchdog initial timeout value
	 */
	u_int8 wd_en;
	/**
	 * Watchdog is in state WD_INIT or IDLE for (wd_itout != 0)
	 * Watchdog is in state WD_RUN for (wd_itout == 0)
	 */
	u_int8 wd_init_tout;
	/**
	 * SUPV unique ID
	 */
	u_int16 supv_id;
	/**
	 * Toggles for each new frame (life_sign_bit either == 0 or != 0)
	 */
	u_int8 life_sign_bit;
	/**
	 * Build number
	 */
	u_int8 supv_id_build_no;
	/**
	 * 7-bit CRC
	 */
	u_int8 crc;
};

extern int32 __MAPILIB SMB2BMC_Exit(void);
extern char* __MAPILIB SMB2BMC_Ident(void);
extern int32 __MAPILIB SMB2BMC_Init(char *deviceP);
extern char* __MAPILIB SMB2BMC_Errstring(u_int32 errCode, char *strBuf);
extern int32 __MAPILIB SMB2BMC_GetFirm_Ver(struct bmc_fwversion *fw_revision);
extern int32 __MAPILIB SMB2BMC_Set_HW_Brd(u_int16 s_board);
extern int32 __MAPILIB SMB2BMC_Get_HW_Brd(u_int16 *g_board);
extern int32 __MAPILIB SMB2BMC_Get_Features(struct bmc_features *features);
extern int32 __MAPILIB SMB2BMC_WDOG_enable(void);
extern int32 __MAPILIB SMB2BMC_WDOG_disable(void);
extern int32 __MAPILIB SMB2BMC_WDOG_trig(void);
extern int32 __MAPILIB SMB2BMC_WDOG_TimeSet(u_int16 wd_max_tout);
extern int32 __MAPILIB SMB2BMC_WDOG_TimeGet(u_int16 *wd_max_tout);
extern int32 __MAPILIB SMB2BMC_WDOG_GetState(u_int8 *wd_state);
extern int32 __MAPILIB SMB2BMC_WDOG_Arm(void);
extern int32 __MAPILIB SMB2BMC_WDOG_GetArmState(u_int8 *arm_state);
extern int32 __MAPILIB SMB2BMC_WDOG_MinTimeSet(u_int16 wd_min_tout);
extern int32 __MAPILIB SMB2BMC_WDOG_MinTimeGet(u_int16 *wd_min_tout);
extern int32 __MAPILIB SMB2BMC_ResumeModeSet(u_int8 res_mode);
extern int32 __MAPILIB SMB2BMC_ResumeModeGet(u_int8 *res_mode);
extern int32 __MAPILIB SMB2BMC_ExtPwrFailModeSet(u_int8 ext_pwr_fail_mode);
extern int32 __MAPILIB SMB2BMC_ExtPwrFailModeGet(u_int8 *ext_pwr_fail_mode);
extern int32 __MAPILIB SMB2BMC_ResetInModeSet(u_int8 reset_in_mode);
extern int32 __MAPILIB SMB2BMC_ResetInModeGet(u_int8 *reset_in_mode);
extern int32 __MAPILIB SMB2BMC_SW_Reset(u_int16 reset_cause);
extern int32 __MAPILIB SMB2BMC_SW_ColdReset(u_int16 reset_cause);
extern int32 __MAPILIB SMB2BMC_SW_RTC_Reset(u_int16 reset_cause);
extern int32 __MAPILIB SMB2BMC_SW_Halt(u_int16 reset_cause);
extern int32 __MAPILIB SMB2BMC_RstReasonGet(struct bmc_rst_reason *reset_reason);
extern int32 __MAPILIB SMB2BMC_RstReasonCLR(void);
extern int32 __MAPILIB SMB2BMC_Volt_Max_Num(u_int8 *volt_max_num);
extern int32 __MAPILIB SMB2BMC_Volt_Get(u_int8 volt_idx,
										struct bmc_voltage_report *volt_report);
extern int32 __MAPILIB SMB2BMC_Get_PwrCycleCnt(u_int32 *pwr_cycles);
extern int32 __MAPILIB SMB2BMC_Get_OpHoursCnt(u_int32 *op_time);
extern int32 __MAPILIB SMB2BMC_Get_EventLog_Status(struct bmc_evlog_status *evlog_stat);
extern int32 __MAPILIB SMB2BMC_Add_Event(u_int16 ev_code, u_int8 ev_info1, 
										u_int8 ev_info2, u_int8 ev_info3, u_int8 ev_info4);
extern int32 __MAPILIB SMB2BMC_EventLog_Read(u_int16 evlog_idx, 
											 struct bmc_event_report *evlog_report);
extern int32 __MAPILIB SMB2BMC_ErrCnt_MaxIDX(u_int8 *errcnt_max_idx);
extern int32 __MAPILIB SMB2BMC_ErrCnt_Clear(void);
extern int32 __MAPILIB SMB2BMC_Get_ErrCnt(u_int8 errcnt_idx, u_int16 *error_cnt);
extern int32 __MAPILIB SMB2BMC_StatusOutput_Set(enum STATUS_OUTPUT status_out, u_int8 on_off);
extern int32 __MAPILIB SMB2BMC_StatusOutput_Get(enum STATUS_OUTPUT status_out, u_int8 *status);
extern int32 __MAPILIB SMB2BMC_RTC_Set(u_int16 year, u_int8 month, u_int8 mday, 
										u_int8 hrs, u_int8 min, u_int8 sec );
extern int32 __MAPILIB SMB2BMC_RTC_Get(struct bmc_rtc *rtc);
extern int32 __MAPILIB SMB2BMC_CPCI_BrdMode(u_int8 *cpci_mode);
extern int32 __MAPILIB SMB2BMC_CPCI_SlotAddr(u_int8 *cpci_slotaddr);
extern int32 __MAPILIB SMB2BMC_GPO_Caps(enum GPO gpo, u_int8 *gpo_support);
extern int32 __MAPILIB SMB2BMC_GPO_Set(enum GPO gpo, u_int8 on_off);
extern int32 __MAPILIB SMB2BMC_GPO_Get(enum GPO gpo, u_int8 *status);
extern int32 __MAPILIB SMB2BMC_GPI_Caps(enum GPI gpi, u_int8 *gpi_support);
extern int32 __MAPILIB SMB2BMC_GPI_Get(enum GPI gpi, u_int8 *status);
extern int32 __MAPILIB SMB2BMC_PWR_GetEvLog(u_int8 *pwr_log_mode);
extern int32 __MAPILIB SMB2BMC_PWR_SetEvLog(u_int8 pwr_log_mode);
extern int32 __MAPILIB SMB2BMC_StatusFrame_trigger(void);
extern int32 __MAPILIB SMB2BMC_GetStatusFrame(struct bmc_status_frame *status_frame);

#ifdef __cplusplus
	}
#endif

#endif /*_SMB2_BMC_API_H*/

