/****************************************************************************
 ************                                                    ************
 ************                     SMB2_SHC_CTRL                  ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file smb2_shc_ctrl.c
 *
 *       \author  quoc.bui@men.de
 *         $Date: 2015/02/24 17:26:39 $
 *     $Revision: 1.2 $
 *
 *        \brief  Tool to access the Shelf Controller via the SMB2_SHC API
 *
 *      Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api, smb2_shc
 *
 *--------------------------------[ History ]-------------------------------
 *
 * $Log: smb2_shc_ctrl.c,v $
 * Revision 1.2  2015/02/24 17:26:39  MRoth
 * R: 1. temperature calculation delivers wrong values
 *    2. cosmetics
 * M: 1. changed temperature variables to u_int16
 *    2. revised code
 *
 *--------------------------------------------------------------------------
 * (c) Copyright 2014 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
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
static const char RCSid[]="$Id: smb2_shc_ctrl.c,v 1.2 2015/02/24 17:26:39 MRoth Exp $";

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
#pragma warning(disable:4996)
#endif

/*-------------------------------------+
|   INCLUDES                           |
+-------------------------------------*/
#include <stdio.h>

#include <MEN/smb2_shc.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>

/*-------------------------------------+
|   DEFINES                            |
+-------------------------------------*/
#define ABS_ZERO    273    /* calculate Celsius from Kelvin */

/*-------------------------------------+
|   GLOBALS                            |
+-------------------------------------*/
u_int32 SMB2CTRL_errCount;
void *SMB2CTRL_smbHdl;

/*-------------------------------------+
|   PROTOTYPES                         |
+-------------------------------------*/
static void print_psu(int32 psu_id);
static void print_fan(int32 fan_id);
static void print_voltlevel(void);
static void print_ups(int32 ups_id);
static void shut_down(void);
static void power_off(void);
static void print_configdata(void);
static void get_psu_state(u_int32 psu_nbr);
static void get_ups_state(u_int32 ups_nbr);
static void get_fan_state(u_int32 fan_nbr);
static void print_firm_version(void);
static void PrintError(char *info, int32 errCode);


/********************************* header ***********************************/
/**  Prints the headline
 */
