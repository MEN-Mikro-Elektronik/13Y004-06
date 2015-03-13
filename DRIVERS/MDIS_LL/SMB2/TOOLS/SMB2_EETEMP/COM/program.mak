#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#          $Date: 2014/10/01 09:05:37 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SMB2_EETEMP tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2014/10/01 09:05:37  ts
#   R: compiling the other tools smb2_boardident and _bmc failed due to link order of libs
#   M: changed order of libmdis and libsmb2 in this tool too
#
#   Revision 1.1  2011/06/09 13:04:59  MRoth
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2011 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=smb2_eetemp

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h \
         $(MEN_INC_DIR)/usr_utl.h  \
         $(MEN_INC_DIR)/mdis_api.h \
         $(MEN_INC_DIR)/usr_oss.h  \
         $(MEN_INC_DIR)/smb2_api.h \


MAK_INP1=smb2_eetemp$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
