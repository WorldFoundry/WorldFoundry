; shell.s
; Sample World Foundry Game Shell

(
	include "mailbox.def"
	(define mailbox phase  6000 )

	( if phase
		( begin
			( set! phase 0)
		)
		( begin
			( set! phase 1)
			( set! LEVEL_TO_RUN 0 )
		)
	)

	( writeln "Default Shell" )
	( write "new level = ")
	( write LEVEL_TO_RUN )
	( writeln " " )
;	( set! LEVEL_TO_RUN 1)
)
