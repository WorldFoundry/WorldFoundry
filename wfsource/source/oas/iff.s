@*==============================================================================
@* iff.s generates the iff type files for 3dsmax attrib dialog boxes
@*==============================================================================
//==============================================================================
// created from iff.s DO NOT MODIFY
//==============================================================================

@define IFF_S

@* get common definitions
@include types.h
@* kts what was oad.ph? 7/11/99 
@* @include oad.ph

@*@define FIXED16(num)  ((short) num * 256)
@define FIXED32(num) @=f(num)

@define RADIOBUTTONNAMELEN 10

@define TAB@t

@define        SHOW_AS_N_A              0@t
@define        SHOW_AS_NUMBER           1@t                      @* numbers
@define        SHOW_AS_SLIDER           2@t                      @* numbers
@define        SHOW_AS_TOGGLE           3@t
@define        SHOW_AS_DROPMENU         4@t
@define        SHOW_AS_RADIOBUTTONS     5@t
@define        SHOW_AS_HIDDEN           6@t                      @* anything
@define        SHOW_AS_COLOR            7@t                      @* int32
@define			SHOW_AS_CHECKBOX		8@t
@define			SHOW_AS_MAILBOX			9@t
@define			SHOW_AS_COMBOBOX		10@t
@define			SHOW_AS_TEXTEDITOR	11@t
@define			SHOW_AS_FILENAME  	12@t

@* maximum value of a long int
@define LONG_MAX    2147483647
@* minimum value of a long int
@define LONG_MIN    -2147483647

@define	XDATA_IGNORE "Conversion: Ignore"
@define	XDATA_COPY "Conversion: Copy"                          
@define	XDATA_OBJECTLIST "Conversion: ObjectList"
@define	XDATA_CONTEXTUALANIMATIONLIST "Conversion: AnimationList"    
@define	XDATA_SCRIPT "Conversion: Convert"                     
@define	XDATA_CONVERSION_MAX "Conversion: Convert Max"             


@*============================================================================
@* create array

@define TYPEHEADER(displayName,variableName=displayName) @define OASNAME variableName@t@n{ 'TYPE'@n	{ 'NAME' "@+displayName@+" }@n
@define TYPEFOOTER }

@define PROPERTY_SHEET_HEADER(name,active=0,szEnableExpression="1",size=0)@-	{ 'STRU'	// PROPERTY SHEET START@nTAB		{ 'NAME' "@+name@+" }@n		{ 'OPEN' active@+y }
@define PROPERTY_SHEET_FOOTER()	}	// PROPERTY SHEET END

