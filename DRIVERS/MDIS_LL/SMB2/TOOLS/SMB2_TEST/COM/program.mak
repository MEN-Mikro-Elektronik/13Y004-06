#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2016/04/11 11:16:00 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the SMB2_TEST tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.1  2016/04/11 11:16:00  dpfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2016 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=smb2_test

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h \
         $(MEN_INC_DIR)/usr_utl.h  \
         $(MEN_INC_DIR)/mdis_api.h \
         $(MEN_INC_DIR)/usr_oss.h  \
         $(MEN_INC_DIR)/smb2_api.h \


MAK_INP1=smb2_test$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)