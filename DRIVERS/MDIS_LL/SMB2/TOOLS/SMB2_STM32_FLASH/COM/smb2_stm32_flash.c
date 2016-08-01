/****************************************************************************
************                                                     ************
************                   SMB2_STM32_FLASH                  ************
************                                                     ************
*****************************************************************************/
/*!
*         \file  smb2_stm32_flash.c
*
*       \author  quoc.bui@men.de
*         $Date: 2015/02/24 17:26:45 $
*     $Revision: 1.1 $
*
*        \brief  Tool to flash the STM32 Microcontroller via the SMB2_API.
*
*      Required: libraries: mdis_api, usr_oss, usr_utl, smb2_api
*
*--------------------------------[ History ]---------------------------------
*
* $Log: smb2_stm32_flash.c,v $
* Revision 1.1  2015/02/24 17:26:45  MRoth
* Initial Revision
*
*----------------------------------------------------------------------------
* (c) Copyright 2014 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
*****************************************************************************/
static const char RCSid[] = "$Id: smb2_stm32_flash.c,v 1.1 2015/02/24 17:26:45 MRoth Exp $";

/* still using deprecated sscanf, sprintf,.. */
#ifdef WINNT
#pragma warning(disable:4996)
#endif

/*-------------------------------------+
|   INCLUDES                           |
+-------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/smb2_api.h>

/*-------------------------------------+
|   DEFINES                            |
+-------------------------------------*/
#define MAX_ZEICHEN          32
#define DATA_OFFSET          293

/* bootloader command codes */
#define BTL_SETADDR_OPCODE   0x10
#define BTL_GETADDR_OPCODE   0x20
#define BTL_READ_OPCODE      0x30
#define BTL_WRITE_OPCODE     0x40
#define BTL_ERASE_OPCODE     0x50
#define BTL_LASTOP_OPCODE    0x60
#define BTL_LVEBTL_OPCODE    0x70
#define BTL_GETPSIZE_OPCODE  0x80
#define BTL_BOOTVER_OPCODE   0x90

#define ENTER_BTL_OPCODE     0x63

#define STM32_SMBFLAGS       0x00
#define CRC_BTL_SMBADDR      0xea
#define FW_SMBADDR           0xea

#define MEMORY_ADDR_LENGTH   0x04
#define BOOTLD_VER_LENGTH    0x07

#define BTL_ACK              0x79
#define BTL_NACK             0x1F

#define BYTE0                0x03
#define BYTE1                0x0C
#define BYTE2                0x30
#define BYTE3                0xC0

/*-------------------------------------+
|   MACROS                             |
+-------------------------------------*/
/* for dfu CRC check */
#define _crc(accum,delta) \
((accum) = _crctbl[((accum) ^ (delta)) & 0xff] ^ ((accum) >> 8))

/*-------------------------------------+
|   GLOBALS                            |
+-------------------------------------*/
/* SMB2 Handle */
void *SMB2BTL_smbHdl;

/* CRC table */
unsigned long _crctbl[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d };

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static int32 dfu_check(FILE *dfu_fileP);
static int32 get_dfu_data(FILE *dfu_fileP, u_int8 *start_address, u_int32 *data_size);
static int32 set_address(u_int8 *start_address);
static int32 get_address(u_int8 *tmp_address);
static int32 read_command(void);
static int32 write_command(u_int8 byte_count, u_int8 *data);
static int32 erase_command(u_int8 page_count, u_int16 page_size);
static int32 check_last_op(void);
static int32 enter_bootloader(void);
static int32 leave_bootloader(void);
static int32 get_page_size(u_int16 *page_size);
static int32 download_fw(FILE *dfu_fileP);
static int32 print_bootld_version(void);
static void PrintError(char *info, int32 errCode);


/********************************** header **********************************/
/**  Prints the headline
*/
static void header(void)
{
	printf(
		"\n=================================================="
		"\n===  MEN STM32F042XX I2C firmware update tool  ==="
		"\n=================================================="
		"\n(c)Copyright 2015 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid
	);
}

