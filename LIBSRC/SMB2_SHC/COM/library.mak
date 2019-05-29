#***************************  M a k e f i l e  *******************************
#
#         Author: andreas.werner@men.de
#          $Date: 2015/02/24 17:26:53 $
#      $Revision: 1.2 $
#
#    Description: Makefile descriptor file for SMB2_SHC lib
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2015-2019, MEN Mikro Elektronik GmbH
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
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

MAK_NAME=smb2_shc
MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)  \

MAK_INCL=$(MEN_INC_DIR)/men_typs.h  \
         $(MEN_INC_DIR)/smb2_api.h  \
         $(MEN_INC_DIR)/smb2_shc.h

MAK_INP1 = smb2_shc$(INP_SUFFIX)

MAK_INP  = $(MAK_INP1)

