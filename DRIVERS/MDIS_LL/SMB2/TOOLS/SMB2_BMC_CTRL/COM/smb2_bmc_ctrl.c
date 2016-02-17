/****************************************************************************
 ************                                                    ************
 ************                     SMB2_BMC_CTRL                  ************
 ************                                                    ************
 ****************************************************************************/
/**!
 *         \file smb2_bmc_ctrl.c
 *
 *       \author  quoc.bui@men.de
 *         $Date: 2015/08/14 17:53:29 $
 *     $Revision: 1.2 $
 *
 *        \brief  Tool to access the MEN BMC via the SMB2_BMC API
 *
 *      Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api, smb2_bmc_api
 *
 *--------------------------------[ History ]-------------------------------
 *
 * $Log: smb2_bmc_ctrl.c,v $
 * Revision 1.2  2015/08/14 17:53:29  awerner
 * R: Windows package does not compile due to some cast warnings.
 * M: Fixed warnings and added casts to the functions calls.
 *
 *--------------------------------------------------------------------------
 * (c) Copyright 2014 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/
static const char RCSid[]="$Id: smb2_bmc_ctrl.c,v 1.2 2015/08/14 17:53:29 awerner Exp $";

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
#pragma warning(disable:4996)
#endif

/*-------------------------------------+
|   INCLUDES                           |
+-------------------------------------*/
#include <stdio.h>

#include <MEN/smb2_bmc_api.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>

/*-------------------------------------+
|   DEFINES                            |
+-------------------------------------*/
/* BMC Management Comamnds */
#define FIRMWARE_REV		0x01
#define SET_HW_BOARD		0x02
#define GET_HW_BOARD		0x03
#define FEATURES			0x04

/* Watchdog Commands */
#define WDOG_ENABLE			0x01
#define WDOG_DISABLE		0x02
#define WDOG_TRIG			0x03
#define WDOG_TIME_SET		0x04
#define WDOG_TIME_GET		0x05
#define WDOG_STATE_GET		0x06
#define WDOG_ARM			0x07
#define WDOG_ARM_STATE		0x08
#define WDOG_MIN_TIME_SET	0x09
#define WDOG_MIN_TIME_GET	0x0A

/* Power and Reset Control Commands */
#define RESUME_MODE_SET			0x01
#define RESUME_MODE_GET			0x02
#define EXT_PWR_FAIL_MODE_SET	0x03
#define EXT_PWR_FAIL_MODE_GET	0x04
#define RESET_IN_MODE_SET		0x05
#define RESET_IN_MODE_GET		0x06
#define EXT_PS_ON_MODE_SET		0x07
#define EXT_PS_ON_MODE_GET		0x08
#define SW_RESET				0x09
#define SW_COLD_RESET			0x0a
#define SW_RTC_RESET			0x0b
#define SW_HALT					0x0c
#define PWRBTN					0x0d
#define PWRBTN_OVRD				0x0e
#define RST_REASON_GET			0x0f
#define RST_REASON_CLR			0x10

/* Event Log Commands */
#define EVLOG_STAT			0x01
#define EVLOG_WRITE			0x02
#define EVLOG_READ			0x03

/* Error Counters Commands */
#define ERR_CNT_MAX_NUM		0x01
#define ERR_CNT_CLR			0x02
#define ERR_CNT_GET			0x03

/* Status Outputs Commands */
#define STATUS_OUT_SET		0x01
#define STATUS_OUT_GET		0x02

/* RTC Commands */
#define RTC_SET				0x01
#define RTC_GET				0x02

/* Backplane CPCI Commands */
#define CPCI_BRDMODE		0x01
#define CPCI_SLOTADDR		0x02

/* GPIO Commands */
#define GPO_CAPS			0x01
#define GPO_SET				0x02
#define GPO_GET				0x03
#define GPI_CAPS			0x04
#define GPI_GET				0x05

/* Cluster Support Commands */
#define CLUS_CHAN_STATE_GET 0x01
#define CLUS_CHAN_STATE_SET 0x02

/* CB30C Specific Commands */
#define PWR_LOG_MODE_SET	0x01
#define PWR_LOG_MODE_GET	0x02
#define STAT_FRM_TRIG		0x03
#define STAT_FRM_GET		0x04

#define WDOG_ON				0x01
#define WDOG_OFF			0x00

/*-------------------------------------+
|   GLOBALS                            |
+-------------------------------------*/
u_int32 SMB2BMC_CTRL_errCount;
void *SMB2BMC_CTRL_smbHdl;

/*-------------------------------------+
|   PROTOTYPES                         |
+-------------------------------------*/
static void bmc_manage_command(int bmc_manage_cmd, int argc, char* argv[]);
static void print_firm_version(void);
static void set_hw_board(int argc, char* argv[]);
static void print_hw_board(void);
static void print_features(void);

static void wdog_command(int wd_cmd, int argc, char* argv[]);
static void wdog_on(void);
static void wdog_off(void);
static void wdog_trigger(int argc, char* argv[]);
static void wdog_time_set(int argc, char* argv[]);
static void wdog_time_get(void);
static void get_wdog_state(void);
static void arm_wdog(void);
static void get_wdog_armstate(void);
static void set_wdog_mintime(int argc, char* argv[]);
static void get_wdog_mintime(void);

static void pwr_rst_ctrl(int prc_cmd, int argc, char* argv[]);
static void set_res_mode(int argc, char* argv[]);
static void get_res_mode(void);
static void set_ExtPwrFail_mode(int argc, char* argv[]);
static void get_ExtPwrFail_mode(void);
static void set_reset_in(int argc, char* argv[]);
static void get_reset_in(void);
static void init_sw_reset(u_int16 reset_cause);
static void init_cold_reset(u_int16 reset_cause);
static void init_rtc_reset(u_int16 reset_cause);
static void init_halt(u_int16 reset_cause);
static void get_last_rst_reason(void);
static void clr_last_rst_reason(void);

static void print_voltage_report(void);

static void print_life_time_report(void);

static void event_log_command(int event_log_cmd, int argc, char* argv[]);
static void print_evlog_status(void);
static void evlog_write(int argc, char* argv[]);
static void print_event_report(int argc, char* argv[]);

static void error_cnt_command(int error_cnt_cmd, int argc, char* argv[]);
static void print_errcnt_max(void);
static void clear_errcnt(void);
static void print_errcnt(int argc, char* argv[]);

static void status_outputs_command(int status_out_cmd, int argc, char* argv[]);
static void set_status_output(int argc, char* argv[]);
static void get_status_output(void);

static void rtc_command(int rtc_cmd, int argc, char* argv[]);
static void set_rtc(int argc, char* argv[]);
static void print_rtc(void);

static void cpci_command(int cpci_cmd);
static void print_cpci_mode(void);
static void print_cpci_slotaddr(void);

static void gpio_command(int gpio_cmd, int argc, char* argv[]);
static void print_gpo_caps(void);
static void gpo_set(int argc, char* argv[]);
static void gpo_get(void);
static void print_gpi_caps(void);
static void gpi_get(void);

static void cluster_command(int cluster_cmd, int argc, char* argv[]);
static void get_cluster_state(void);
static void set_cluster_state(int argc, char* argv[]);

static void cb30c_command(int cb30c_cmd, int argc, char* argv[]);
static void set_pwr_log_mode(int argc, char* argv[]);
static void get_pwr_log_mode(void);
static void stat_frm_trigger(void);
static void print_stat_frm(void);

static void PrintError(char *info, int32 errCode);

/********************************* header ***********************************/
/**  Prints the headline
 */
static void header(void)
{
	printf( "\n====================="
		"\n=== SMB2_BMC_CTRL ==="
		"\n====================="
		"\n(c)Copyright 2015 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid);
}

/********************************* usage ************************************/
/**  Prints the program usage
 */
static void usage(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName <opts>\n"
		"Function:  Tool to access the MEN BMC via the SMB2_BMC API\n"
		"Options:\n"
		"    devName      device name e.g. smb2_1                   \n"
		"    -?           Usage                                     \n"
		"    -i           Get Shelf Controller API Identifier       \n"
		"    -b=[cmd]     Send BMC Management command               \n"
		"    -w=[cmd]     Send WDOG command							\n"
		"    -p=[cmd]     Send Power Reset command					\n"
		"    -v           Get Voltage Report  						\n"
		"    -l           Get Life Time Report  					\n"
		"    -e=[cmd]     Send Event Log command  					\n"
		"    -x=[cmd]     Send Error Counter command				\n"
		"    -s=[cmd]     Send Status Outputs command				\n"
		"    -r=[cmd]     Send RTC command							\n"
		"    -c=[cmd]     Send Backplane CPCI command				\n"
		"    -g=[cmd]     Send GPIO command							\n"
		"    -k=[cmd]     Send Cluster Support command				\n"
		"    -z=[cmd]     Send CB30C Specific command				\n"
		"Calling examples:                                          \n"
		"    Get Firmware Revision:	smb2_bmc_ctrl smb2_1 -i         \n"
		"\nNote: cmd = 0 for individual usage						\n\n"
		);
}