/********************************** usage ***********************************/
/**  Prints the program usage
*/
static void usage(void)
{
	printf(
		"\nUsage:     smb2_stm32_flash  devName  <opts>\n"
		"Function:  Tool to flash the STM32 Microcontroller via the SMB2_API\n"
		"Options:\n"
		"    devName    device name e.g. smb2_1     \n"
		"    -f <file>  Download firmware           \n"
		"    -e         Enter Bootloader            \n"
		"    -l         Leave Bootloader            \n"
		"    -r         Get Bootloader version      \n"
		"\nAttention: Firmware must be a .dfu file! \n"
		"\nCalling examples: \n"
		"Download a firmware file: smb2_stm32_flash smb2_2 -f 14AF02-00.dfu \n"
		"\n(c)Copyright 2015 by MEN Mikro Elektronik GmbH\n%s\n\n", RCSid
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
	FILE *dfu_fileP;
	int i, ret=0, err=0;
	char *errstr;
	char *deviceP = NULL;
	char argbuf[100];

	header();

	/*--------------------+
	|  check arguments    |
	+--------------------*/
	errstr = UTL_ILLIOPT("?elfr", argbuf);
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
	|  get SMBus device   |
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
	|  init library       |
	+--------------------*/
	err = SMB2API_Init(deviceP, &SMB2BTL_smbHdl);
	if (err) {
		PrintError("***ERROR: SMB2API_Init", err);
		ret = 1;
		goto EXIT;
	}

	/*--------------------+
	|  enter bootloader   |
	+--------------------*/
	if (UTL_TSTOPT("e")) {
		err = enter_bootloader();
		if (err)
			goto CLEANUP;
	}

	/*--------------------+
	|  leave bootloader   |
	+--------------------*/
	if (UTL_TSTOPT("l")) {
		err = leave_bootloader();
		if (err)
			goto CLEANUP;
	}

	/*--------------------+
	|  download firmware  |
	+--------------------*/
	if (UTL_TSTOPT("f")) {
		if (argc < 4) {
			printf("Not enough arguments.\n");
			goto CLEANUP;
		}
		if ((dfu_fileP = fopen(argv[3], "rb")) == NULL) {
			printf("***ERROR: cannot open %s file\n", argv[2]);
			goto CLEANUP;
		}
		err = download_fw(dfu_fileP);

		printf("-------------------------------------------------------\n");

		if (err) {
			printf("Download firmware failed.\n");
			goto CLEANUP;
		}
		else
			printf("Download firmware passed.\n");
	}

	/*---------------------+
	|  Bootloader version  |
	+---------------------*/
	if (UTL_TSTOPT("r"))
		print_bootld_version();

CLEANUP:
	/*--------------------+
	|  exit library       |
	+--------------------*/
	if ((err = SMB2API_Exit(&SMB2BTL_smbHdl))) {
		PrintError("***ERROR: SMB2API_Exit", err);
		ret = 1;
	}

EXIT:
	return ret;
}


/****************************************************************************/
/** Check DFU file
*
*  \param dfu_fileP    \IN    pointer to dfu file
*
*  \return             success (0) or error (-1)
*/
static int32 dfu_check(FILE *dfu_fileP)
{
	u_int8  buffer[100];
	u_int32 filecrc, crc_check;
	int i;

	/* readout signature from file and compare it */
	memset(buffer, '\0', sizeof(buffer));
	if (fread(buffer, 1, 5, dfu_fileP) != 5) {
		printf("***ERROR: DFU_CHECK: error in fread\n");
		return -1;
	}
	printf("%s\n", buffer);
	if (strncmp((char *)buffer, "DfuSe", 5)) {
		printf("***ERROR: firmware file not supported\n");
		return -1;
	}

	memset(buffer, '\0', sizeof(buffer));
	fseek(dfu_fileP, 11, SEEK_SET);
	if (fread(buffer, 1, 6, dfu_fileP) != 6) {
		printf("***ERROR: DFU_CHECK: error in fread\n");
		return -1;
	}
	printf("%s\n", buffer);
	if (strncmp((char *)buffer, "Target", 6)) {
		printf("***ERROR: firmware file not supported\n");
		return -1;
	}

	memset(buffer, '\0', sizeof(buffer));
	fseek(dfu_fileP, -8, SEEK_END);
	if (fread(buffer, 1, 3, dfu_fileP) != 3) {
		printf("***ERROR: DFU_CHECK: error in fread\n");
		return -1;
	}
	printf("%s\n", buffer);
	if (strncmp((char *)buffer, "UFD", 3)) {
		printf("***ERROR: firmware file not supported\n");
		return -1;
	}

	/* compute the CRC up to the last 4 bytes */
	fseek(dfu_fileP, -4, SEEK_END);
	i = ftell(dfu_fileP);
	rewind(dfu_fileP);

	filecrc = 0xffffffff;
	for (; i; i--)
		_crc(filecrc, (unsigned char)fgetc(dfu_fileP));

	printf("Computed CRC: \t 0x%.8x\n", filecrc);

	/* readout CRC from file*/
	memset(buffer, '\0', sizeof(buffer));
	fseek(dfu_fileP, -4, SEEK_END);
	if (fread(buffer, 1, 4, dfu_fileP) != 4) {
		printf("***ERROR: DFU_CHECK: error in fread\n");
		return -1;
	}

	crc_check = (u_int32)buffer[0];
	crc_check += (u_int32)buffer[1] << 8;
	crc_check += (u_int32)buffer[2] << 16;
	crc_check += (u_int32)buffer[3] << 24;

	printf("File CRC: \t 0x%.8x\n", crc_check);

	/* compare computed CRC and read CRC */
	if (filecrc != crc_check){
		printf("***ERROR: DFU_CHECK: Corrupted file\n");
		return -1;
	}

	rewind(dfu_fileP);

	return 0;
}

/****************************************************************************/
/** Extract data information from DFU file
*
*  \param dfu_fileP        \IN   pointer to dfu file
*  \param start_address    \OUT  start address of the firmware in the memory
*  \param data_size        \OUT  size of the firmware
*
*  \return                 success (0) or error (-1)
*/
static int32 get_dfu_data(FILE *dfu_fileP, u_int8 *start_address, u_int32 *data_size)
{
	u_int8 data_size_buffer[4];
	u_int8 buffer;

	/*---------------------------------------+
	|  extract start address                 |
	|  ATTENTION: Little Endian format !     |
	|             start_address[3] is MSB.   |
	+---------------------------------------*/
	fseek(dfu_fileP, 285, SEEK_SET);
	if (fread(start_address, 1, 4, dfu_fileP) != 4) {
		printf("***ERROR: GET_DFU_DATA: error in fread\n");
		return -1;
	}

	buffer = start_address[0];
	start_address[0] = start_address[3];
	start_address[3] = buffer;
	buffer = start_address[1];
	start_address[1] = start_address[2];
	start_address[2] = buffer;

	/*------------------------------------+
	|  extract size of the data           |
	|  ATTENTION: Little Endian format !  |
	+------------------------------------*/
	fseek(dfu_fileP, 289, SEEK_SET);
	if (fread(data_size_buffer, 1, 4, dfu_fileP) != 4) {
		printf("***ERROR: GET_DFU_DATA: error in fread\n");
		return -1;
	}

	*data_size  = (u_int32)data_size_buffer[0];
	*data_size += (u_int32)data_size_buffer[1] << 8;
	*data_size += (u_int32)data_size_buffer[2] << 16;
	*data_size += (u_int32)data_size_buffer[3] << 24;

	rewind(dfu_fileP);

	return 0;
}

/****************************************************************************/
/** Set the operating address
*
*  \param start_address    \IN  operating address for the CRC bootloader
*
*  \return                 success (0) or error code
*/
static int32 set_address(u_int8 *start_address)
{
	int err=0;

	err = SMB2API_WriteBlockData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_SETADDR_OPCODE, MEMORY_ADDR_LENGTH, start_address);
	if (err) {
		PrintError("***ERROR: SET_ADDRESS:", err);
		return err;
	}

	return 0;
}

