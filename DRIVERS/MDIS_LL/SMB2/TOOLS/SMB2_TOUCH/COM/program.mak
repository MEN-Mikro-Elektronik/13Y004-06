#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2014/09/19 15:58:18 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the smb2_touch program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2014/09/19 15:58:18  ts
#   R: compile under linux failed due to link order of libs
#   M: changed order of libmdis_api and libsmb2_api
#
#   Revision 1.1  2013/09/18 09:28:17  dpfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2013 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_touch

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/smb2_api.h	\
         $(MEN_INC_DIR)/smb2.h		\

MAK_INP1=smb2_touch$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
