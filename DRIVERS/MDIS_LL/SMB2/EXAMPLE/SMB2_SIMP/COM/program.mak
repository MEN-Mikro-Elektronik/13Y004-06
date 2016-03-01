#***************************  M a k e f i l e  *******************************
#
#         Author: dp
#          $Date: 2014/10/10 15:45:20 $
#      $Revision: 1.4 $
#
#    Description: Makefile definitions for the SMB2 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.4  2014/10/10 15:45:20  channoyer
#   R: AD78 not required to build SMB2_SIMP
#   M: Remove include ad78_drv.h
#
#   Revision 1.3  2014/07/17 17:31:23  ts
#   R: defined from AD78 were missing
#   M: added include ad78.h
#
#   Revision 1.2  2009/06/22 11:59:12  dpfeuffer
#   R: MDVE warning
#   M: added smb2.h
#
#   Revision 1.1  2006/02/28 15:57:29  DPfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=smb2_simp

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/smb2_api$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)   \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/smb2_api.h	\
         $(MEN_INC_DIR)/smb2.h		\


MAK_INP1=smb2_simp$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