/********************************* usage ************************************/
/**  Print the BMC Management options.
 */
static void print_bmc_manage_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -b=[opts]\n"
		"Options:\n"
		"     1      Get BMC Firmware Revision                    \n"
		"     2      Set Hardware Board                           \n"
		"     3      Get Hardware Board                           \n"
		"     4      Get Supported Feature Sets and Options       \n"
		);
}

/********************************* usage ************************************/
/**  Prints the WDOG options.
 */
static void print_wd_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -w=[opts]\n"
		"Options:\n"
		"     1                 Enable WDOG                                           \n"
		"     2                 Disable WDOG                                          \n"
		"     3 <trigger_time>  Trigger WDOG                                          \n"
		"			Note: -trigger_time (optional) in ms; default: 30 s               \n"
		"     4  <upper_limit>  Set upper limit of trigger time window (unit: 100 ms) \n"
		"     5                 Get upper limit of trigger time window (unit: 100 ms) \n"
		"     6                 Get WDOG State						   	              \n"
		"     7                 Arm WDOG (Main CPU) and BIOS timeouts	              \n"
		"     8                 Get WDOG Arming State					              \n"
		"     9  <lower_limit>  Set lower limit of trigger time window (unit: 10 ms)  \n"
		"     a                 Get lower limit of trigger time window (unit: 10 ms)  \n"
		);
}

/********************************* usage ************************************/
/**  Prints the Power and Reset options.
 */
static void print_prc_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -p=[cmd]\n"
		"Options:\n"
		"     1  <resmd>  Set Power Resume Mode                          \n"
		"		  Note: -resmd: 0 to 2			       					 \n"
		"     2           Get Power Resume Mode                          \n"
		"     3  <epfmd>  Set External Power Supply Failure Mode         \n"
		"		  Note: -epfmd: 0 to 1		    		   				 \n"
		"     4           Get External Power Supply Failure Mode    	 \n"
		"     5  <rsimd>  Set RESET_IN Mode    							 \n"
		"		  Note: -rsimd: 0 to 1				       				 \n"
		"     6           Get RESET_IN Mode								 \n"
		"     7           Set EXT_PS_ON Mode             **Status: NA	 \n"
		"     8           Get EXT_PS_ON Mode             **Status: NA	 \n"
		"     9           Initiate Software Reset	   					 \n"
		"     a           Initiate Cold Reset	   						 \n"
		"     b           Initiate Cold Reset combined with RTC Reset	 \n"
		"     c           Initiate Halt	   								 \n"
		"     d           Perform Power Button Press     **Status: NA	 \n"
		"     e           Perform Power Button Override  **Status: NA	 \n"
		"     f           Get last reset reason	   						 \n"
		"    10           Clear last reset reason	   					 \n"
		);
}

/********************************* usage ************************************/
/**  Print Event Log options.
 */
static void print_evlog_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -e=[opts]\n"
		"Options:\n"
		"     1                                  Get Event Log Status              \n"
		"     2  <code info1 info2 info3 info4>  Add Event to Event Log            \n"
		"     3  <idx>                           Read one Event from the Event LOG \n"
		);
}

/********************************* usage ************************************/
/**  Print Error Counter options.
 */
static void print_errcnt_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -x=[opts]\n"
		"Options:\n"
		"     1       Get number of error counters supported   \n"
		"     2       Clear all Error Counters                 \n"
		"     3 <idx> Get Error Counter by Index               \n"
		);
}

/********************************* usage ************************************/
/**  Print Status Output options.
 */
static void print_staout_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -s=[opts]      \n"
		"Options:\n"
		"     1 <output 1/0>     Set status outputs        \n"
		"			 Note: -output: 0 to 6  \n"
		"     2                  Get status outputs        \n"
		);
}

/********************************* usage ************************************/
/**  Print RTC options.
 */
static void print_rtc_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -r=[opts]\n"
		"Options:\n"
		"     1 <year month day hrs min sec>     Set RTC                \n"
		"     2                                  Get RTC                \n"
		);
}

/********************************* usage ************************************/
/**  Print Backplane CPCI options.
 */
static void print_cpci_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -c=[opts]\n"
		"Options:\n"
		"     1      Get CPCI Board Mode                \n"
		"     2      Get CPCI peripheral slot address   \n"
		);	
}

/********************************* usage ************************************/
/**  Print GPIO options.
 */
static void print_gpio_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -g=[opts]\n"
		"Options:\n"
		"     1               Report which GPOs are supported    \n"
		"     2 <gpo_nbr 1/0> Set general purpose outputs        \n"
		"		      Note: -gpo_nbr: 0 to 6  \n"
		"     3               Get general purpose outputs        \n"
		"     4               Report which GPIs are supported    \n"
		"     5               Get general purpose inputs         \n"
		);	
}

/********************************* usage ************************************/
/**  Print Cluster Support options.
 */
static void print_cluster_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -k=[opts]\n"
		"Options:\n"
		"     1      Get cluster channel state       \n"
		"     2 <1>  Set cluster channel state       \n"
		);	
}

/********************************* usage ************************************/
/**  Print Cluster Support options.
 */
static void print_cb30c_specific_options(void)
{
	printf(
		"\nUsage:     smb2_bmc_ctrl devName -k=[opts]\n"
		"Options:\n"
		"     1 <1/0> Enable/disable power on/off event logging          \n"
		"     2       Get current setting of power on/off event logging  \n"
		"     3       Trigger a new SUPV Status Frame Transfer           \n"
		"     4       Get last from SUPV received status frame           \n"
		);	
}

/****************************************************************************/
/** Program main function
 *
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *
 *  \return        success (0) or error (1)
 */
int main(int argc, char** argv)
{
	int err, ret=0, i;
	char *optP;
	char *errstr;
	char argbuf[100];
	char *deviceP = NULL;
	unsigned int bmc_manage_cmd = 0;
	unsigned int wd_cmd = 0;
	unsigned int prc_cmd = 0;
	unsigned int evlog_cmd = 0;
	unsigned int errcnt_cmd = 0;
	unsigned int status_out_cmd = 0;
	unsigned int rtc_cmd = 0;
	unsigned int cpci_cmd = 0;
	unsigned int gpio_cmd = 0;
	unsigned int cluster_cmd = 0;
	unsigned int cb30c_cmd = 0;

	header();

	/*--------------------+
	|  check arguments    |
	+--------------------*/
	errstr = UTL_ILLIOPT("?ib=w=p=vle=x=s=r=c=g=k=z=", argbuf);
	if (errstr) {
		printf("*** %s\n", errstr);
		usage();
		return 1;
	}

	if (UTL_TSTOPT("?")) {
		usage();
		return 1;
	}

	/*--------------------+
	|  get arguments      |
	+--------------------*/
	for (i=1; i<argc; i++) {
		if (*argv[i] != '-') {
			if (deviceP == NULL)
				deviceP = argv[i];
		}
	}
	if (!deviceP) {
		printf("***ERROR: missing SMB device name\n");
		usage();
		return 1;
	}

	/*--------------------+
	|  Init BMC library   |
	+--------------------*/
	err = SMB2BMC_Init(deviceP);
	if (err) {
		PrintError("***ERROR: SMB2_BMC_Init", err);
		ret=1;
		goto EXIT;
	}

	/* get SMB2BMC API identifier */
	if (UTL_TSTOPT("i")){
		printf("%s\n", SMB2BMC_Ident());
	}
	
	/* send BMC Management command */
	if ((optP = UTL_TSTOPT("b="))) {
		sscanf(optP, "%x", &bmc_manage_cmd);
		bmc_manage_command(bmc_manage_cmd, argc, argv);
	}
	
	/* send WDOG command */
	if ((optP = UTL_TSTOPT("w="))) {
		sscanf(optP, "%x", &wd_cmd);
		wdog_command(wd_cmd, argc, argv);
	}
	
	/* send Power and Reset Control command */
	if ((optP = UTL_TSTOPT("p="))) {
		sscanf(optP, "%x", &prc_cmd);
		pwr_rst_ctrl(prc_cmd, argc, argv);
	}
	
	/* send Voltage Reporting command */
	if (UTL_TSTOPT("v")) {
		print_voltage_report();
	}
	
	/* send Life Time Reporting command */
	if (UTL_TSTOPT("l")) {
		print_life_time_report();
	}
	
	/* send Event Log command */
	if ((optP = UTL_TSTOPT("e="))) {
		sscanf(optP, "%x", &evlog_cmd);
		event_log_command(evlog_cmd, argc, argv);
	}
	
	/* send Error Counter command */
	if ((optP = UTL_TSTOPT("x="))) {
		sscanf(optP, "%x", &errcnt_cmd);
		error_cnt_command(errcnt_cmd, argc, argv);
	}
	
	/* send Status Outputs command */
	if ((optP = UTL_TSTOPT("s="))) {
		sscanf(optP, "%x", &status_out_cmd);
		status_outputs_command(status_out_cmd, argc, argv);
	}
	
	/* send RTC command */
	if ((optP = UTL_TSTOPT("r="))) {
		sscanf(optP, "%x", &rtc_cmd);
		rtc_command(rtc_cmd, argc, argv);
	}
	
	/* send Backplane CPCI command */
	if ((optP = UTL_TSTOPT("c="))) {
		sscanf(optP, "%x", &cpci_cmd);
		cpci_command(cpci_cmd);
	}
	
	/* send GPIO command */
	if ((optP = UTL_TSTOPT("g="))) {
		sscanf(optP, "%x", &gpio_cmd);
		gpio_command(gpio_cmd, argc, argv);
	}
	
	/* send Cluster Support command */
	if ((optP = UTL_TSTOPT("k="))) {
		sscanf(optP, "%x", &cluster_cmd);
		cluster_command(cluster_cmd, argc, argv);
	}
	
	/* send CB30C specific command */
	if ((optP = UTL_TSTOPT("z="))) {
		sscanf(optP, "%x", &cb30c_cmd);
		cb30c_command(cb30c_cmd, argc, argv);
	}
	
EXIT:
	/*--------------------+
	|  Exit BMC library   |
	+--------------------*/
	SMB2BMC_Exit();

	return ret;
}

