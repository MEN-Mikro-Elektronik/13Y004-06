#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#          $Date: 2013/01/21 18:41:49 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SMB2_BOARDIDENT tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2013/01/21 18:41:49  ts
#   R: building the tool as static linked failed under linux
#   M: changed order of libs linking (relevant in static build)
#
#   Revision 1.1  2009/07/31 17:25:27  MRoth
#   Initial Revision
#
#   Revision 1.1  2009/07/17 15:25:30  MRoth
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2013 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************
MAK_NAME=smb2_boardident

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
