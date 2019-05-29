#***************************  M a k e f i l e  *******************************
#
#         Author: andreas.werner@men.de
#          $Date: 2015/08/14 15:35:22 $
#      $Revision: 1.1 $
#
#    Description: Makefile descriptor file for SMB2_BMC_API lib
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2015-2019, MEN Mikro Elektronik GmbH
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

MAK_NAME=smb2_bmc_api
MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)  \

MAK_INCL=$(MEN_INC_DIR)/men_typs.h  \
         $(MEN_INC_DIR)/smb2_api.h  \
         $(MEN_INC_DIR)/smb2_bmc_api.h

MAK_INP1 = smb2_bmc_api$(INP_SUFFIX)

MAK_INP  = $(MAK_INP1)