/****************************************************************************/
/** Get and print the firmware version
*
*/
static void print_firm_version(void)
{
	int err;
	struct bmc_fwversion firm_version;

	printf("Firmware Version:\n");
	printf("-----------------------------------------\n");

	err = SMB2BMC_GetFirm_Ver(&firm_version);
	if (err) {
		PrintError("***ERROR: SMB2BMC_GetFirm_Ver:", err);
	}
	else {
		printf("BMC Firmware Revision %d.%d\n", firm_version.maj_revision,
							firm_version.min_revision);
		printf("BMC Maintenance Revision: %d\n", firm_version.mtnce_revision);
		printf("BMC Firmware Build Number: %d\n", firm_version.build_nbr);
		printf("Verified: %s\n", firm_version.veri_flag ? "TRUE" : "FALSE");
	}
}

/****************************************************************************/
/** Set the hardware board
*
*  \param argc    \IN  argument counter
*  \param argv    \IN  argument vector
*/
static void set_hw_board(int argc, char* argv[])
{
	int err;
	unsigned int s_board;
	
	if (argc > 3){
		sscanf( argv[3], "%x", &s_board );
		
		err = SMB2BMC_Set_HW_Brd((u_int16)s_board);
		if (err) {
			PrintError("***ERROR: SMB2BMC_Set_HW_Brd:", err);
		}
		else {
			printf("Hardware Board was set to 0x%x:", s_board);
			switch(s_board){
				case 0x0001: printf("Hardware Board: XM1\n"); break;
				case 0x0002: printf("Hardware Board: MM1\n"); break;
				case 0x0003: printf("Hardware Board: XM2\n"); break;
				case 0x0004: printf("Hardware Board: F19P\n"); break;
				case 0x0005: printf("Hardware Board: G20\n"); break;
				case 0x0006: printf("Hardware Board: SC21\n"); break;
				case 0x0007: printf("Hardware Board: F11S\n"); break;
				case 0x0008: printf("Hardware Board: F21P\n"); break;
				case 0x0009: printf("Hardware Board: MM2\n"); break;
				case 0x0010: printf("Hardware Board: F75P\n"); break;
				case 0x0011: printf("Hardware Board: F76P\n"); break;
				default: printf("Hardware Board not specified..\n");
			}
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get the hardware board
*
*/
static void print_hw_board(void)
{
	int err;
	u_int16 g_board;
	
	err = SMB2BMC_Get_HW_Brd(&g_board);

	if (err) {
		PrintError("***ERROR: SMB2BMC_Get_HW_Brd:", err);
	}
	else {
		switch(g_board){
			case 0x0001: printf("Hardware Board: XM1\n"); break;
			case 0x0002: printf("Hardware Board: MM1\n"); break;
			case 0x0003: printf("Hardware Board: XM2\n"); break;
			case 0x0004: printf("Hardware Board: F19P\n"); break;
			case 0x0005: printf("Hardware Board: G20\n"); break;
			case 0x0006: printf("Hardware Board: SC21\n"); break;
			case 0x0007: printf("Hardware Board: F11S\n"); break;
			case 0x0008: printf("Hardware Board: F21P\n"); break;
			case 0x0009: printf("Hardware Board: MM2\n"); break;
			case 0x0010: printf("Hardware Board: F75P\n"); break;
			case 0x0011: printf("Hardware Board: F76P\n"); break;
			default: printf("Hardware Board not specified..\n");
		}
	}
}

/****************************************************************************/
/** Get the features
*
*/
static void print_features(void)
{
	int err;
	struct bmc_features features;

	printf("Features:\n");
	printf("-----------------------------------------\n");

	err = SMB2BMC_Get_Features(&features);
	if (err) {
		PrintError("***ERROR: SMB2BMC_Get_Features:", err);
	}
	else {
		printf("GPIO  : Features Set FS_GPIO \t\t\t%s\n", 
				features.gpio_support ? "supported" : "not supported");
		printf("CPCI  : Features Set FS_BACKPLANE_CPCI \t\t%s\n", 
				features.cpci_support ? "supported" : "not supported");
		printf("FUP   : Features Set FS_FIRMWARE_UPDATE \t%s\n", 
				features.fup_support ? "supported" : "not supported");
		printf("RTC   : Features Set FS_RTC \t\t\t%s\n", 
				features.rtc_support ? "supported" : "not supported");
		printf("ERRCNT: Features Set FS_ERROR_COUNTER \t\t%s\n", 
				features.errcnt_support ? "supported" : "not supported");
		printf("EVLOG : Features Set FS_EVENT_LOG \t\t%s\n", 
				features.evlog_support ? "supported" : "not supported");
		printf("VREP  : Features Set FS_VOLTAGE_REPORTING \t%s\n", 
				features.vrep_support ? "supported" : "not supported");
		printf("CLUS  : Features Set FS_CLUSTER_SUPPORT \t%s\n", 
				features.clus_support ? "supported" : "not supported");
		printf("SRTCR : Command SW_RTC_RESET \t\t\t%s\n", 
				features.srtcr_support ? "supported" : "not supported");
		printf("RSIMD : Command RESET_IN_MODE_SET/GET \t\t%s\n", 
				features.rsimd_support ? "supported" : "not supported");
		printf("EPFMD : Command EXT_PWR_FAIL_MODE_SET \t\t%s\n", 
				features.epfmd_support ? "supported" : "not supported");
		printf("RESMD : Command RESUME_MODE_SET/GET \t\t%s\n", 
				features.resmd_support ? "supported" : "not supported");
		printf("HWB   : Command HW_BOARD_GET/SET \t\t%s\n", 
				features.hwb_support ? "supported" : "not supported");
	}
}

/****************************************************************************/
/** Send BMC Management command.
*
*  \param bmc_manage_cmd    \IN  BMC Management command
*/
static void bmc_manage_command(int bmc_manage_cmd, int argc, char* argv[])
{	
	switch(bmc_manage_cmd){
		case 0x00:
			print_bmc_manage_options();
			break;
		case '?':
			print_bmc_manage_options();
			break;
		case FIRMWARE_REV:
			print_firm_version();
			break;
		case GET_HW_BOARD:
			print_hw_board();
			break;
		case SET_HW_BOARD:
			set_hw_board(argc, argv);
			break;
		case FEATURES:
			print_features();
			break;
		default: printf("***ERROR: Cluster Support command not available.\n");
	}
}

/****************************************************************************/
/** Enable WDOG
*
*/
static void wdog_on(void)
{
	int err;
	
	err = SMB2BMC_WDOG_enable();
	
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_enable:", err);
	}
	else {
		printf("Watchdog successfully enabled.\n");
	}
}

/****************************************************************************/
/** Disable WDOG
*
*/
static void wdog_off(void)
{
	int err;
	
	err = SMB2BMC_WDOG_disable();
	
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_disable:", err);
	}
	else {
		printf("Watchdog successfully disabled.\n");
	}
}

/****************************************************************************/
/** WDOG trigger
*
*  \param argc    \IN  argument counter
*  \param argv    \IN  argument vector
*/
static void wdog_trigger(int argc, char* argv[])
{
	int err;
	unsigned int trigTime;
	u_int8 wdog_state;
	
	if( argc > 3 ) {
		sscanf( argv[3], "%d", &trigTime );
	}
	else {
		trigTime = 30000;	/* 30sec */
	}

	/* check WDog state */
	err = SMB2BMC_WDOG_GetState(&wdog_state);
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_GetState:", err);
	}
	
	if( wdog_state == WDOG_ON ) {		/* WDOG enabled */

		printf( " Watchdog state: enabled -> trigger all %dmsec\n", trigTime );

		/* trigger loop */
		do {
			UOS_Delay( trigTime );
			err = SMB2BMC_WDOG_trig();
			if (err) {
				PrintError("***ERROR: SMB2BMC_WDOG_trig:", err);
			}
			else {
				printf( " Watchdog triggered - Press any key to abort\n" );
			}
		} while( UOS_KeyPressed() == -1 );
	}
	else {
		printf( " Watchdog state: disabled -> no trigger" );
	}

	printf( "\n" );
}

