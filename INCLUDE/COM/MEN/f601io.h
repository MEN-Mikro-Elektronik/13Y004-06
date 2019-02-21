/***********************  I n c l u d e  -  F i l e  ************************/
/*!  
 *        \file  f601io.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2007/01/24 11:38:49 $
 *    $Revision: 3.2 $
 * 
 *  	 \brief  Header file for I2C Expander (Philips PCF8574) on F601
 *                      
 *     Switches: -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright (c) 2006-2019, MEN Mikro Elektronik GmbH
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

#ifndef _F601IO_H
#define _F601IO_H

#ifdef __cplusplus
	extern "C" {
#endif


/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/*! \defgroup _F601IO F601 IO defines */
/**@{*/
#define F601IO_ADDR		0x44	/**< SMB device address */

#define F601IO_OUT1		0x01	/**< Output #1 */
#define F601IO_OUT2		0x02	/**< Output #2 */

#define F601IO_IN1		0x04	/**< Input #1 */
#define F601IO_IN2		0x08	/**< Input #2 */
#define F601IO_IN3		0x10	/**< Input #3 */

#define F601IO_IO5		0x20	/**< I/O #5 */
#define F601IO_IO6		0x40	/**< I/O #6 */
#define F601IO_IO7		0x80	/**< I/O #7 */
/**@}*/


/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/

/** direction values */
enum ioDir
{
      dirIn,
      dirOut,
      dirInOut,
};

/** i/o table */
struct _IO_TABLE
{
    char		*name;	/** port name */
    enum ioDir	dir;	/** direction */
    u_int8		bit;	/** bit of i/o */
} G_F601IO_tbl[] =
{/* port name	direction   bit */
	{"OUT1",	dirOut,		F601IO_OUT1},
	{"OUT2",	dirOut,		F601IO_OUT2},
	{"IN1 ",	dirIn,		F601IO_IN1},
	{"IN2 ",	dirIn,		F601IO_IN2},
	{"IN3 ",	dirIn,		F601IO_IN3},
	{"IO5 ",	dirInOut,	F601IO_IO5},
	{"IO6 ",	dirInOut,	F601IO_IO6},
	{"IO7 ",	dirInOut,	F601IO_IO7},
};

/** number of i/o */
#define F601IO_NBR_OFF_IO\
    sizeof(G_F601IO_tbl)/sizeof(struct _IO_TABLE)

#define F601IO_IN_MASK	(F601IO_IN1 | F601IO_IN2 | F601IO_IN3)

#ifdef __cplusplus
	}
#endif

#endif	/* _F601IO_H */

