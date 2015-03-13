#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2009/06/22 11:59:25 $
#      $Revision: 1.5 $
#
#    Description: Makefile definitions for SMB2 tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.5  2009/06/22 11:59:25  dpfeuffer
#   R: MDVE warning
#   M: added smb2.h
#
#   Revision 1.4  2006/05/31 08:22:21  DPfeuffer
#   smb2.h added
#
#   Revision 1.3  2006/03/07 15:00:47  DPfeuffer
#   problem during check-in fixed
#
#   Revision 1.2  2006/03/07 14:30:30  DPfeuffer
#   usr_oss.h added
#
#   Revision 1.1  2006/02/28 15:57:41  DPfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_ctrl

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/smb2_api.h	\
         $(MEN_INC_DIR)/smb2.h	\
         $(MEN_MOD_DIR)/cmd_tbl.h	\
         $(MEN_MOD_DIR)/ident.h		\
         $(MEN_MOD_DIR)/smb2_ctrl.h	\
         $(MEN_INC_DIR)/mdis_err.h	\

MAK_INP1=smb2_ctrl$(INP_SUFFIX)
MAK_INP2=smb2api$(INP_SUFFIX)

MAK_INP=$(MAK_INP1) $(MAK_INP2)
