#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#          $Date: 2014/10/01 09:05:35 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SMB2_EEPROD2 tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2014/10/01 09:05:35  ts
#   R: compiling the other tools smb2_boardident and _bmc failed due to link order of libs
#   M: changed order of libmdis and libsmb2 in this tool too
#
#   Revision 1.1  2009/07/31 17:25:30  MRoth
#   Initial Revision
#
#   Revision 1.1  2009/07/17 15:25:10  MRoth
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=smb2_eeprod2

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h \
         $(MEN_INC_DIR)/usr_utl.h  \
         $(MEN_INC_DIR)/mdis_api.h \
         $(MEN_INC_DIR)/usr_oss.h  \
         $(MEN_INC_DIR)/smb2_api.h \
         $(MEN_INC_DIR)/eeprod.h   \

MAK_INP1=smb2_eeprod2$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