@* only int32's are allowed until we get better structure alignment
@define TYPEENTRYINT32(name, displayName=name, min, max, def=min, buttons="", showas=SHOW_AS_NUMBER,help="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		{ 'I32'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'RANG' min@+l max@+l }@n@\
TAB			{ 'DATA' def@+l }@n@\
TAB			{ 'DISP' showas@-@+l }@n@\
TAB			{ 'ENVL' buttons }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@* only fixed32's are allowed until we get better structure alignment
@define TYPEENTRYFIXED32(name, displayName=name, min, max, def=min,showas=SHOW_AS_NUMBER,help="",szEnableExpression="1",y=-1,x=-1)@\
@-TAB		{ 'F32'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'RANG' min max }@n@\
TAB			{ 'DATA' def }@n@\
TAB			{ 'DISP' showas@-@+l }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYVECTOR(name, displayName=name, min, max, def,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FIXED32, "@+name@+X", min, max, def, "", SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },
@define TYPEENTRYSTRING(name,displayName=name, count,help="",szEnableExpression="1",y=-1,x=-1,buttons="",showas=SHOW_AS_N_A) { BUTTON_STRING, "@+name@+", 0, 0, 0, count, "", SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },
@define TYPEENTRYSTRING_ENUM(name,displayName=name, count,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_STRING, "@+name@+", 0, 0, 0, count, "", SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },
@define TYPEENTRYSTRING_IGNORE(name,displayName=name,help="",szEnableExpression="1",y=-1,x=-1,showas=SHOW_AS_N_A) @\
@-TAB	{ 'STR'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'HINT' "Conversion: Ignore" }@n@\
TAB			{ 'DATA' "@+name@+" }@n@\
TAB			{ 'DISP' showas@+l }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYBOOLEAN(name,displayName=name, def,showas=SHOW_AS_CHECKBOX,help="",szEnableExpression="1",y=-1,x=-1)@\
TAB	{ 'I32'	// boolean@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'RANG' 0l 1l }@n@\
TAB			{ 'DATA' def@+l }@n@\
TAB			{ 'DISP' showas@-@+l }@n@\
TAB			{ 'ENVL' "False|True" }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYBOOLEANTOGGLE(name,displayName=name, def,showas=SHOW_AS_RADIOBUTTONS,buttons="FALSE|TRUE",help="",szEnableExpression="1",y=-1,x=-1)@\
TAB	{ 'I32'	//boolean@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'RANG' 0l 1l }@n@\
TAB			{ 'DATA' def@+l }@n@\
TAB			{ 'DISP' showas@-@+l }@n@\
TAB			{ 'ENVL' buttons }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@*@define TYPEENTRYBOOLEANTOGGLE(name,displayName=name, def,showas=SHOW_AS_RADIOBUTTONS,buttons="FALSE|TRUE",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_INT32, "@+name@+", 0,1, def, 0, buttons, showas, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },

@define TYPEENTRYOBJREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1,def="")@\
TAB		{ 'STR'		// Object Reference@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'HINT' "Object Reference" }@n@\
TAB			{ 'DATA' def }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@*@define TYPEENTRYCAMERAREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1)        { BUTTON_CAMERA_REFERENCE,"@+name@+",0,0,0,12,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@*@define TYPEENTRYLIGHTREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_LIGHT_REFERENCE,"@+name@+",0,0,0,12,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@*@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FILENAME,"@+name@+",0,0,0,80,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@*@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FILENAME,"@+name@+",0,0,0,80,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}, filespec "\0\0" },
@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="1",y=-1,x=-1) @\
TAB		{ 'STR'  // filename @n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'DATA' "" }@n@\
TAB			{ 'HINT' "FILESPEC:" filespec }@n@\
TAB			{ 'DISP' SHOW_AS_FILENAME@+l }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@*		{ BUTTON_FILENAME,"@+name@+",0,0,0,80,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}, filespec "\0\0" },

@*@define TYPEENTRYXDATA(name,displayName=name, chunkName,help="",szEnableExpression="1",y=-1,x=-1) @\
@*@-TAB		{ 'STR'@n@\
@*TAB			{ 'NAME' "@+name@+" }@n@\
@*TAB			{ 'DSNM' "@+displayName@+" }@n@\
@*TAB			{ 'HINT' XDATA_IGNORE }@n@\
@*TAB			{ 'BOOL' { 'NAME' "Required" } { 'DATA' 0 } }@n@\
@*TAB			{ 'DISP' SHOW_AS_TEXTEDITOR@+l }@n@\
@*TAB			{ 'HELP' help }@n@\
@*TAB			{ 'ENBL' szEnableExpression }@n@\
@*TAB		}


@define TYPEENTRYXDATA_CONVERT(name,displayName=name, chunkName,required, help="",szEnableExpression="1",y=-1,x=-1, conversion=XDATA_COPY) @\
@-TAB	{ 'STR'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'HINT' conversion }@n@\
TAB			{ 'DATA' @+chunkName@+ }@n@\
TAB			{ 'BOOL' { 'NAME' "Required" } { 'DATA' required@+l } }@n@\
TAB			{ 'DISP' SHOW_AS_TEXTEDITOR@+l }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYCOLOR(name, displayName=name, def, help="",szEnableExpression="1",y=-1,x=-1)  @\
@-TAB	{ 'I32'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'RANG' 0l  16777215l }@n@\
TAB			{ 'HINT' "Color" }@n@\
TAB			{ 'DATA' def@+l }@n@\
TAB			{ 'DISP' SHOW_AS_COLOR@+l }@n@\
TAB			{ 'HELP' help }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYCAMERA(name,displayName=name, followObj, szEnableExpression="1") @\
@-TAB	{ 'STR'@n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'HINT' "Extract Camera" }@n@\
TAB			{ 'DATA' followObj }@n@\
TAB			{ 'DISP' SHOW_AS_HIDDEN@+l }@n@\
TAB			{ 'HELP' "Indicates the camera will be updated with new values such as look at, roll, and FOV" }@n@\
TAB			{ 'ENBL' szEnableExpression }@n@\
TAB		}

