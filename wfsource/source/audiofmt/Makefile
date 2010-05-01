################################################################################
# Makefile for audiofmt library
################################################################################
# 'make all'		: Builds the audiofmt library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = audiofmt
!include ../../Makefile.env
TEST_PROG = sox
#TEST_OBJS += $(LIB_OBJ_DIR)util.obj
#TEST_LIBS += $(PIGS_OBJ_DIR)/getopt/getopt.lib

LIB_OBJS += \
	$(LIB_OBJ_DIR)raw.obj \
	$(LIB_OBJ_DIR)voc.obj \
	$(LIB_OBJ_DIR)au.obj \
	$(LIB_OBJ_DIR)sf.obj \
	$(LIB_OBJ_DIR)aiff.obj \
	$(LIB_OBJ_DIR)hcom.obj \
	$(LIB_OBJ_DIR)8svx.obj \
	$(LIB_OBJ_DIR)sndrtool.obj \
	$(LIB_OBJ_DIR)wav.obj \
	$(LIB_OBJ_DIR)smp.obj \
	$(LIB_OBJ_DIR)sbdsp.obj \
	$(LIB_OBJ_DIR)auto.obj \
	$(LIB_OBJ_DIR)cdr.obj \
	$(LIB_OBJ_DIR)dat.obj \
	$(LIB_OBJ_DIR)g711.obj \
	$(LIB_OBJ_DIR)g72x.obj \
	$(LIB_OBJ_DIR)g721.obj \
	$(LIB_OBJ_DIR)g723_24.obj \
	$(LIB_OBJ_DIR)g723_40.obj \
	$(LIB_OBJ_DIR)maud.obj \
	$(LIB_OBJ_DIR)wve.obj \
	$(LIB_OBJ_DIR)null.obj \
	\
	$(LIB_OBJ_DIR)copy.obj \
	$(LIB_OBJ_DIR)avg.obj \
	$(LIB_OBJ_DIR)pred.obj \
	$(LIB_OBJ_DIR)stat.obj \
	$(LIB_OBJ_DIR)vibro.obj \
	$(LIB_OBJ_DIR)echo.obj \
	$(LIB_OBJ_DIR)rate.obj \
	$(LIB_OBJ_DIR)band.obj \
	$(LIB_OBJ_DIR)lowp.obj \
	$(LIB_OBJ_DIR)highp.obj \
	$(LIB_OBJ_DIR)reverse.obj \
	$(LIB_OBJ_DIR)cut.obj \
	$(LIB_OBJ_DIR)map.obj \
	$(LIB_OBJ_DIR)split.obj \
	$(LIB_OBJ_DIR)pick.obj \
	$(LIB_OBJ_DIR)resample.obj \
	$(LIB_OBJ_DIR)nulleff.obj \
	\
	$(LIB_OBJ_DIR)libst.obj \
	$(LIB_OBJ_DIR)log.obj \
	$(LIB_OBJ_DIR)util.obj \
	$(LIB_OBJ_DIR)misc.obj \
	$(LIB_OBJ_DIR)mask.obj \
	$(LIB_OBJ_DIR)handlers.obj \

################################################################################

!include ../Makefile.lib

################################################################################