static void header(void)
{
	printf( "\n====================="
			"\n=== SMB2_SHC_CTRL ==="
			"\n====================="
			"\n(c)Copyright 2014 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid);
}

/********************************* usage ************************************/
/**  Prints the program usage
 */
static void usage(void)
{
	printf(
		"\nUsage:     smb2_shc_ctrl devName <opts>\n"
		"Function:  Tool to access the Shelf Controller via the SMB2_SHC API\n"
		"Options:\n"
		"    devName      device name e.g. smb2_1                   \n"
		"    -?           Usage                                     \n"
		"    -t           Get Temperature                           \n"
		"    -i           Get shelf controller API identifier       \n"
		"    -p=[psu_id]  Get status of one or all PSU              \n"
		"                 (psu_id: 0 to 3, 0: to get all psu status)\n"
		"    -f=[fan_id]  Get status of one or all FAN              \n"
		"                 (fan_id: 0 to 3, 0: to get all fan status)\n"
		"    -u=[ups_id]  Get status of one or all UPS              \n"
		"                 (ups_id: 0 to 2, 0: to get all ups status)\n"
		"    -v           Get input voltage level                   \n"
		"    -s           Indicates that CPU is shutting down       \n"
		"    -o           Indicates that CPU wants to shut          \n"
		"                 off the power supply                      \n"
		"    -c           Get Configuration Data                    \n"
		"    -r           Get Firmware Version                      \n"
		"Calling examples:                                          \n"
		"    Get FAN status:  smb2_shc_ctrl smb2_1 -f=0             \n"
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
	u_int32 id_nbr=0;
	u_int16 tempK;
	u_int16 tempC;

	header();

	/*--------------------+
	|  check arguments    |
	+--------------------*/
	errstr = UTL_ILLIOPT("?cf=op=irstu=v", argbuf);
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
	|  Init SHC library   |
	+--------------------*/
	err = SMB2SHC_Init(deviceP);
	if (err) {
		PrintError("***ERROR: SMB2_SHC_Init", err);
		ret=1;
		goto EXIT;
	}

	/* get fan status */
	if ((optP = UTL_TSTOPT("f="))) {
		sscanf(optP, "%x", &id_nbr);
		get_fan_state(id_nbr);
	}

	/* get shelf controller API identifier */
	if (UTL_TSTOPT("i"))
		printf("%s\n", SMB2SHC_Ident());

	/* get temperature */
	if (UTL_TSTOPT("t")) {
		err = SMB2SHC_GetTemperature((u_int16*)&tempK);
		if (err) {
			PrintError("***ERROR: SMB2SHC_GET_TEMP", err);
			goto EXIT;
		}
		tempC = tempK - ABS_ZERO;
		printf("Temperature: %dK (%dC)\n", tempK, tempC);
	}

	/* get voltage levels */
	if ((optP = UTL_TSTOPT("v")))
		print_voltlevel();

	/* get ups status */
	if ((optP = UTL_TSTOPT("u="))) {
		sscanf(optP, "%x", &id_nbr);
		get_ups_state(id_nbr);
	}

	/* indicates that the CPU is shutting down */
	if ((optP = UTL_TSTOPT("s")))
		shut_down();

	/* indicates that the CPU wants to shut off the power supply */
	if ((optP = UTL_TSTOPT("o")))
		power_off();

	/* get configuration data */
	if ((optP = UTL_TSTOPT("c")))
		print_configdata();

	/* get psu status */
	if ((optP = UTL_TSTOPT("p="))) {
		sscanf(optP, "%x", &id_nbr);
		get_psu_state(id_nbr);
	}

	/* get firmware version */
	if ((optP = UTL_TSTOPT("r")))
		print_firm_version();

EXIT:
	/*--------------------+
	|  Exit SHC library   |
	+--------------------*/
	SMB2SHC_Exit();

	return ret;
}


/****************************************************************************/
/** Get and print the state of PSU
 *
 *  \param psu_id    \IN  ID number of the PSU
 */
static void print_psu(int32 psu_id)
{
	int err;
	struct shc_psu shc_psu_state;

	err = SMB2SHC_GetPSU_State(psu_id, &shc_psu_state);
	if (err) {
		PrintError("***ERROR: SMB2SHC_GET_PSU:", err);
	}
	else {
		printf("-----------------------------------------\n");
		printf("PSU %d Present:\t\t%s\n", psu_id + 1,
				shc_psu_state.isPresent ? "TRUE" : "FALSE");
		printf("PSU %d Failure:\t\t%s\n", psu_id + 1,
				shc_psu_state.intFailure ? "TRUE" : "FALSE");
		printf("PSU %d External PWR:\t%s\n", psu_id +1,
				shc_psu_state.isEPwrPresent ? "TRUE" : "FALSE");
	}
}

/****************************************************************************/
/** Get and print the state of FAN
 *
 *  \param fan_id    \IN  ID number of the FAN
 */
static void print_fan(int32 fan_id)
{
	int err;
	struct shc_fan shc_fan_state;

	err = SMB2SHC_GetFAN_State(fan_id, &shc_fan_state);
	if (err) {
		PrintError("***ERROR: SMB2SHC_GET_FAN:", err);
	}
	else {
		printf("-----------------------------------------\n");
		printf("FAN %d Present:\t\t%s\n", fan_id + 1,
				shc_fan_state.isPresent ? "TRUE" : "FALSE");
		printf("FAN %d Status:\t\t%s\n", fan_id + 1,
				shc_fan_state.state == SHC_FAN_OK ? "OK" : "FAIL");
		printf("FAN %d Speed (rpm):\t%d\n", fan_id + 1,
				shc_fan_state.speedRpm);
	}
}

/****************************************************************************/
/** Get and print the voltage level
 */
static void print_voltlevel()
{
	int i, err, pwr_mon_nr;
	u_int16 voltlevel;

	printf("Voltage Level Reporting:\n");
	printf("-----------------------------------------\n");

	for (i=SHC_PWR_MON_1; i<=SHC_PWR_MON_4; i++) {
		err = SMB2SHC_GetVoltLevel(i, &voltlevel);
		if (err) {
			PrintError("***ERROR: SMB2SHC_GET_VOLT_LEVEL:", err);
			continue;
		}
		pwr_mon_nr = i + 1;
		printf("PWR_MON %d Voltage Level in mV:\t\t%d\n", pwr_mon_nr, voltlevel);
	}
}

/****************************************************************************/
/** Get and print the state of UPS
*
*  \param ups_id    \IN  ID number of the UPS
*/
static void print_ups(int32 ups_id)
{
	int err;
	struct shc_ups shc_ups_state;

	err = SMB2SHC_GetUPS_State(ups_id, &shc_ups_state);
	if (err) {
		PrintError("***ERROR: SMB2SHC_GET_UPS:", err);
	}
	else {
		printf("-----------------------------------------\n");
		printf("UPS %d Present:\t\t%s\n", ups_id + 1,
			shc_ups_state.isPresent ? "TRUE" : "FALSE");
		printf("UPS %d Failure:\t\t%s\n", ups_id + 1,
			shc_ups_state.intFailure ? "TRUE" : "FALSE");
		printf("UPS %d provides PWR:\t%s\n", ups_id + 1,
			shc_ups_state.provPWR ? "TRUE" : "FALSE");
		printf("UPS %d charging level in %%: %d\n", ups_id + 1, shc_ups_state.chrg_lvl);
	}
}

/****************************************************************************/
/** Shut down CPU
*/
static void shut_down()
{
	int err;

	printf("CPU is shutting down now.\n");

	err = SMB2SHC_ShutDown();
	if (err) {
		PrintError("***ERROR: SMB2SHC_SHUT_DOWN:", err);
	}
}

/****************************************************************************/
/** Shut off power supply
*/
static void power_off()
{
	int err;

	printf("Shutting off power supply now.\n");

	err = SMB2SHC_PowerOff();
	if (err) {
		PrintError("***ERROR: SMB2SHC_POWER_OFF:", err);
	}
}

/****************************************************************************/
/** Print the slot status of configuration data
*
*  \param slot_status    \IN  status of the slot
*  \param slot_nbr       \IN  slot number
*/
static void print_slot_status(u_int8 slot_status, int32 slot_nbr)
{
	if (slot_status == SHC_SLOT_EMPTY)
		printf("Slot %d is empty.\n", slot_nbr);
	else if (slot_status == SHC_SLOT_PSU)
		printf("Slot %d contains PSU.\n", slot_nbr);
	else if (slot_status == SHC_SLOT_UPS)
		printf("Slot %d contains UPS.\n", slot_nbr);
	else
		printf("Slot status not defined.\n");
}

/****************************************************************************/
/** Get and print the configuration data
*/
static void print_configdata()
{
	int err;
	struct shc_configdata cdata;

	printf("Configuration Data Reporting:\n");
	printf("-----------------------------------------\n");

	err = SMB2SHC_GetConf_Data(&cdata);
	if (err) {
		PrintError("***ERROR: SMB2SHC_GetConf_Data:", err);
	}
	else {
		print_slot_status(cdata.pwrSlot2, 2);
		print_slot_status(cdata.pwrSlot3, 3);
		printf("-----------------------------------------\n");
		printf("UPS Minimum Start Charge in %%: \t\t%d\n", cdata.upsMinStartCharge);
		printf("UPS Minimum Run Charge in %%: \t\t%d\n", cdata.upsMinRunCharge);
		printf("-----------------------------------------\n");
		printf("Low Temperature Warning Limit in K: \t%d\n", cdata.tempWarnLow);
		printf("Upper Temperature Warning Limit in K: \t%d\n", cdata.tempWarnHigh);
		printf("Low Temperature Run Limit in K: \t%d\n", cdata.tempRunLow);
		printf("High Temperature Run Limit in K: \t%d\n", cdata.tempRunHigh);
		printf("-----------------------------------------\n");
		printf("Number of Fans: %d\n", cdata.fanNum);
		printf("Fan Min Duty Cycle in %%: \t\t%d\n", cdata.fanDuCyMin);
		printf("Fan Start Temperature in K: \t\t%d\n", cdata.fanTempStart);
		printf("Fan Max Speed Temperature in K: \t%d\n", cdata.fanTempMax);
		printf("-----------------------------------------\n");
		printf("SHC State Machine ID: \t%d\n", cdata.StateMachineID);
	}
}

/****************************************************************************/
/** Get the state of one or all PSU
*
*  \param psu_nbr    \IN  entered number for PSU selection
*/
static void get_psu_state(u_int32 psu_nbr)
{
	int i;

	printf("Power Supply Reporting:\n");

	if (psu_nbr == 0) {
		for (i = SHC_PSU1; i <= SHC_PSU3; i++) {
			print_psu(i);
		}
	}
	else
		print_psu(psu_nbr - 1);
}

/****************************************************************************/
/** Get the state of one or all UPS
*
*  \param ups_nbr    \IN  entered number for UPS selection
*/
static void get_ups_state(u_int32 ups_nbr)
{
	int i;

	printf("Uninterruptible Power Supply Reporting:\n");

	if (ups_nbr == 0) {
		for (i=SHC_UPS1; i<=SHC_UPS2; i++) {
			print_ups(i);
		}
	}
	else
		print_ups(ups_nbr - 1);
}

/****************************************************************************/
/** Get the state of one or all FAN
*
*  \param fan_nbr    \IN  entered number for FAN selection
*/
static void get_fan_state(u_int32 fan_nbr)
{
	int i;

	printf("Fan Status Reporting:\n");

	if (fan_nbr == 0) {
		for (i=SHC_FAN1; i<=SHC_FAN3; i++) {
			print_fan(i);
		}
	}
	else
		print_fan(fan_nbr - 1);
}

/****************************************************************************/
/** Get and print the firmware version
*
*/
static void print_firm_version()
{
	int err;
	struct shc_fwversion firm_version;

	printf("Firmware Version:\n");
	printf("-----------------------------------------\n");

	err = SMB2SHC_GetFirm_Ver(&firm_version);
	if (err) {
		PrintError("***ERROR: SMB2SHC_GetFirm_Ver:", err);
	}
	else {
		printf("Error Code: %s\n", firm_version.errcode ? "ERROR" : "OK");
		printf("BMC Firmware Revision %d.%.02d\n", firm_version.maj_revision,
													firm_version.min_revision);
		printf("BMC Maintenance Revision: %d\n", firm_version.mtnce_revision);
		printf("BMC Firmware Build Number: %d\n", firm_version.build_nbr);
		printf("Verified: %s\n", firm_version.veri_flag ? "TRUE" : "FALSE");
	}
}

/******************************** PrintError ********************************/
/** Routine to print SMB2SHC/MDIS error message
 *
 *  \param info       \IN  info string
 *  \param errCode    \IN  error code number
 */
static void PrintError(char *info, int32 errCode)
{
	static char errMsg[512];

	if (!errCode)
		errCode = UOS_ErrnoGet();

	printf("%s: %s\n", info, SMB2SHC_Errstring(errCode, errMsg));
}