/****************************************************************************/
/** Get the operating address
*
*  \param tmp_address    \OUT  buffer for the read address
*
*  \return               success (0) or error code
*/
static int32 get_address(u_int8 *tmp_address)
{
	int err=0;
	u_int8 address_length;

	err = SMB2API_ReadBlockData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_GETADDR_OPCODE, &address_length, tmp_address);
	if (err) {
		PrintError("***ERROR: GET_ADDRESS:", err);
		return err;
	}
	if (address_length != MEMORY_ADDR_LENGTH){
		printf("***ERROR: GET_ADDRESS: Wrong memory length\n");
		return -1;
	}

	return 0;
}

/****************************************************************************/
/** Send read memory command - not used
*
*  \return    success (0) or error code
*/
static int32 read_command()
{
	int err=0, i;
	u_int8 length;
	u_int8 data_buf[MAX_ZEICHEN];

	err = SMB2API_ReadBlockData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_READ_OPCODE, &length, data_buf);
	if (err) {
		PrintError("***ERROR: READ_COMMAND:", err);
		return err;
	}

	printf("Number of read bytes: %d\n", length);

	for (i=0; i<length; i++) {
		printf("%.x ", data_buf[i]);
		if (((i+1)%8) == 0)
			printf("\n");
	}

	return 0;
}

