#===============================================================================
# Makefile for velocity levels
#===============================================================================

.SUFFIXES:
.SUFFIXES: .def .des .lvl .max .prj .ini


OBJS = \
!include "levels.1"

ALL :	$(OBJS) mailbox.def

!include "levels.2"
