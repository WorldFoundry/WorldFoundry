@*============================================================================
@* moveman.inc: oas data for movement manager common block
@*============================================================================

@ifndef(DEFAULT_MOBILITY)(@define DEFAULT_MOBILITY 0)

@ifndef(DEFAULT_MOBILITY_ENABLE_EXPR)(@define DEFAULT_MOBILITY_ENABLE_EXPR "1")

@ifndef(DEFAULT_MOVESBETWEENROOMS)(@define DEFAULT_MOVESBETWEENROOMS 0)

@ifndef(DEFAULT_MOVEMENT_PROPSHEET)(@define DEFAULT_MOVEMENT_PROPSHEET 0)

@ifndef(DEFAULT_VISIBILITY)(@define DEFAULT_VISIBILITY 1)

@ifndef(DEFAULT_MASS)(@define DEFAULT_MASS 75.0)

@ifndef(DEFAULT_FRICTION)(@define DEFAULT_FRICTION 0.95)
                                                                                       
@* get enumerated object types                                                                 
@include objects.p
                                                                 
@*============================================================================

PROPERTY_SHEET_HEADER(Movement,DEFAULT_MOVEMENT_PROPSHEET)

	TYPEENTRYINT32(MovementClass,Movement Class, 0, 100, @e0(OASNAME@+_KIND)@-, , SHOW_AS_NUMBER,,1)
        @* kts note: the Mobility type list must be kept in sync with enumeration in oas/movement.h
	TYPEENTRYINT32(Mobility,, 0, 4, DEFAULT_MOBILITY, "Anchored|Physics|Path|Camera|Follow", SHOW_AS_DROPMENU,,DEFAULT_MOBILITY_ENABLE_EXPR)
  	TYPEENTRYFIXED32(Mass,,FIXED32(0),FIXED32(100),FIXED32( DEFAULT_MASS ))
	TYPEENTRYBOOLEAN(Moves Between Rooms,,DEFAULT_MOVESBETWEENROOMS,,,"Mobility!=0")
	TYPEENTRYINT32(Movement Mailbox,, 0, 3999, 1, "", SHOW_AS_NUMBER,,"Mobility!=0")
	TYPEENTRYFIXED32(Step Size,,FIXED32(0),FIXED32(100),FIXED32(0.55))
	TYPEENTRYFIXED32(Vertical Elasticity,,FIXED32(0),FIXED32(1),FIXED32(0.5))
	TYPEENTRYFIXED32(Horizontal Elasticity,,FIXED32(0),FIXED32(1),FIXED32(0.5))
	TYPEENTRYFIXED32(Surface Friction,,FIXED32(0),FIXED32(1.0),FIXED32(DEFAULT_FRICTION))

	GROUP_START(Path,,"Mobility==2")
		TYPEENTRYINT32(At End Of Path,At End, 0, 5, 0, "Ping-Pong|Stop|Jumpback|Delete|Derail|WarpBack", SHOW_AS_DROPMENU,,"Mobility==2")
		TYPEENTRYOBJREFERENCE(Object To Follow,,,"Mobility==2",,,,SHOW_AS_VECTOR)
		TYPEENTRYOBJREFERENCE(Follow Offset,,,"Mobility==2",,,,SHOW_AS_VECTOR)
	GROUP_STOP()

	GROUP_START(On Ground,,"Mobility==1")
		TYPEENTRYFIXED32(Running Acceleration,Acceleration,FIXED32(-1000),FIXED32(1000),FIXED32(1.0),,,"Mobility==1")
		TYPEENTRYFIXED32(Running Deceleration,Deceleration,FIXED32(-1.0),FIXED32(1.0),FIXED32(0.90),,,"Mobility==1")
		TYPEENTRYFIXED32(Max Ground Speed,Max Speed, FIXED32(0),FIXED32(100),FIXED32(10.0),,,"Mobility==1")
		TYPEENTRYFIXED32(Turn Rate,,FIXED32(0),FIXED32(100),FIXED32(5),,,"Mobility==1")
	GROUP_STOP()

	GROUP_START(Crawling,,"Mobility==1")
		TYPEENTRYFIXED32(Crawling Acceleration,Acceleration,FIXED32(-1000),FIXED32(1000),FIXED32(0.50),,,"Mobility==1")
	GROUP_STOP()

	GROUP_START(Jumping,,"Mobility==1")
		TYPEENTRYFIXED32(Jumping Acceleration,Acceleration,FIXED32(-1000),FIXED32(1000),FIXED32(18.0),,,"Mobility==1")
		TYPEENTRYFIXED32(Jumping Momentum Transfer,Momentum Xfer,FIXED32(-1.0),FIXED32(1.0),FIXED32(0.50),,,"Mobility==1")
	GROUP_STOP()

	GROUP_START(In Air,,"Mobility==1")
		TYPEENTRYFIXED32(Air Acceleration,Acceleration,FIXED32(-1000),FIXED32(1000),FIXED32(1.0),,,"Mobility==1")
		TYPEENTRYFIXED32(Horiz Air Drag,Horiz Drag, FIXED32(-1000),FIXED32(1000),FIXED32(0.25),,,"Mobility==1")
		TYPEENTRYFIXED32(Vert Air Drag,Vert Drag, FIXED32(-1000),FIXED32(1000),FIXED32(0.25),,,"Mobility==1")
		TYPEENTRYFIXED32(Max Air Speed,Max Speed, FIXED32(0),FIXED32(100),FIXED32(10.0),,,"Mobility==1")
	GROUP_STOP()

	GROUP_START(Falling,,"Mobility==1")
		TYPEENTRYFIXED32(Falling Acceleration,Acceleration,FIXED32(-1000),FIXED32(1000),FIXED32(9.81),,,"Mobility==1")
		TYPEENTRYFIXED32(Fall Anim Threshold,Anim Threshold, FIXED32(0),FIXED32(100),FIXED32(2.0),,,"Mobility==1")
	GROUP_STOP()

	TYPEENTRYFIXED32(Stun Threshold,,FIXED32(0),FIXED32(100),FIXED32(2.5),,,"Mobility==1")
	TYPEENTRYFIXED32(Stun Duration,,FIXED32(0),FIXED32(100),FIXED32(1.0),,,"Mobility==1")

PROPERTY_SHEET_FOOTER
LEVELCONFLAGENDCOMMON

@*============================================================================