/****************************************************************************/
/** Send write memory command
*
*  \param byte_count    \IN  number of bytes to be written
*  \param data          \IN  data to be written
*
*  \return              success (0) or error code
*/
static int32 write_command(u_int8 byte_count, u_int8 *data)
{
	int err=0;

	if (byte_count > 32){
		printf("***ERROR: WRITE_COMMAND: Number of bytes to be write is out of range.\n");
		return -1;
	}

	err = SMB2API_WriteBlockData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
									BTL_WRITE_OPCODE, byte_count, data);
	if (err) {
		PrintError("***ERROR: WRITE_COMMAND:", err);
		return err;
	}

	return 0;
}

/****************************************************************************/
/** Send erase memory command
*
*  \param page_count    \IN  number of pages to be erased
*  \param page_size     \IN  page size
*
*  \return              success (0) or error code
*/
static int32 erase_command(u_int8 page_count, u_int16 page_size)
{
	int err=0;
	u_int8 tmp_address[4];
	u_int32 tmp_32address;

	get_address(tmp_address);
	tmp_32address  = (u_int32)tmp_address[0] << 24;
	tmp_32address += (u_int32)tmp_address[1] << 16;
	tmp_32address += (u_int32)tmp_address[2] << 8;
	tmp_32address += (u_int32)tmp_address[3];

	if (tmp_32address % page_size) {
		printf("Address must be a multiple of %d (1 page)\n", page_size);
		return -1;
	}

	err = SMB2API_WriteByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
									BTL_ERASE_OPCODE, page_count);
	if (err) {
		PrintError("***ERROR: ERASE_COMMAND:", err);
		return err;
	}

	printf("%d page(s) was/were erased.\n", page_count);

	return 0;
}

/****************************************************************************/
/** Check the last operation
*
*  \return    success (0) or error code
*/
static int32 check_last_op()
{
	int err=0;
	int32 ret=(-1);
	u_int8 ack;

	err = SMB2API_ReadByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_LASTOP_OPCODE, &ack);
	if (err) {
		PrintError("***ERROR: CHECK_LAST_OP:", err);
		ret = err;
	}
	else {
		if (ack == BTL_NACK) {
			printf("Last operation failed.\n");
		}
		else if (ack == BTL_ACK)
			ret = 0;
		else
			printf("***ERROR: CHECK_LAST_OP: Last operation has a not defined behaviour.\n");
	}

	return ret;
}

/****************************************************************************/
/** Enter the bootloader
*
*  \return    success (0) or error code
*/
static int32 enter_bootloader()
{
	int err=0;
	u_int8 dum_byte;

	err = SMB2API_ReadByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_LASTOP_OPCODE, &dum_byte);
	if (err) {
		PrintError("***ERROR: ENTER_BOOTLOADER:", err);
		return err;
	}

	if ((dum_byte == BTL_ACK) || (dum_byte == BTL_NACK)) {
		printf("Inside bootloader already.\n");
		return 0;
	}

	err = SMB2API_WriteByte(SMB2BTL_smbHdl, STM32_SMBFLAGS,
							FW_SMBADDR, ENTER_BTL_OPCODE);
	if (err) {
		PrintError("***ERROR: ENTER_BOOTLOADER:", err);
		return err;
	}

	return 0;
}

