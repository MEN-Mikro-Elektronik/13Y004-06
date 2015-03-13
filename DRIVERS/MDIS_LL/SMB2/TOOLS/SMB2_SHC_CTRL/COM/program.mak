#***************************  M a k e f i l e  *******************************
#
#         Author: andreas.werner@men.de
#          $Date: 2015/02/24 17:26:42 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SMB2_SHC_CTRL tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2015/02/24 17:26:42  MRoth
#   R: cosmetics
#   M: revised code
#
#   Revision 1.1  2014/10/15 13:00:50  awerner
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2014 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=smb2_shc_ctrl

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)   \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_shc$(LIB_SUFFIX)  \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)  \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h  \
         $(MEN_INC_DIR)/usr_utl.h   \
         $(MEN_INC_DIR)/mdis_api.h  \
         $(MEN_INC_DIR)/usr_oss.h   \
         $(MEN_INC_DIR)/smb2_api.h  \
         $(MEN_INC_DIR)/smb2.h      \
         $(MEN_INC_DIR)/mdis_err.h  \
         $(MEN_INC_DIR)/smb2_shc.h

MAK_INP1=smb2_shc_ctrl$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)

