@*============================================================================
@* common.inc: entries common to all objects
@*============================================================================

@ifndef(DEFAULT_HITPOINTS)(@define DEFAULT_HITPOINTS 32767)

@ifndef(DEFAULT_SCRIPTNAME)(@define DEFAULT_SCRIPTNAME Script)

@*============================================================================

	@include xdata.inc

	PROPERTY_SHEET_HEADER(Common)
@*----------------------
		TYPEENTRYFIXED32(hp,,FIXED32(0),FIXED32(32767),FIXED32(DEFAULT_HITPOINTS))
		TYPEENTRYINT32(Number Of Local Mailboxes,Local Mailboxes,0,40,0,"",SHOW_AS_SLIDER)
		TYPEENTRYOBJREFERENCE(Poof)
		TYPEENTRYBOOLEAN(Is Needle Gun Target,Needle Gun Target,0)
		TYPEENTRYINT32(Write To Mailbox On Death,Death Mailbox, 0, 3999, 0,,SHOW_AS_NUMBER,"Mailbox to write to when object is destroyed")
@*		TYPEENTRYXDATA_CONVERT(Script,DEFAULT_SCRIPTNAME,"Cave Logic Studios AI [Script]||..\\scripts\\default.s",XDATA_NOT_REQUIRED,,,,,XDATA_SCRIPT)
		TYPEENTRYXDATA_CONVERT(Script,DEFAULT_SCRIPTNAME,"Cave Logic Studios AI [Script]||what is this?..\\scripts\\default.s",XDATA_NOT_REQUIRED,,,,,XDATA_COPY)
		TYPEENTRYBOOLEAN(Script Controls Input,,0)
		TYPEENTRYXDATA_NOTES

		TYPEENTRYFIXED32(slopeA,, FIXED32(-32768), FIXED32(32767), FIXED32(0), SHOW_AS_HIDDEN, "Sloped surface plane coefficient A")
		TYPEENTRYFIXED32(slopeB,, FIXED32(-32768), FIXED32(32767), FIXED32(0), SHOW_AS_HIDDEN, "Sloped surface plane coefficient B")
		TYPEENTRYFIXED32(slopeC,, FIXED32(-32768), FIXED32(32767), FIXED32(0), SHOW_AS_HIDDEN, "Sloped surface plane coefficient C")
		TYPEENTRYFIXED32(slopeD,, FIXED32(-32768), FIXED32(32767), FIXED32(0), SHOW_AS_HIDDEN, "Sloped surface plane coefficient D")

	PROPERTY_SHEET_FOOTER
	LEVELCONFLAGENDCOMMON

@*============================================================================
