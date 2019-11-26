#***************************  M a k e f i l e  *******************************
#  
#         Author: dieter.pfeuffer@men.de
#                      
#    Description: Makefile descriptor file for SMB2_API lib
#                      
#-----------------------------------------------------------------------------
#   Copyright 2006-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

MAK_NAME=smb2_api
# the next line is updated during the MDIS installation
STAMPED_REVISION="13Y004-06_01_42-24-ge5f4d78-dirty_2019-05-30"

DEF_REVISION=MAK_REVISION=$(STAMPED_REVISION)
MAK_SWITCH=$(SW_PREFIX)$(DEF_REVISION)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h    	\
		 $(MEN_INC_DIR)/mdis_err.h		\
         $(MEN_INC_DIR)/mdis_api.h		\
		 $(MEN_INC_DIR)/usr_oss.h		\
		 $(MEN_INC_DIR)/smb2_api.h		\
		 $(MEN_INC_DIR)/smb2_drv.h		\
		 $(MEN_INC_DIR)/smb2.h	\

MAK_INP1 = smb2_api$(INP_SUFFIX)

MAK_INP  = $(MAK_INP1)

