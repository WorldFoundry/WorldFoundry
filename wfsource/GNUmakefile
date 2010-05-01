# top level makefile for World Foundry
# NOTE that according to the GNU make info page, ${MAKEFLAGS} is prefered
# over the older ${MFLAGS}, but as MAKEFLAGS does NOT start with a '-', it
# breaks things, where as the older MFLAGS starts with a hyphen and works well.

.PHONY : ALL clean
ALL :
	${MAKE} -C source

clean:
	${MAKE} -C source clean