/****************************************************************************/
/** Set upper limit of trigger time window
*
*  \param argc    \IN  argument counter
*  \param argv    \IN  argument vector
*/
static void wdog_time_set(int argc, char* argv[])
{
	int err;
	unsigned int wd_max_tout;
	if (argc > 3){
		sscanf( argv[3], "%d", &wd_max_tout );
		
		err = SMB2BMC_WDOG_TimeSet((u_int16)wd_max_tout);
		if (err) {
			PrintError("***ERROR: SMB2BMC_WDOG_TimeSet:", err);
		}
		else {
			printf("Watchdog upper limit of trigger time window was\n"
				   "successfully set to %d.\n", wd_max_tout);
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get upper limit of trigger time window
*
*/
static void wdog_time_get(void)
{
	int err;
	u_int16 wd_max_tout;
	
	err = SMB2BMC_WDOG_TimeGet(&wd_max_tout);
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_TimeGet:", err);
	}
	else {
		printf("Watchdog upper limit of trigger time window is %d.\n", wd_max_tout);
	}
	
}

/****************************************************************************/
/** Get WDOG state
*
*/
static void get_wdog_state(void)
{
	int err;
	u_int8 wd_state;
	
	err = SMB2BMC_WDOG_GetState(&wd_state);
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_GetState:", err);
	}
	else {
		printf("Watchdog is %s.\n", wd_state ? "on" : "off");
	}
}

/****************************************************************************/
/** Arm WDOG (Main CPU) and BIOS timeouts.
*
*/
static void arm_wdog(void)
{
	int err;
	
	err = SMB2BMC_WDOG_Arm();
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_Arm:", err);
	}
	else {
		printf("Watchdog successfully armed.\n");
	}
}

/****************************************************************************/
/** Get WDOG arming state
*
*/
static void get_wdog_armstate(void)
{
	int err;
	u_int8 arm_state;
	
	err = SMB2BMC_WDOG_GetArmState(&arm_state);
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_GetArmState:", err);
	}
	else {
		printf("Watchdog is %s.\n", arm_state ? "armed" : "not armed");
	}
}

