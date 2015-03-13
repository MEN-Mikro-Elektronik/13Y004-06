#***************************  M a k e f i l e  *******************************
#
#         Author: Roman.Schneider@men.de
#          $Date: 2014/09/19 15:57:54 $
#      $Revision: 1.3 $
#
#    Description: Makefile definitions for SMB_BMC tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.3  2014/09/19 15:57:54  ts
#   R: compile under linux failed due to link order of libs
#   M: changed order of libmdis_api and libsmb2_api
#
#   Revision 1.2  2014/02/20 18:21:48  MRoth
#   R: cosmetics
#   M: changed file header
#
#   Revision 1.1  2014/01/07 17:38:05  MRoth
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2013 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_bmc

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h    \
         $(MEN_INC_DIR)/usr_oss.h     \
         $(MEN_INC_DIR)/mdis_api.h    \
         $(MEN_INC_DIR)/mdis_err.h    \

MAK_INP1=smb2_bmc$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
