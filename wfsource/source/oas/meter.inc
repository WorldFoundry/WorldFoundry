	PROPERTY_SHEET_HEADER(Meter,1)
@*-----------------------
  TYPEENTRYOBJREFERENCE(Object)
  TYPEENTRYINT32(Mailbox To Display,, 0, 3999, 0,,SHOW_AS_NUMBER, "", "Object" )
	TYPEENTRYINT32(Mailbox Type,, 0, 1, 0, " Scalar | Integer ", SHOW_AS_RADIOBUTTONS )
@*TYPEENTRYFILENAME(Overlay Image,,,)
  TYPEENTRYFIXED32(Path X Start,, FIXED32(-200), FIXED32(200), FIXED32(-140),, "" )
  TYPEENTRYFIXED32(Path Y Start,, FIXED32(-200), FIXED32(200), FIXED32(-130),, "" )
	GROUP_START(Display)
		TYPEENTRYINT32(Increment Before Display,Trigger Delta, 0, 1000, 1, "", SHOW_AS_NUMBER ,"Change in value in last displayed value before value will be displayed again" )
		TYPEENTRYINT32(Pause At Keyframe,, 0, 1000, 1, "", SHOW_AS_NUMBER, "Which keyframe is paused" )
		TYPEENTRYFIXED32(Display Pause Duration,Pause Duration,FIXED32(0),FIXED32(32000),FIXED32(1),,"How long the meter is displayed")
	GROUP_STOP()

  TYPEENTRYINT32(Text X Offset,, 0, 319, 15, "", SHOW_AS_NUMBER )
  TYPEENTRYINT32(Text Y Offset,, 0, 223, 2, "", SHOW_AS_NUMBER )
@*TYPEENTRYFILENAME(Font,,,,)
	PROPERTY_SHEET_FOOTER