/****************************************************************************/
/** Leave the bootloader
*
*  \return    success (0) or error code
*/
static int32 leave_bootloader()
{
	int err=0;
	u_int8 dum_byte;

	err = SMB2API_ReadByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_LASTOP_OPCODE, &dum_byte);
	if (err) {
		PrintError("***ERROR: LEAVE_BOOTLOADER:", err);
		return err;
	}

	if (dum_byte == 0) {
		printf("Inside firmware already.\n");
		return 0;
	}

	/* Do not check error because bootloader is not acknowledging this command */
	SMB2API_WriteByte(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
						BTL_LVEBTL_OPCODE);

	return 0;
}

/***************************************************************************/
/** Retrieve the page size
*
*  \param page_size    \OUT  size of a page in the microcontroller
*
*  \return             success (0) or error code
*/
static int32 get_page_size(u_int16 *page_size)
{
	int err=0;

	err = SMB2API_ReadWordData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_GETPSIZE_OPCODE, page_size);
	if (err) {
		PrintError("***ERROR: GET_PAGE_SIZE:", err);
		return err;
	}

	return 0;
}

/***************************************************************************/
/** Download firmware
*
*  \param dfu_fileP    \IN  pointer to dfu file
*
*  \return             success (0) or error code
*/
static int32 download_fw(FILE *dfu_fileP)
{
	int err = 0, i;
	int write_count;
	u_int32 data_size;
	u_int16 page_size = 0;
	u_int8 page_count;
	u_int8 byte_count;
	u_int8 dum_byte;
	u_int8 remainder;
	u_int8 start_address[4];
	u_int8 tmp_address[4];
	u_int8 fw_data[MAX_ZEICHEN];
	u_int32 start_32address;
	u_int32 offset_address;

	printf("Start Download Firmware\n");
	printf("-------------------------------------------------------\n");

	/* Check if we are in the bootloader */
	err = SMB2API_ReadByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_LASTOP_OPCODE, &dum_byte);
	if (dum_byte == 0){
		printf("Entering the bootloader.\n");

		err = enter_bootloader();
		if (err)
			return err;

		UOS_Delay(500);
	}
	else if (err)
		return err;

	/* Check dfu file */
	err = dfu_check(dfu_fileP);
	if (err)
		return err;

	/* Get start address and size of data */
	err = get_dfu_data(dfu_fileP, start_address, &data_size);
	if (err)
		return err;

	printf("The flash address shall be 0x%.2x%.2x%.2x%.2x.\n", 
		start_address[0], start_address[1], start_address[2], start_address[3]);
	printf("The size of the data is %d bytes.\n", data_size);

	/* Set operating address */
	err = set_address(start_address);
	if (err)
		return err;

	printf("The operating address has been set to 0x%.2x%.2x%.2x%.2x.\n",
		start_address[0], start_address[1], start_address[2], start_address[3]);

	err = check_last_op();
	if (err)
		return err;

	/* Get operating address */
	err = get_address(tmp_address);
	if (err)
		return err;

	printf("The current operating address is 0x%.2x%.2x%.2x%.2x.\n",
		tmp_address[0], tmp_address[1], tmp_address[2], tmp_address[3]);

	err = check_last_op();
	if (err)
		return err;

	/* Compare addresses */
	for (i = 0; i < 4; i++){
		if (start_address[i] != tmp_address[i]){
			printf("***ERROR: Wrong operating address\n");
			return -1;
		}
	}

	/* Get page size */
	err = get_page_size(&page_size);
	if (err)
		return err;

	printf("Size of a page: %d bytes\n", page_size);

	err = check_last_op();
	if (err)
		return err;

	/* Erase pages */
	page_count = (u_int8)((data_size / page_size) + 1);
	err = erase_command(page_count, page_size);
	if (err)
		return err;

	UOS_Delay(500);

	/*--------------+
	| Flash memory  |
	+--------------*/
	start_32address  = (u_int32)start_address[0] << 24;
	start_32address += (u_int32)start_address[1] << 16;
	start_32address += (u_int32)start_address[2] << 8;
	start_32address += (u_int32)start_address[3];
	byte_count = MAX_ZEICHEN;

	printf("Flash address: 0x%.8x\n", start_32address);
	printf("Download firmware...\n");

	write_count = data_size / MAX_ZEICHEN;

	for (i=0; i<write_count; i++) {
		/* Copy data in buffer */
		fseek(dfu_fileP, (DATA_OFFSET + i * MAX_ZEICHEN), SEEK_SET);
		if (fread(fw_data, 1, MAX_ZEICHEN, dfu_fileP) != MAX_ZEICHEN) {
			printf("***ERROR: DOWNLOAD_FW: error in fread\n");
			return -1;
		}
		/* Set address */
		offset_address = start_32address + i * MAX_ZEICHEN;
		tmp_address[0] = (u_int8)(offset_address >> 24);
		tmp_address[1] = (u_int8)(offset_address >> 16);
		tmp_address[2] = (u_int8)(offset_address >> 8);
		tmp_address[3] = (u_int8)offset_address;
		err = set_address(tmp_address);
		if (err)
			return err;
		err = check_last_op();
		if (err)
			return err;
		/* Write data in memory */
		err = write_command(byte_count, fw_data);
		if (err)
			return err;
		err = check_last_op();
		if (err)
			return err;
	}

	remainder = (u_int8)(data_size - (write_count * MAX_ZEICHEN));
	if (remainder != 0) {
		/* Copy data in buffer */
		fseek(dfu_fileP, (DATA_OFFSET + write_count * MAX_ZEICHEN), SEEK_SET);
		if (fread(fw_data, 1, remainder, dfu_fileP) != remainder) {
			printf("***ERROR: DOWNLOAD_FW: error in fread\n");
			return -1;
		}

		/* Set address */
		offset_address = start_32address + write_count * MAX_ZEICHEN;
		tmp_address[0] = (u_int8)(offset_address >> 24);
		tmp_address[1] = (u_int8)(offset_address >> 16);
		tmp_address[2] = (u_int8)(offset_address >> 8);
		tmp_address[3] = (u_int8)offset_address;
		err = set_address(tmp_address);
		if (err)
			return err;
		err = check_last_op();
		if (err)
			return err;
		/* Write data in memory */
		err = write_command(remainder, fw_data);
		if (err)
			return err;
		err = check_last_op();
		if (err)
			return err;
	}

	rewind(dfu_fileP);

	/* Leave bootloader */
	err = leave_bootloader();
	if (err)
		return err;

	return 0;
}

