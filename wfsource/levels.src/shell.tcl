## shell.tcl
## Sample World Foundry Game Shell for tcl

   set phase 6000

	if { [read-mailbox $phase] } { 
			 write-mailbox $phase 0
		} else {
			write-mailbox $phase 1
			write-mailbox $INDEXOF_LEVEL_TO_RUN 0 
		}




##	( writeln "Default Shell" )
##	( write "new level = ")
##	( write LEVEL_TO_RUN )
##	( writeln " " )
##;	( set! LEVEL_TO_RUN 1)