@define TYPEENTRYWAVEFORM(name,displayName=name, help="", szEnableExpression="1") { BUTTON_WAVEFORM, "@+name@+", 0, 0, 0, 0, "", SHOW_AS_N_A, -1, -1, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@define TYPEENTRYCLASSREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1,def="") @\
TAB		{ 'STR'  // class reference @n@\
TAB			{ 'NAME' "@+name@+" }@n@\
TAB			{ 'DSNM' "@+displayName@+" }@n@\
TAB			{ 'HINT' "Class Reference" }@n@\
TAB			{ 'DATA' def }@n@\
TAB			{ 'ENBL' "@+displayName@+" }@n@\
TAB		}

@* BUTTON_CLASS_REFERENCE,"@+name@+",0,0,0,12,def,SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@define GROUP_START(name,width=0,szEnableExpression="1",y=-1,x=-1)@-TAB		{ 'STRU'	// GROUP START@nTAB			{ 'NAME' "@+name@+" }@undef TAB @define TAB@t@n
@define GROUP_STOP(y=-1,x=-1)@-TAB	} 	// GROUP STOP@undef TAB @define TAB@t

@* entries which are only used by the level converter

@*@define LEVELCONFLAGSHORTCUT		{ LEVELCONFLAG_SHORTCUT,    "LEVELCONFLAG_SHORTCUT",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGSHORTCUT	@\
@-TAB		{ 'HINT' "ShortCut" }

@*@define LEVELCONFLAGNOINSTANCES    { LEVELCONFLAG_NOINSTANCES,    "LEVELCONFLAG_NOINSTANCES",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGNOINSTANCES @\
@-TAB		{ 'HINT' "No Instances" }

@*@define LEVELCONFLAGNOMESH         { LEVELCONFLAG_NOMESH,         "LEVELCONFLAG_NOMESH",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGEXTRACTLIGHT @\
@-TAB		{ 'HINT' "Extract Light" }
@*{ LEVELCONFLAG_EXTRACTLIGHT,   "LEVELCONFLAG_EXTRACTLIGHT",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGSINGLEINSTANCE { LEVELCONFLAG_SINGLEINSTANCE, "LEVELCONFLAG_SINGLEINSTANCE",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGTEMPLATE       { LEVELCONFLAG_TEMPLATE,       "LEVELCONFLAG_TEMPLATE",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGEXTRACTCAMERA  { LEVELCONFLAG_EXTRACTCAMERA,  "LEVELCONFLAG_EXTRACTCAMERA",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGEXTRACTCAMERANEW
@*{ LEVELCONFLAG_EXTRACTCAMERANEW,  "LEVELCONFLAG_EXTRACTCAMERANEW",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },

@define LEVELCONFLAGROOM @\
@-TAB		{ 'HINT' "Extract Room" }

@define LEVELCONFLAGCOMMONBLOCK(name)@-TAB	{ 'STRU'	{ 'NAME' "@+name@+" } { 'FLAG' "COMMONBLOCK" }	// COMMONBLOCK@undef TAB @define TAB	@t@include name@+.inc
@*LEVELCONFLAGCOMMONBLOCK,"@+name@+",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },@include name@+.inc
@define LEVELCONFLAGENDCOMMON @-TAB	}	// ENDCOMMON

//============================================================================

@include TYPEFILE_OAS@+.oas

//============================================================================

