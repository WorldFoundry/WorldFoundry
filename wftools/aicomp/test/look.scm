(
;============================================================================
; main character script
;
; Uses button C to toggle Look mode on/off
;============================================================================

	; includes
	include "objects.id"
	include "mailbox.def"
	include "joystick.def"
	include "user.def"

	( define GMACamTar 1 )
	( define GMACamShot 2 )

	; joystick mappings
	( define JOYSTICK_BUTTON_LOOK JOYSTICK_BUTTON_C )	; which button the user presses to activate panning mode

	; constants
	( define LOOK_ROTATION_RATE 0.0125 )

	; local mailboxes
	( define LOOK_FLAG 2001 )		; are we in LOOK mode?
	( define VISIBILITY 2002 )		; visibility mailbox, keep in sync with visibility mailbox field in oad
	( define TARGET_YAW 2003 )		; local storage of yaw
	( define TARGET_PITCH 2004 )	; local storage of pitch
	( define VECTOR_X 2005 )		; intermediate value of direction vector
	( define VECTOR_Y 2006 )		; 	(before offset to camshot)
	( define VECTOR_Z 2007 )

	( if (read-mailbox self LOOK_FLAG )
		( begin                                     ;do this if LOOK mode active
			( write-mailbox self INPUT 0)
			( write-mailbox self VISIBILITY FALSE )

			; if C was just pressed, exit LOOK mode
			( if ( & JOYSTICK_BUTTON_LOOK (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW_JUSTPRESSED ) ) )
				( begin
					( writeln "End LOOK" )
			   		( write-mailbox self LOOK_FLAG FALSE)
					( write-mailbox self DIRECTOR_CAMSHOT_HIGH NULL )
				)
				( 0 )
			)

			; change the eulers based on joystick input
 			( if ( & JOYSTICK_BUTTON_LEFT (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW ) ) )
				( begin
					( write-mailbox self TARGET_YAW ( + ( read-mailbox self TARGET_YAW ) LOOK_ROTATION_RATE ) )
					( writeln "LEFT" )
				)
				( 0 )
			)
 			( if ( & JOYSTICK_BUTTON_RIGHT (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW ) ) )
 				( write-mailbox self TARGET_YAW ( - ( read-mailbox self TARGET_YAW ) LOOK_ROTATION_RATE ) )
 				( 0 )
 			)

			( if ( & JOYSTICK_BUTTON_DOWN (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW ) ) )
 				( write-mailbox self TARGET_PITCH ( - ( read-mailbox self TARGET_PITCH ) LOOK_ROTATION_RATE ) )
 				( 0 )
 			)

 			( if ( & JOYSTICK_BUTTON_UP (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW ) ) )
 				( write-mailbox self TARGET_PITCH ( + ( read-mailbox self TARGET_PITCH ) LOOK_ROTATION_RATE ) )
 				( 0 )
 			)

 			( write-mailbox self TARGET_PITCH ( max ( min (read-mailbox self TARGET_PITCH) 0.23)-0.23) )
		; set the target position relative to the camshot based on the eulers
			( euler-to-vector 0 (read-mailbox self TARGET_PITCH) (read-mailbox self TARGET_YAW) 10 self VECTOR_X)
			( write-mailbox GMACamTar X_POS ( + (read-mailbox GMACamShot X_POS) (read-mailbox self VECTOR_X ) ) )
			( write-mailbox GMACamTar Y_POS ( + (read-mailbox GMACamShot Y_POS) (read-mailbox self VECTOR_Y ) ) )
			( write-mailbox GMACamTar Z_POS ( + (read-mailbox GMACamShot Z_POS) (read-mailbox self VECTOR_Z ) ) )
		)

		( begin         	; do this if LOOK mode false
			( write-mailbox self VISIBILITY TRUE )
			( write-to-mailbox self INPUT (&  (~ (int JOYSTICK_BUTTON_LOOK)) ( read-mailbox self HARDWARE_JOYSTICK1 )))
			( if ( & JOYSTICK_BUTTON_LOOK (fixed (read-mailbox self HARDWARE_JOYSTICK1_RAW_JUSTPRESSED ) ) )
				( begin
					( writeln "Begin LOOK" )
					(write-to-mailbox self LOOK_FLAG TRUE)
					(write-to-mailbox self DIRECTOR_CAMSHOT_HIGH (int GMACamShot) )
				)
				(write-mailbox self TARGET_YAW (- (read-mailbox self ROTATION_C) 0.25 ))
			)
		)
	)

;==============================================================================
)
