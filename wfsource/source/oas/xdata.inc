@* xdata.inc

@ifdef(XDATA_NOT_REQUIRED)(@begincomment)

@define XDATA_NOT_REQUIRED	0
@define XDATA_REQUIRED		1

@define TYPEENTRYXDATA_NOTES	TYPEENTRYSTRING_IGNORE(Cave Logic Studios Notes|,Notes,,,,,SHOW_AS_TEXTEDITOR)

@* @define TYPEENTRYXDATA_AI	TYPEENTRYXDATA(AI Script,,"Cave Logic Studios AI|aicomp|Cave Logic Studios Compiled AI Script|Converting AI Script")

@* @define TYPEENTRYXDATA_OBJECTLIST	TYPEENTRYXDATA(Object List,,"Cave Logic Studios Object List|")

@define TYPEENTRYXDATA_CONTEXTUALANIMS TYPEENTRYXDATA_CONVERT(Contextual Animations,,"Cave Logic Studios Contextual Animation List|",XDATA_NOT_REQUIRED,"Animations to load in this room",,,, XDATA_CONTEXTUALANIMATIONLIST)


@define ALLFILES_FILESPEC	"All Files (*.*)\0*.*\0"
@define BITMAP_FILESPEC		"Windows Bitmap Files (*.bmp)\0*.bmp\0Targa Bitmap Files (*.tga)\0*.tga\0SGI Bitmap Files (*.rgb,*.rgba,*.bw)\0*.rgb;*.rgba;*.bw\0"
@define MAP_FILESPEC		"Scrolling Map Files (*.map)\0*.bmp\0"

@endcomment