/****************************************************************************/
/** Set lower limit of trigger time window.
*
*  \param argc    \IN  argument counter
*  \param argv    \IN  argument vector
*/
static void set_wdog_mintime(int argc, char* argv[])
{
	int err;
	unsigned int wd_min_tout;
	
	if(argc > 3){
		sscanf(argv[3], "%d", &wd_min_tout);
		
		err = SMB2BMC_WDOG_MinTimeSet((u_int16)wd_min_tout);
		if (err) {
			PrintError("***ERROR: SMB2BMC_WDOG_MinTimeSet:", err);
		}
		else {
			printf("Watchdog lower limit of trigger time window was\n"
				   "successfully set to %d.\n", wd_min_tout);
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get lower limit of trigger time window.
*
*/
static void get_wdog_mintime(void)
{
	int err;
	u_int16 wd_min_tout;
	
	err = SMB2BMC_WDOG_MinTimeGet(&wd_min_tout);
	if (err) {
		PrintError("***ERROR: SMB2BMC_WDOG_MinTimeGet:", err);
	}
	else {
		printf("Watchdog lower limit of trigger time window is %d.\n", wd_min_tout);
	}
}
/****************************************************************************/
/** WDOG commands
*
*  \param argc    \IN  argument counter
*  \param argv    \IN  argument vector
*  \param wd_cmd    \IN  WDOG command
*/
static void wdog_command(int wd_cmd, int argc, char* argv[])
{	
	switch(wd_cmd){
		case 0x00:
			print_wd_options();
			break;
		case WDOG_ENABLE: 
			wdog_on();
			break;
		case WDOG_DISABLE: 
			wdog_off();
			break;
		case WDOG_TRIG: 
			wdog_trigger(argc, argv);
			break;
		case WDOG_TIME_SET: 
			wdog_time_set(argc, argv);
			break;
		case WDOG_TIME_GET: 
			wdog_time_get();
			break;
		case WDOG_STATE_GET: 
			get_wdog_state();
			break;
		case WDOG_ARM: 
			arm_wdog();
			break;
		case WDOG_ARM_STATE: 
			get_wdog_armstate();
			break;
		case WDOG_MIN_TIME_SET: 
			set_wdog_mintime(argc, argv);
			break;
		case WDOG_MIN_TIME_GET:
			get_wdog_mintime();
			break;
		default: printf("***ERROR: WDOG option not available.\n");
	}
}

/****************************************************************************/
/** Set Power Resume Mode.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_res_mode(int argc, char* argv[])
{
	int err;
	unsigned int res_mode;
	
	if (argc > 3){
		sscanf( argv[3], "%d", &res_mode );
		
		err = SMB2BMC_ResumeModeSet((u_int8)res_mode);
		if (err) {
			PrintError("***ERROR: SMB2BMC_ResumeModeSet:", err);
		}
		else {
			printf("Resume Mode was successfully set.\n");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get Power Resume Mode.
*
*/
static void get_res_mode(void)
{
	int err;
	u_int8 res_mode;
	
	err = SMB2BMC_ResumeModeGet(&res_mode);
	if (err) {
		PrintError("***ERROR: SMB2BMC_ResumeModeGet:", err);
	}
	else {
		switch(res_mode){
			case 0x00: printf("Resume mode is off.\n");break;
			case 0x01: printf("Resume mode is on.\n");break;
			case 0x02: printf("Resume mode is former.\n");break;
			default: printf("***ERROR: Undefined Resume mode.\n");
		}
	}
}

/****************************************************************************/
/** Set external Power Supply Failure Mode.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_ExtPwrFail_mode(int argc, char* argv[])
{
	int err;
	unsigned int ext_pwr_fail_mode;
	
	if (argc > 3){
		sscanf( argv[3], "%d", &ext_pwr_fail_mode );
		
		err = SMB2BMC_ExtPwrFailModeSet((u_int8)ext_pwr_fail_mode);
		if (err) {
			PrintError("***ERROR: SMB2BMC_ExtPwrFailModeSet:", err);
		}
		else {
			printf("External power supply failure mode was successfully set.\n");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
	
	
}

/****************************************************************************/
/** Get external Power Supply Failure Mode.
*
*/
static void get_ExtPwrFail_mode(void)
{
	int err;
	u_int8 ext_pwr_fail_mode;
	
	err = SMB2BMC_ExtPwrFailModeGet(&ext_pwr_fail_mode);
	if (err) {
		PrintError("***ERROR: SMB2BMC_ExtPwrFailModeGet:", err);
	}
	else {
		if (ext_pwr_fail_mode == 0x00){
			printf("External power supply failure mode shall be ignored.\n");
		}
		else if (ext_pwr_fail_mode == 0x01){
			printf("External power supply failure mode shall be treat as error.\n");
		}
	}
}
/****************************************************************************/
/** Set RESET_IN Mode.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_reset_in(int argc, char* argv[])
{
	int err;
	unsigned int reset_in_mode;
	
	if (argc > 3){
		sscanf( argv[3], "%d", &reset_in_mode );
		
		err = SMB2BMC_ResetInModeSet((u_int8)reset_in_mode);
		if (err) {
			PrintError("***ERROR: SMB2BMC_ResetInModeSet:", err);
		}
		else {
			printf("RESET_IN mode was successfully set.\n");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
	
	
}

/****************************************************************************/
/** Get RESET_IN Mode.
*
*/
static void get_reset_in(void)
{
	int err;
	u_int8 reset_in_mode;
	
	err = SMB2BMC_ResetInModeGet(&reset_in_mode);
	if (err) {
		PrintError("***ERROR: SMB2BMC_ResetInModeGet:", err);
	}
	else {
		printf("Resets are %s.\n", reset_in_mode ? "enabled" : "masked");
	}
}

/****************************************************************************/
/** Set EXT_PS_ON Mode.
*
*/

/****************************************************************************/
/** Get EXT_PS_ON Mode.
*
*/

/****************************************************************************/
/** Initiate Software Reset
*
*  \param reset_cause    \IN  reset_cause
*/
static void init_sw_reset(u_int16 reset_cause)
{
	int err;
	
	err = SMB2BMC_SW_Reset(reset_cause);
	if (err) {
		PrintError("***ERROR: SMB2BMC_SW_Reset:", err);
	}
	else {
		printf("Software reset was successfully initiated.\n");
	}
}

/****************************************************************************/
/** Initiate Cold Reset
*
*  \param reset_cause    \IN  reset_cause
*/
static void init_cold_reset(u_int16 reset_cause)
{
	int err;
	
	err = SMB2BMC_SW_ColdReset(reset_cause);
	if (err) {
		PrintError("***ERROR: SMB2BMC_SW_ColdReset:", err);
	}
	else {
		printf("Cold reset was successfully initiated.\n");
	}
}

/****************************************************************************/
/** Initiate Cold Reset combined with RTC Reset.
*
*  \param reset_cause    \IN  reset_cause
*/
static void init_rtc_reset(u_int16 reset_cause)
{
	int err;
	
	err = SMB2BMC_SW_RTC_Reset(reset_cause);
	if (err) {
		PrintError("***ERROR: SMB2BMC_SW_RTC_Reset:", err);
	}
	else {
		printf("Cold reset with rtc reset was successfully initiated.\n");
	}
}

/****************************************************************************/
/** Initiate Halt.
*
*  \param reset_cause    \IN  reset_cause
*/
static void init_halt(u_int16 reset_cause)
{
	int err;
	
	err = SMB2BMC_SW_Halt(reset_cause);
	if (err) {
		PrintError("***ERROR: SMB2BMC_SW_Halt:", err);
	}
	else {
		printf("Halt successfully initiated.\n");
	}
}

/****************************************************************************/
/** Perform Power Button Press.
*
*/

/****************************************************************************/
/** Perform Power Button Override.
*
*/

/****************************************************************************/
/** Get last reset reason.
*
*/
static void get_last_rst_reason(void)
{
	int err;
	struct bmc_rst_reason reset_reason;
	
	err = SMB2BMC_RstReasonGet(&reset_reason);
	if (err) {
		PrintError("***ERROR: SMB2BMC_RstReasonGet:", err);
	}
	else {
		printf("Processor ID: %d.\n", reset_reason.procID);
		printf("Event Code: 0x%04x.\n", reset_reason.ev_code);
		printf("EV_INFO_1: %d\n", reset_reason.ev_info1);
		printf("EV_INFO_2: %d\n", reset_reason.ev_info2);
		printf("EV_INFO_3: %d\n", reset_reason.ev_info3);
		printf("EV_INFO_4: %d\n", reset_reason.ev_info4);
	}
}

/****************************************************************************/
/** Clear last reset reason.
*
*/
static void clr_last_rst_reason(void)
{
	int err;
	
	err = SMB2BMC_RstReasonCLR();
	if (err) {
		PrintError("***ERROR: SMB2BMC_RstReasonCLR:", err);
	}
	else {
		printf("Last reset reason was successfully cleared.\n");
	}
}

/****************************************************************************/
/** Power and Reset Control
*
*  \param prc_cmd    \IN  Power and Reset Control command
*/
static void pwr_rst_ctrl(int prc_cmd, int argc, char* argv[])
{
	u_int16 reset_cause = 0xbeef;
	
	switch(prc_cmd){
		case 0x00:
			print_prc_options();
			break;
		case RESUME_MODE_SET:
			set_res_mode(argc, argv);
			break;
		case RESUME_MODE_GET: 
			get_res_mode();
			break;
		case EXT_PWR_FAIL_MODE_SET: 
			set_ExtPwrFail_mode(argc, argv);
			break;
		case EXT_PWR_FAIL_MODE_GET: 
			get_ExtPwrFail_mode();
			break;
		case RESET_IN_MODE_SET: 
			set_reset_in(argc, argv);
			break;
		case RESET_IN_MODE_GET: 
			get_reset_in();
			break;
		case SW_RESET: 
			init_sw_reset(reset_cause);
			break;
		case SW_COLD_RESET: 
			init_cold_reset(reset_cause);
			break;
		case SW_RTC_RESET: 
			init_rtc_reset(reset_cause);
			break;
		case SW_HALT:
			init_halt(reset_cause);
			break;
		case RST_REASON_GET:
			get_last_rst_reason();
			break;
		case RST_REASON_CLR:
			clr_last_rst_reason();
			break;
		default: printf("***ERROR: Power Reset Control option not available.\n");
	}
}

/****************************************************************************/
/** Prints actual value and configured value of one voltage.
*
*/
static void print_voltage_report(void)
{
	int err;
	u_int8 volt_max_num;
	u_int8 volt_idx;
	struct bmc_voltage_report volt_report;
	
	err = SMB2BMC_Volt_Max_Num(&volt_max_num);
	if (err) {
		PrintError("***ERROR: SMB2BMC_Volt_Max_Num:", err);
	}
	else {
		for (volt_idx=0; volt_idx < volt_max_num; volt_idx++){
			err = SMB2BMC_Volt_Get(volt_idx, &volt_report);
			if (err) {
				PrintError("***ERROR: SMB2BMC_Volt_Get:", err);
			}
			else {
				printf("\nVoltage %d\n", volt_idx + 1);
				printf("-----------------------------------------------------\n");
				printf("Actual voltage value is %d.\n", volt_report.actual_volt);
				printf("Nominal voltage value is %d.\n", volt_report.nominal_volt);
				printf("Lowest measured value of this voltage since BMC power on is %d.\n", 
						volt_report.low_lim_volt);
				printf("Highest measured value of this voltage since BMC power on is %d.\n",
						volt_report.up_lim_volt);
				printf("Number of voltages: %d.\n", volt_max_num);
			}
			
		}
	}
}

/****************************************************************************/
/** Send Life Time Reporting command.
*
*/
static void print_life_time_report(void)
{	
	int err;
	u_int32 pwr_cycles;
	u_int32 op_time;
	
	err = SMB2BMC_Get_PwrCycleCnt(&pwr_cycles);
	if (err) {
		PrintError("***ERROR: SMB2BMC_Get_PwrCycleCnt:", err);
	}
	else {
		printf("Power cycles on the external power supply: %u\n", (unsigned int)pwr_cycles);
	}
	
	err = SMB2BMC_Get_OpHoursCnt(&op_time);
	if (err) {
		PrintError("***ERROR: SMB2BMC_Get_OpHoursCnt:", err);
	}
	else {
		printf("Operation time: %u hours\n", (unsigned int)op_time);
	}
}

/****************************************************************************/
/** Print event log status.
*
*/
static void print_evlog_status(void)
{
	int err;
	struct bmc_evlog_status evlog_stat;
	
	err = SMB2BMC_Get_EventLog_Status(&evlog_stat);
	if (err) {
		PrintError("***ERROR: SMB2BMC_Get_EventLog_Status:", err);
	}
	else {
		printf("Event Log Status\n");
		printf("------------------------------------------------\n");
		printf("Timestamps are derived from %s.\n", 
				evlog_stat.rtcts ? "RTC" : "operation time counter");
		printf("Max. number of entries: %d.\n", evlog_stat.max_entries);
		printf("Actual number of entries: %d.\n", evlog_stat.act_entries);
	}
}

/****************************************************************************/
/** Add event to event log.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void evlog_write(int argc, char* argv[])
{
	int err;
	unsigned int ev_code = 0;
	unsigned int ev_info1 = 0;
	unsigned int ev_info2 = 0;
	unsigned int ev_info3 = 0;
	unsigned int ev_info4 = 0;
	
	if (argc > 7){
		sscanf( argv[3], "%d", &ev_code );
		sscanf( argv[4], "%d", &ev_info1 );
		sscanf( argv[5], "%d", &ev_info2 );
		sscanf( argv[6], "%d", &ev_info3 );
		sscanf( argv[7], "%d", &ev_info4 );
		
		err = SMB2BMC_Add_Event((u_int16)ev_code, (u_int8)ev_info1, (u_int8)ev_info2, 
								(u_int8)ev_info3, (u_int8)ev_info4);
		if (err) {
			PrintError("***ERROR: SMB2BMC_Add_Event:", err);
		}
		else {
			printf("Event was successfully added.\n");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
	
}

/****************************************************************************/
/** Print Event Report.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void print_event_report(int argc, char* argv[])
{
	int err;
	unsigned int evlog_idx;
	struct bmc_event_report event_report;
	
	if (argc > 3){
		sscanf( argv[3], "%d", &evlog_idx );

		err = SMB2BMC_EventLog_Read((u_int16)evlog_idx, &event_report);
		if (err) {
			PrintError("***ERROR: SMB2BMC_EventLog_Read:", err);
		}
		else {
			printf("Event Report\n");
			printf("------------------------------------------------\n");
			printf("Event Log Index: %d\n", evlog_idx);
			printf("Timestamp: %llu seconds.\n", event_report.tstamp);
			printf("RTC time is %s\n", event_report.rtcv ? "valid." : "invalid!");
			if(event_report.procID == 0){
				printf("Not processor specific.\n");
			}
			else {
				printf("Processor ID: %d\n", event_report.procID);
			}
			printf("Event Code: %d.\n", event_report.ev_code);
			printf("Event Info_1: %d.\n", event_report.ev_info1);
			printf("Event Info_2: %d.\n", event_report.ev_info2);
			printf("Event Info_3: %d.\n", event_report.ev_info3);
			printf("Event Info_4: %d.\n", event_report.ev_info4);
		}
	}

	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Send Event Log command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param event_log_cmd    \IN  Event Log command
*/
static void event_log_command(int event_log_cmd, int argc, char* argv[])
{	
	switch(event_log_cmd){
		case 0x00:
			print_evlog_options();
			break;
		case EVLOG_STAT:
			print_evlog_status();
			break;
		case EVLOG_WRITE: 
			evlog_write(argc, argv);
			break;
		case EVLOG_READ: 
			print_event_report(argc, argv);
			break;
		default: printf("***ERROR: Event Log command not available.\n");
	}
}

/****************************************************************************/
/** Get number of error counters supported by this BMC.
*
*/
static void print_errcnt_max(void)
{
	int err;
	u_int8 errcnt_max_idx;
	
	err = SMB2BMC_ErrCnt_MaxIDX(&errcnt_max_idx);
	if (err) {
		PrintError("***ERROR: SMB2BMC_ErrCnt_MaxIDX:", err);
	}
	else {
		if (errcnt_max_idx == 0x00){
			printf("No error counters supported.\n");
		}
		else {
			printf("Number of error counters supported: %d\n", errcnt_max_idx);
		}
	}
}

/****************************************************************************/
/** Clear all Error Counters.
*
*/
static void clear_errcnt(void)
{
	int err;
	
	err = SMB2BMC_ErrCnt_Clear();
	if (err) {
		PrintError("***ERROR: SMB2BMC_ErrCnt_Clear:", err);
	}
	else {
		printf("Error Counters were successfully cleared.\n");
	}
}

/****************************************************************************/
/** Get Error Counter by Index.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void print_errcnt(int argc, char* argv[])
{
	int err;
	unsigned int errcnt_idx;
	u_int16 error_cnt;
	
	if (argc > 3){
		sscanf(argv[3], "%d", &errcnt_idx);
		
		err = SMB2BMC_Get_ErrCnt((u_int8)errcnt_idx, &error_cnt);
		if (err) {
			PrintError("***ERROR: SMB2BMC_Get_ErrCnt:", err);
		}
		else {
			printf("Error Counter Report\n");
			printf("------------------------------------------------\n");
			printf("Error Counter Index: %d\n", errcnt_idx);
			printf("Number of errors occurred: %d\n", error_cnt);
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
	
}

/****************************************************************************/
/** Send Error Counter command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param error_cnt_cmd    \IN  Error Counter command
*/
static void error_cnt_command(int error_cnt_cmd, int argc, char* argv[])
{	
	switch(error_cnt_cmd){
		case 0x00:
			print_errcnt_options();
			break;
		case '?':
			print_errcnt_options();
			break;
		case ERR_CNT_MAX_NUM:
			print_errcnt_max();
			break;
		case ERR_CNT_CLR: 
			clear_errcnt();
			break;
		case ERR_CNT_GET: 
			print_errcnt(argc, argv);
			break;
		default: printf("***ERROR: Error Counter command not available.\n");
	}
}

/****************************************************************************/
/** Set status outputs.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_status_output(int argc, char* argv[])
{
	int err;
	unsigned int status_output;
	unsigned int on_off;
	
	if (argc > 4) {
		sscanf(argv[3], "%d", &status_output);
		sscanf(argv[4], "%d", &on_off);
		
		err = SMB2BMC_StatusOutput_Set((enum STATUS_OUTPUT)status_output, (u_int8)on_off);
		if (err) {
			PrintError("***ERROR: SMB2BMC_StatusOutput_Set:", err);
		}
		else {
			switch(status_output){
				case STA:
					printf("Status LED turned %s\n", on_off ? "on" : "off");
					break;
				case HTSWP:
					printf("Hot Swap LED turned %s\n", on_off ? "on" : "off");
					break;
				case USR1:
					printf("User Output %d %s\n", USR1, on_off ? "enabled" : "disabled");
					break;
				case USR2:
					printf("User Output %d %s\n", USR2, on_off ? "enabled" : "disabled");
					break;
				case USR3:
					printf("User Output %d %s\n", USR3, on_off ? "enabled" : "disabled");
					break;
				case USR4:
					printf("User Output %d %s\n", USR4, on_off ? "enabled" : "disabled");
					break;
				case USR5:
					printf("User Output %d %s\n", USR5, on_off ? "enabled" : "disabled");
					break;
				default: printf("***ERROR: Status Output not available.\n");
			}
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get status outputs.
*
*/
static void get_status_output(void)
{
	int err, i;
	u_int8 status;
	
	printf("Status Outputs Report\n");
	printf("------------------------------------------------\n");
	for(i=0; i < 7; i++){
		err = SMB2BMC_StatusOutput_Get(i, &status);
		if (err) {
			PrintError("***ERROR: SMB2BMC_StatusOutput_Get:", err);
		}
		else{
			switch(i){
				case STA:
					printf("Status LED is %s\n", status ? "on" : "off");
					break;
				case HTSWP:
					printf("Hot Swap LED is %s\n", status ? "on" : "off");
					break;
				case USR1:
					printf("User Output %d is %s\n", USR1, status ? "enabled" : "disabled");
					break;
				case USR2:
					printf("User Output %d is %s\n", USR2, status ? "enabled" : "disabled");
					break;
				case USR3:
					printf("User Output %d is %s\n", USR3, status ? "enabled" : "disabled");
					break;
				case USR4:
					printf("User Output %d is %s\n", USR4, status ? "enabled" : "disabled");
					break;
				case USR5:
					printf("User Output %d is %s\n", USR5, status ? "enabled" : "disabled");
					break;
				default: printf("***ERROR: Status Output not available.\n");
			}
		}
	}
}

/****************************************************************************/
/** Send Status Output command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param status_out_cmd    \IN  Status Outputs command
*/
static void status_outputs_command(int status_out_cmd, int argc, char* argv[])
{	
	switch(status_out_cmd){
		case 0x00:
			print_staout_options();
			break;
		case '?':
			print_staout_options();
			break;
		case STATUS_OUT_SET:
			set_status_output(argc, argv);
			break;
		case STATUS_OUT_GET:
			get_status_output();
			break;
		default: printf("***ERROR: Status Outputs command not available.\n");
	}
}

/****************************************************************************/
/** Set RTC.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_rtc(int argc, char* argv[])
{
	int err;
	unsigned int year = 0;
	unsigned int month = 0;
	unsigned int mday = 0;
	unsigned int hrs = 0;
	unsigned int min = 0;
	unsigned int sec = 0;
	
	if (argc > 8){
		sscanf( argv[3], "%d", &year );
		sscanf( argv[4], "%d", &month );
		sscanf( argv[5], "%d", &mday );
		sscanf( argv[6], "%d", &hrs );
		sscanf( argv[7], "%d", &min );
		sscanf( argv[7], "%d", &sec );
		
		err = SMB2BMC_RTC_Set((u_int16)year, (u_int8)month, (u_int8)mday, 
								(u_int8)hrs, (u_int8)min, (u_int8) sec);
		if (err) {
			PrintError("***ERROR: SMB2BMC_RTC_Set:", err);
		}
		else {
			printf("RTC was set to: %d-%d-%d, %d:%d:%d.\n", year, month, mday,
															hrs, min, sec);
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get RTC.
*
*/
static void print_rtc(void)
{
	int err;
	struct bmc_rtc rtc;

	err = SMB2BMC_RTC_Get(&rtc);
	if (err) {
		PrintError("***ERROR: SMB2BMC_RTC_Get:", err);
	}
	else {
		printf("Current Time: %d-%d-%d, %d:%d:%d\n", rtc.year, rtc.month, rtc.mday, 
												rtc.hours, rtc.minutes, rtc.seconds);
	}
}

/****************************************************************************/
/** Send RTC command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param rtc_cmd    \IN  RTC command
*/
static void rtc_command(int rtc_cmd, int argc, char* argv[])
{	
	switch(rtc_cmd){
		case 0x00:
			print_rtc_options();
			break;
		case '?':
			print_rtc_options();
			break;
		case RTC_SET:
			set_rtc(argc, argv);
			break;
		case RTC_GET:
			print_rtc();
			break;
		default: printf("***ERROR: Status Outputs command not available.\n");
	}
}

/****************************************************************************/
/** Get CPCI Board Mode.
*
*/
static void print_cpci_mode(void)
{
	int err;
	u_int8 cpci_mode;

	err = SMB2BMC_CPCI_BrdMode(&cpci_mode);
	if (err) {
		PrintError("***ERROR: SMB2BMC_CPCI_BrdMode:", err);
	}
	else {
		printf("CPCI Board Mode: %s\n", cpci_mode ? "system slot" : "peripheral");
	}
}

/****************************************************************************/
/** Get CPCI peripheral slot address.
*
*/
static void print_cpci_slotaddr(void)
{
	int err;
	u_int8 cpci_slotaddr;

	err = SMB2BMC_CPCI_SlotAddr(&cpci_slotaddr);
	if (err) {
		PrintError("***ERROR: SMB2BMC_CPCI_SlotAddr:", err);
	}
	else {
		printf("CPCI Slot Address: %d\n", cpci_slotaddr);
	}
}

/****************************************************************************/
/** Send Backplane CPCI command.
*
 *  \param cpci_cmd    \IN  Backplane CPCI command
*/
static void cpci_command(int cpci_cmd)
{	
	switch(cpci_cmd){
		case 0x00:
			print_cpci_options();
			break;
		case '?':
			print_cpci_options();
			break;
		case CPCI_BRDMODE:
			print_cpci_mode();
			break;
		case CPCI_SLOTADDR:
			print_cpci_slotaddr();
			break;
		default: printf("***ERROR: Backplane CPCI command not available.\n");
	}
}

/****************************************************************************/
/** Report which GPOs are supported.
*
*/
static void print_gpo_caps(void)
{
	int err, i;
	u_int8 gpo_support;

	for(i=GPO_0; i<=GPO_6; i++){
		err = SMB2BMC_GPO_Caps(i, &gpo_support);
		if (err) {
			PrintError("***ERROR: SMB2BMC_GPO_Caps:", err);
		}
		else {
			printf("GPO_%d is %s\n", i, gpo_support ? "supported" : "not supported");
		}
	}
}

/****************************************************************************/
/** Set general purpose outputs.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void gpo_set(int argc, char* argv[])
{
	int err;
	unsigned int gpo_set;
	unsigned int on_off;
	
	if (argc > 4) {
		sscanf(argv[3], "%d", &gpo_set);
		sscanf(argv[4], "%d", &on_off);
		
		err = SMB2BMC_GPO_Set((enum GPO)gpo_set, (u_int8)on_off);
		if (err) {
			PrintError("***ERROR: SMB2BMC_GPO_Set:", err);
		}
		else {
			printf("GPO_%d turned %s\n", gpo_set, on_off ? "on" : "off");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get general purpose outputs.
*
*/
static void gpo_get(void)
{
	int err, i;
	u_int8 status;

	for (i=GPO_0; i<=GPO_6; i++){
		err = SMB2BMC_GPO_Get(i, &status);
		if (err) {
			PrintError("***ERROR: SMB2BMC_GPO_Get:", err);
		}
		else {
			printf("GPO_%d is %s\n", i, status ? "HIGH" : "LOW");
		}
	}
}

/****************************************************************************/
/** Report which GPIs are supported.
*
*/
static void print_gpi_caps(void)
{
	int err, i;
	u_int8 gpi_support;

	for (i=GPI_0; i<=GPI_6; i++){
		err = SMB2BMC_GPI_Caps(i, &gpi_support);
		if (err) {
			PrintError("***ERROR: SMB2BMC_GPI_Caps:", err);
		}
		else {
			printf("GPI_%d is %s\n", i, gpi_support ? "supported" : "not supported");
		}
	}
}

/****************************************************************************/
/** Get general purpose inputs.
*
*/
static void gpi_get(void)
{
	int err, i;
	u_int8 status;

	for (i=GPI_0; i<=GPI_6; i++){
		err = SMB2BMC_GPI_Get(i, &status);
		if (err) {
			PrintError("***ERROR: SMB2BMC_GPI_Get:", err);
		}
		else {
			printf("GPI_%d is %s\n", i, status ? "HIGH" : "LOW");
		}
	}
}

/****************************************************************************/
/** Send GPIO command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param gpio_cmd    \IN  GPIO command
*/
static void gpio_command(int gpio_cmd, int argc, char* argv[])
{	
	switch(gpio_cmd){
		case 0x00:
			print_gpio_options();
			break;
		case '?':
			print_gpio_options();
			break;
		case GPO_CAPS:
			print_gpo_caps();
			break;
		case GPO_SET:
			gpo_set(argc, argv);
			break;
		case GPO_GET:
			gpo_get();
			break;
		case GPI_CAPS:
			print_gpi_caps();
			break;
		case GPI_GET:
			gpi_get();
			break;
		default: printf("***ERROR: Backplane CPCI command not available.\n");
	}
}

/****************************************************************************/
/** Get cluster channel state.
*
*/
static void get_cluster_state(void)
{
	int err;
	u_int8 cluster_state;
	u_int8 link_state;

	err = SMB2BMC_ClusterCh_GetState(&cluster_state);
	if (err) {
		PrintError("***ERROR: SMB2BMC_ClusterCh_GetState:", err);
	}
	else {
		link_state = cluster_state & 0x80;

		switch(cluster_state){
			case 0x00:
				printf("Link State: %s\n", link_state ? "Link fail" : "Link OK");
				printf("Cluster Channel State: Disabled\n");
				break;
			case 0x01:
				printf("Link State: %s\n", link_state ? "Link fail" : "Link OK");
				printf("Cluster Channel State: Work-By\n");
				break;
			case 0x02:
				printf("Link State: %s\n", link_state ? "Link fail" : "Link OK");
				printf("Cluster Channel State: Active\n");
				break;
			default: 
				printf("Link State: %s\n", link_state ? "Link fail" : "Link OK");
				printf("***ERROR: Undefined Cluster Channel State.\n");
		}
	}
}

/****************************************************************************/
/** Set cluster channel state.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_cluster_state(int argc, char* argv[])
{
	int err;
	unsigned int cluster_state;

	if(argc > 3){
		sscanf( argv[3], "%d", &cluster_state);
		
		if(cluster_state == 1){
			err = SMB2BMC_ClusterCh_SetState((u_int8)cluster_state);
			if (err) {
				PrintError("***ERROR: SMB2BMC_ClusterCh_GetState:", err);
			}
			else {
				printf("Cluster Channel State was successfully set.\n");
			}
		}
		else {
			printf( " Illegal chan. state : Only Work-by (0x01) allowed\n" );
			printf( " Info : Other channel states cannot be commanded through this command.\n" );
			printf( "      \"Disabled\" state can be commanded indirectly via the Fs_power_reset_control.\n" );
			printf( "      \"Active\" state can be only entered if the BMC detects the other channel as disabled.\n" );
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Send Cluster Support command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param cluster_cmd    \IN  Cluster command
*/
static void cluster_command(int cluster_cmd, int argc, char* argv[])
{	
	switch(cluster_cmd){
		case 0x00:
			print_cluster_options();
			break;
		case '?':
			print_cluster_options();
			break;
		case CLUS_CHAN_STATE_GET:
			get_cluster_state();
			break;
		case CLUS_CHAN_STATE_SET:
			set_cluster_state(argc, argv);
			break;
		default: printf("***ERROR: Cluster Support command not available.\n");
	}
}

/****************************************************************************/
/** Enable/disable power on/off event logging.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
*/
static void set_pwr_log_mode(int argc, char* argv[])
{
	int err;
	unsigned int pwr_log_mode;

	if(argc > 3){
		sscanf( argv[3], "%d", &pwr_log_mode);
		
		if ((pwr_log_mode == 1) || (pwr_log_mode == 0)){
			err = SMB2BMC_PWR_SetEvLog((u_int8)pwr_log_mode);
			if (err) {
				PrintError("***ERROR: SMB2BMC_PWR_SetEvLog:", err);
			}
			else {
				printf("Logging of power on/off events %s.\n", 
						pwr_log_mode ? "enabled" : "disabled");
			}
		}
		else {
			printf("***ERROR: Invalid value\n");
		}
	}
	else {
		printf("***ERROR: Not enough arguments\n");
	}
}

/****************************************************************************/
/** Get current setting of power on/off event logging.
*
*/
static void get_pwr_log_mode(void)
{
	int err;
	u_int8 pwr_log_mode;

	err = SMB2BMC_PWR_GetEvLog(&pwr_log_mode);
	if (err) {
		PrintError("***ERROR: SMB2BMC_PWR_GetEvLog:", err);
	}
	else {
		printf("Logging of power on/off events %s.\n", pwr_log_mode ? "enabled" : "disabled");
	}
}

/****************************************************************************/
/** Trigger a new SUPV Status Frame Transfer.
*
*/
static void stat_frm_trigger(void)
{
	int err;

	err = SMB2BMC_StatusFrame_trigger();
	if (err) {
		PrintError("***ERROR: SMB2BMC_StatusFrame_trigger:", err);
	}
	else {
		printf("New SUPV Status Frame Transfer was triggered.\n");
	}
}


/****************************************************************************/
/** Get and print the SUPV Status Frame
*
*/
static void print_stat_frm(void)
{
	int err;
	struct bmc_status_frame status_frame;
	unsigned int wdw_hi = 0;

	printf("SUPV Status Frame:\n");
	printf("-----------------------------------------\n");

	err = SMB2BMC_GetStatusFrame(&status_frame);
	if (err) {
		PrintError("***ERROR: SMB2BMC_GetStatusFrame:", err);
	}
	else {
		printf("wd_itout            = 0b%d: Watchdog is in state %s.\n",
				status_frame.wd_itout ? 1 : 0,
				status_frame.wd_itout ? "WD_INIT or IDLE" : "WD_RUN");
		printf("isw_st              = 0b%d: MFB (Monitored Functional Block) is %s.\n",
				status_frame.isw_st ? 1 : 0,
				status_frame.isw_st ? "powered" : "not powered");
		printf("fpga_rdy            = 0b%d: FPGA %s configuration loading.\n",
				status_frame.fpga_rdy ? 1 : 0,
				status_frame.fpga_rdy ? "finished" : "has not finished");
		printf("main_fail_fsm_state = 0x%x: MAIN_FAIL_FSM current state: ",
				status_frame.state);
		switch(status_frame.state) {
			case 0x0: printf("SAFE_INITIAL\n"); break;
			case 0x1: printf("EXT_ON\n"); break;
			case 0x2: printf("PULSE_PWRBTN\n"); break;
			case 0x3: printf("LOC_PUP\n"); break;
			case 0x4: printf("RUN\n"); break;
			case 0x5: printf("ST_RESUME\n"); break;
			case 0x6: printf("SAFE_SHUTDOWN\n"); break;
			case 0x7: printf("SAFE_DISABLED\n"); break;
			case 0x8: printf("SAFE_DISABLED_RESET\n"); break;
			default: printf("***ERROR: Unknown state\n");
		}
		printf("fault_cause         = 0x%x: ",
				status_frame.fault_cause);
		switch(status_frame.fault_cause) {
			case 0x00:
				printf("SUPVCOR has not yet detected an error.\n"); 
				break;
			case 0x01:
				printf("SUPV detected under-voltage on PWR_IN\n"); 
				break;
			case 0x02:
				printf("SUPV detected overvoltage on PWR_IN\n"); 
				break;
			case 0x03: 
				printf("SUPV detected incorrect clock on SS_CLK\n"); 
				break;
			case 0x04: 
				printf("SUPV detected local power supply overvoltage\n"); 
				break;
			case 0x05: 
				printf("Thermal sensor signaled ""too hot""\n"); 
				break;
			case 0x06:
				printf("Thermal sensor signaled ""too cold""\n");
				break;
			case 0x07:
				printf("SUPV detected a failure PWR_CTRL SW_MON feedback signals\n");
				break;
			case 0x08:
				printf("SUPV Watchdog asserted failure\n");
				break;
			case 0x09:
				printf("SUPV detected failure during FPGA configuration load\n"
					   "or during operation\n");
				break;
			case 0x0A:
				printf("SUPV detected internal failure\n"); 
				break;
			case 0x0B:
				printf("SUPV detected stuck CASC_IN signal\n"); 
				break;
			case 0x0C:
				printf("SUPV detected asserted ERR_P[0] signal\n"); 
				break;
			case 0x0D:
				printf("SUPV detected asserted ERR_P[1] signal\n"); 
				break;
			case 0x0E:
				printf("SUPV detected asserted ERR_N[0] signal\n"); 
				break;
			case 0x0F: 
				printf("SUPV detected asserted ERR_N[1] signal\n"); 
				break;
			case 0x10:
				printf("SUPV detected missing power via ISW_ST\n");
				break;
			case 0x11: 
				printf("SUPV detected local power supply under-voltage\n");
				break;
			case 0x12: 
				printf("SUPV received a command to transit to SAFE_INIT state\n");
				break;
			default: printf("***ERROR: Undefined fault");
		}
		printf("SUPVCOR Revision %d.%d\n", status_frame.maj_rev, status_frame.min_rev);
		printf("ov_shdn             = 0b%d: Overvoltage condition leads to %s state\n",
				status_frame.ov_shdn ? 1 : 0,
				status_frame.ov_shdn ? "SAFE_SHUTDOWN" : "SAFE_DISABLED");
		printf("fpga_mon_en         = 0b%d: FPGA monitor %s.\n",
				status_frame.fpga_mon_en ? 1 : 0,
				status_frame.fpga_mon_en ? "enabled" : "disabled");
		printf("restart             = 0b%d: SUPVCOR %s after fatal error.\n", 
				status_frame.restart ? 1 : 0,
				status_frame.restart ? "restarts" : "does not restart");
		printf("test_mode           = 0b%d: Test mode %s\n",
				status_frame.test_mode ? 1 : 0,
				status_frame.test_mode ? "active" : "inactive");
		printf("nom_ss_clk          = 0x%x: Nominal frequency of SS_CLK: ",
				status_frame.nom_ss_clk);
		switch(status_frame.nom_ss_clk) {
			case 0x0: printf("25 MHz\n"); break;
			case 0x1: printf("32 MHz\n"); break;
			case 0x2: printf("33.333 MHz\n"); break;
			case 0x3: printf("50 MHz\n"); break;
			case 0x4: printf("66.667 MHz\n"); break;
			case 0x5: printf("83.333 MHz\n"); break;
			case 0x6: printf("133.333 MHz\n"); break;
			case 0x7: printf("100 MHz\n"); break;
			default: printf("***ERROR: Undefined value\n");
		}
		
		switch(status_frame.wd_tout_ul) {
			case 0x0: wdw_hi = 20; break;
			case 0x1: wdw_hi = 40; break;
			case 0x2: wdw_hi = 80; break;
			case 0x3: wdw_hi = 160; break;
			case 0x4: wdw_hi = 320; break;
			case 0x5: wdw_hi = 640; break;
			case 0x6: wdw_hi = 1280; break;
			case 0x7: wdw_hi = 2560; break;
			default: printf("***ERROR: Undefined value\n");
		}
		printf("wd_tout_u           = 0x%x: Upper limit of WDOG window: %d ms\n",
				status_frame.wd_tout_ul, wdw_hi);
		printf("wd_tout_l           = 0b%d: Lower limit of WDOG window: %d ms\n",
				status_frame.wd_tout_ll ? 1 : 0,
				status_frame.wd_tout_ll ? (wdw_hi/4) : 0);
		printf("wd_en               = 0b%d: Watchdog is %s\n", 
				status_frame.wd_en ? 1 : 0,
				status_frame.wd_en ? "enabled and running" : "not running");
		printf("wd_init_tout        = 0x%x: Watchdog initial timeout value: ",
				status_frame.wd_init_tout);
		switch(status_frame.wd_init_tout) {
			case 0x0: printf("15 s\n"); break;
			case 0x1: printf("60 s\n"); break;
			case 0x2: printf("120 s\n"); break;
			case 0x3: printf("300 s\n"); break;
			default: printf("***ERROR: Undefined value\n");
		}
		printf("SUPV ID     : 	0x%x\n", status_frame.supv_id);
		printf("Build number: 	%d\n", status_frame.supv_id_build_no);
		printf("CRC         :	0x%x\n", status_frame.crc);
	}
}

/****************************************************************************/
/** Send CB30C specific command.
*
 *  \param argc    \IN  argument counter
 *  \param argv    \IN  argument vector
 *  \param cluster_cmd    \IN  Cluster command
*/
static void cb30c_command(int cb30c_cmd, int argc, char* argv[])
{	
	switch(cb30c_cmd){
		case 0x00:
			print_cb30c_specific_options();
			break;
		case '?':
			print_cb30c_specific_options();
			break;
		case PWR_LOG_MODE_SET:
			set_pwr_log_mode(argc, argv);
			break;
		case PWR_LOG_MODE_GET:
			get_pwr_log_mode();
			break;
		case STAT_FRM_TRIG:
			stat_frm_trigger();
			break;
		case STAT_FRM_GET:
			print_stat_frm();
			break;
		default: printf("***ERROR: CB30C Specific command not available.\n");
	}
}

/******************************** PrintError ********************************/
/** Routine to print SMB2BMC/MDIS error message
 *
 *  \param info       \IN  info string
 *  \param errCode    \IN  error code number
 */
static void PrintError(char *info, int32 errCode)
{
	static char errMsg[512];

	if (!errCode)
		errCode = UOS_ErrnoGet();

	printf("%s: %s\n", info, SMB2BMC_Errstring(errCode, errMsg));
}




