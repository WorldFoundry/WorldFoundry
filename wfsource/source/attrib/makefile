################################################################################
# Makefile for attrib library
################################################################################
# 'make all'		: Builds the library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = attrib
!include ../../Makefile.env
.PATH.cc += lib;buttons
.PATH.cc += udm
.PATH.rc += udm
CCCARGS += -D__ATTRIB__=1
#INCLUDE += -I$(WF_DIR) -I$(WF_TOOLS_DIR)
################################################################################
TEST_PROG = udm
TEST_OBJS += $(LIB_OBJ_DIR)gui.obj $(LIB_OBJ_DIR)version.obj
TEST_LIBS += $(IFFWRITE) $(EVAL) $(LOADFILE) $(REGISTRY) comctl32.lib comdlg32.lib

LIB_OBJS += \
	$(LIB_OBJ_DIR)attrib.obj \
	$(LIB_OBJ_DIR)oad.obj \
	$(LIB_OBJ_DIR)oaddlg.obj \
	\
	$(LIB_OBJ_DIR)int.obj \
	$(LIB_OBJ_DIR)fixed.obj \
	$(LIB_OBJ_DIR)propshet.obj \
	$(LIB_OBJ_DIR)string.obj \
	$(LIB_OBJ_DIR)group.obj \
	$(LIB_OBJ_DIR)dropmenu.obj \
	$(LIB_OBJ_DIR)checkbox.obj \
	$(LIB_OBJ_DIR)filename.obj \

################################################################################

!include ../Makefile.lib

################################################################################