/****************************************************************************/
/** Get and print the bootloader version
*/
static int32 print_bootld_version()
{
	int err;
	u_int8 lastOp = 0;
	u_int8 length = 0;
	u_int8 data_buf[BOOTLD_VER_LENGTH];

	/* Check if we are in the bootloader */
	err = SMB2API_ReadByteData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
								BTL_LASTOP_OPCODE, &lastOp);
	if (err) {
		PrintError("***ERROR: BOOTLOADER_VER:", err);
		return err;
	}

	if ((lastOp != BTL_ACK) && (lastOp != BTL_NACK)) {
		printf("***ERROR: BOOTLOADER_VER: Running firmware, not in bootloader\n");
		return -1;
	}

	/* Get bootloader version */
	err = SMB2API_ReadBlockData(SMB2BTL_smbHdl, STM32_SMBFLAGS, CRC_BTL_SMBADDR,
									BTL_BOOTVER_OPCODE, &length, data_buf);

	if (err) {
		PrintError("***ERROR: BOOTLOADER_VER:", err);
		return err;
	}

	if (length != BOOTLD_VER_LENGTH) {
		printf("***ERROR: BOOTLOADER_VER: Wrong data length\n");
		return -1;
	}

	printf("Bootloader Version:\n");
	printf("-----------------------------------------\n");
	printf("Error Code: %s\n", data_buf[0] ? "ERROR" : "OK");
	printf("Bootloader Revision %d.%d\n", data_buf[1], data_buf[2]);
	printf("Bootloader Maintenance Revision: %d\n", data_buf[3]);
	printf("Bootloader Build Number: %d\n", (data_buf[5] << 8) + data_buf[4]);
	printf("Verified: %s\n", data_buf[6] ? "TRUE" : "FALSE");

	return 0;
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

	printf("%s: %s\n", info, SMB2API_Errstring(errCode, errMsg));
}

