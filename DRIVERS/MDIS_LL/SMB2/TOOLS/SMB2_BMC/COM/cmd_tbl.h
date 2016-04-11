/*********************** P r o g r a m  -  M o d u l e ***********************/
/*!
 *         \file  cmd_tbl.h
 *       \author  dieter.pfeuffer@men.de
 *        $Date: 2014/01/07 17:38:03 $
 *    $Revision: 1.1 $
 *
 *      \project  SMB2_BMC tool
 *        \brief  Command table for dispatching
 *     \switches  none
 *
 *--------------------------------[ History ]---------------------------------
 *
 * $Log: cmd_tbl.h,v $
 * Revision 1.1  2014/01/07 17:38:03  MRoth
 * Initial Revision
 *
 *----------------------------------------------------------------------------
 * (c) Copyright 2013 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 *****************************************************************************/

#ifndef CMD_TBL_H_
#define CMD_TBL_H_

#ifdef __cplusplus
	extern "C" {
#endif

/*--------------------------------------+
|    PROTOTYPES                         |
+--------------------------------------*/
extern int32 GetFirmwareRevision(int argc, char* argv[]);
extern int32 HWBoardGet(int argc, char* argv[]);
extern int32 GetHWBoard(int argc, char* argv[]);
extern int32 SetHWBoard(int argc, char* argv[]);
extern int32 Features(int argc, char* argv[]);
extern int32 WDog_On(int argc, char* argv[]);
extern int32 WDog_Off(int argc, char* argv[]);
extern int32 WDog_trigger(int argc, char* argv[]);
extern int32 WDog_time_get(int argc, char* argv[]);
extern int32 WDog_time_set(int argc, char* argv[]);
extern int32 WDog_state_get(int argc, char* argv[]);
extern int32 WDog_arm(int argc, char* argv[]);
extern int32 WDog_arm_state(int argc, char* argv[]);
extern int32 WDog_min_time_set(int argc, char* argv[]);
extern int32 WDog_min_time_get(int argc, char* argv[]);
extern int32 Ext_pwr_fail_mode_set(int argc, char* argv[]);
extern int32 Ext_pwr_fail_mode_get(int argc, char* argv[]);
extern int32 Reset_in_mode_set(int argc, char* argv[]);
extern int32 Reset_in_mode_get(int argc, char* argv[]);
extern int32 SW_reset(int argc, char* argv[]);
extern int32 SW_cold_reset(int argc, char* argv[]);
extern int32 SW_rtc_reset(int argc, char* argv[]);
extern int32 SW_halt(int argc, char* argv[]);
extern int32 Rst_reason_get(int argc, char* argv[]);
extern int32 Rst_reason_clr(int argc, char* argv[]);
extern int32 Volt_max_idx(int argc, char* argv[]);
extern int32 Volt_set_idx(int argc, char* argv[]);
extern int32 Volt_get(int argc, char* argv[]);
extern int32 Volt_get_all(int argc, char* argv[]);
extern int32 Pwrcycl_cnt(int argc, char* argv[]);
extern int32 Op_hrs_cnt(int argc, char* argv[]);
extern int32 Evlog_stat(int argc, char* argv[]);
extern int32 Evlog_write(int argc, char* argv[]);
extern int32 Evlog_write_idx(int argc, char* argv[]);
extern int32 Evlog_read(int argc, char* argv[]);
extern int32 Status_output_get(int argc, char* argv[]);
extern int32 Status_output_set(int argc, char* argv[]);
extern int32 Cluster_chan_state_get(int argc, char* argv[]);
extern int32 Cluster_chan_state_set(int argc, char* argv[]);

/*--------------------------------------+
|    TYPEDEFS                           |
+--------------------------------------*/
/* command table for dispatching */
struct _COMMAND_TABLE
{
	char *command;
	int32 (*Service)(int argc, char* argv[]);
} commands[] =
{	/* command		function */

	/* FS_BMC_MANAGEMENT */
	"gfr",			GetFirmwareRevision,
	"ghwb",			GetHWBoard,
	"shwb",			SetHWBoard,
	"feat",			Features,

	/* FS_WATCHDOG */
	"wdo",			WDog_On,
	"wdf",			WDog_Off,
	"wdt",			WDog_trigger,
	"wdtg",			WDog_time_get,
	"wdts",			WDog_time_set,
	"wdsg",			WDog_state_get,
	"wdam",			WDog_arm,
	"wdas",			WDog_arm_state,
	"wdmts",		WDog_min_time_set,
	"wdmtg",		WDog_min_time_get,

	/* FS_POWER_RESET_CONTROL */
	"epfms",		Ext_pwr_fail_mode_set,
	"epfmg",		Ext_pwr_fail_mode_get,
	"rims",			Reset_in_mode_set,
	"rimg",			Reset_in_mode_get,
	"swr",			SW_reset,
	"swcr",			SW_cold_reset,
	"swrtcr",		SW_rtc_reset,
	"swh",			SW_halt,
	"rstrg",		Rst_reason_get,
	"rstcl",		Rst_reason_clr,

	/* FS_VOLTAGE_REPORTING */
	"vmidx",		Volt_max_idx,
	"vsidx",		Volt_set_idx,
	"vget",			Volt_get,
	"vgeta",		Volt_get_all,

	/* FS_LIFETIME_REPORTING */
	"pcc",			Pwrcycl_cnt,
	"ohc",			Op_hrs_cnt,

	/* FS_EVENT_LOG */
	"evls",			Evlog_stat,
	"evlw",			Evlog_write,
	"evlwidx",		Evlog_write_idx,
	"evlr",			Evlog_read,

	/* FS_STATUS_OUTPUTS */
	"sog",			Status_output_get,
	"sos",			Status_output_set,

	/* FS_CLUSTER_SUPPORT */
	"csg",			Cluster_chan_state_get,
	"css",			Cluster_chan_state_set,

};

/*--------------------------------------+
|    DEFINES                            |
+--------------------------------------*/
/* number of commands */
#define NUMBER_OF_COMMANDS\
	sizeof(commands)/sizeof(struct _COMMAND_TABLE)

#ifdef __cplusplus
	}
#endif

#endif /* CMD_TBL_H_ */

 
