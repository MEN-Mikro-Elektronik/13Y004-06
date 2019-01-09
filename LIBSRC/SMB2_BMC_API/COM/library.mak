#***************************  M a k e f i l e  *******************************
#
#         Author: andreas.werner@men.de
#          $Date: 2015/08/14 15:35:22 $
#      $Revision: 1.1 $
#
#    Description: Makefile descriptor file for SMB2_BMC_API lib
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: library.mak,v $
#   Revision 1.1  2015/08/14 15:35:22  awerner
#   Initial Revision
#
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2015 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_bmc_api
MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)  \

MAK_INCL=$(MEN_INC_DIR)/men_typs.h  \
         $(MEN_INC_DIR)/smb2_api.h  \
         $(MEN_INC_DIR)/smb2_bmc_api.h

MAK_INP1 = smb2_bmc_api$(INP_SUFFIX)

MAK_INP  = $(MAK_INP1)

