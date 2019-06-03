#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#
#    Description: Makefile definitions for the SMB2_BOARDIDENT tool
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2013-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
MAK_NAME=smb2_boardident
# the next line is updated during the MDIS installation
STAMPED_REVISION="13Y004-06_01_42-24-ge5f4d78-dirty_2019-05-30"

DEF_REVISION=MAK_REVISION=$(STAMPED_REVISION)
MAK_SWITCH=$(SW_PREFIX)$(DEF_REVISION)

MAK_LIBS= \
	             $(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)  \
                 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)  \
                 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
                 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)   \

MAK_INCL=$(MEN_INC_DIR)/men_typs.h \
         $(MEN_INC_DIR)/usr_utl.h  \
         $(MEN_INC_DIR)/mdis_api.h \
         $(MEN_INC_DIR)/usr_oss.h  \
         $(MEN_INC_DIR)/smb2_api.h \
         $(MEN_INC_DIR)/eeprod.h   \

MAK_INP1=smb2_boardident$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
