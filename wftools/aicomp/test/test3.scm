(
	( writeln "Here is a string" )

	( define ACTOR 128 )
	( writeln ACTOR )
	( define Actor ACTOR )
	( writeln Actor )
	( define mailbox MB 2000 )

;	( define XXX ( + 1 2 ) )
	( define XXX 1 )

	( XXX )

	( + 1 2 )

	( set! ACTOR.MB ( + 10.0 3.14 ) )
	( ACTOR.MB )
	( MB )
	( set! ACTOR.MB 5.0 )

;;	( MB )
;;	( self.MB )
;
;;	( ACTOR )
;;	( self.ACTOR )
;;	( set! 100 2000 5.0 )

;;	( define ZZZ ( lambda X (+ 1 2) (+ 3 4) )
;;	)
;;
;;	include "bad.s"
;
;	( define ACTOR 128 )
;;	include "d:/wfsrc/levels.src/oad/room.def"
;	( self.ACTOR )
;	( ACTOR.ACTOR )
;
;	( ACTOR )
;
;;	( event SOUND
;;;		(begin
;;			(+ 3 4)
;;			(+ 3 4)
;;;		)
;;	)

;	( ZZZ )
	( writeln #'1234' )

;	( event SOUND
;		( begin
;			(+ 1 2)
;			(+ 3 4)
;		)
;	)

	( + ( begin (+ 1 2) (+ 3 4)) (begin (+ 1 2) (+ 3 4 )) )
;	( + ( ZZZ ) ( ZZZ ) )

;	( +
;		( fn )
;		( fn )
;	)

	( ACTOR.ACTOR )

	( #'1234' )

	( if 1
		( writeln "then" )
	)

	( begin
		( 3 )
		( 4 )
		( 6 )
		( 7 )
	)

	( if 0
		( if 0
			( 100 )
			( if 1
				( 300 )
				( -300 )
			)
		)
		( if 1
			( begin
				( 200 )
				( 201 )
				( 202 )
				( 203 )
				( 204 )
				( 205 )
			)
			( -200 )
		)
	 )

	( if ( * ( - 1 1 ) 3.14 )
		( writeln "true" )
		( begin
			( writeln "false" )
			( writeln "( fuck off )" )
		)
	)

;	( + 4 2 )

;	( writeln "Hello there" )

;	include "noexist.fuck"

;	include "objects.id"

;	( writeln "write line" )

;	( run-to 1 2 )

;	( begin
;		( 1 )
;		( 2 )
;		( 3 )
;	)

;	( exit )

;	( writeln "Shouldn't get here" )

)
