#***************************  M a k e f i l e  *******************************
#
#         Author: dp
#          $Date: 2014/07/29 11:09:24 $
#      $Revision: 1.3 $
#
#    Description: Makefile definitions for the SMB2_F601 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.3  2014/07/29 11:09:24  ts
#   R: building (linking) the F601 example failed on Ubuntu 12.04
#   M: change linking order of used libraries
#
#   Revision 1.2  2009/06/22 11:59:27  dpfeuffer
#   R: MDVE warning
#   M: added mdis_err.h
#
#   Revision 1.1  2006/05/31 08:22:25  DPfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_f601

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)   \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)  \
	 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/smb2_api.h	\
         $(MEN_INC_DIR)/f601io.h	\
         $(MEN_INC_DIR)/smb2.h		\

MAK_INP1=smb2_f601$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
