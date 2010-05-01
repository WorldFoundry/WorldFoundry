#=============================================================================
# veltools/Makefile
#=============================================================================


%if $(os) == OS2 || $(OS) == NT
; = &
%else
; = ;
%endif


#3rdparty
#bin
#delta
#delta2
#doc
#oldtools
#samples

# Nothing to make
#	progress \

# Requires user intervention
#	psxdisc \

# Old DOS/Windows tools that we don't need anymore
#sendmail \
#delay \
#touch \
#lc \
#reg \
#tail \
#mktree \

# Probably obselete, but need to investigate further
#makedep \

#Not used anymore, but not abandoned
#        vabdump \

# Needs major work before it's useful
#vrml2iff \
#link \

#disr3000 \     [ need to find dumpobj.cc ]

## NOTE: where is bumpver?

## Not built anymore?
##       pclib \


#	aicomp \


DIR_LIST = \
        chargrab \
        eval \
        iffcomp \
        iffdb \
        iffdump \
        iff2lvl \
        imagedir \
        lvldump \
        oaddump \
        prep \
        psxprof \
        reg \
#        template \
        textile \
        wfbind \
        xcd \

##       stripper \


ALL :
	%foreach Dir $(DIR_LIST)
		@%echo ===============================================================================
		@%echo ===============================================================================
		@%echo $(Dir)
		@%echo ===============================================================================
                ( cd $(Dir) $; make /nologo $(MFLAGS) )
	%end

clean :
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; make /nologo $(MFLAGS) $(.TARGET) )
	%end

#==============================================================================

copybinaries: ALL
	copy iffcomp\iffcomp.exe $(WF_DIR)\bin
	copy iff2lvl\iff2lvl.exe $(WF_DIR)\bin
	copy prep\prep.exe $(WF_DIR)\bin
	copy iffdump\iffdump.exe $(WF_DIR)\bin
	copy lvldump\lvldump.exe $(WF_DIR)\bin
	copy textile\textile.exe $(WF_DIR)\bin
	copy reg\reg.exe $(WF_DIR)\bin

#==============================================================================
