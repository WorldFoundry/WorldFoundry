/*==============================================================================*/
/* TYPES3ds.C generates the arrays for 3dstudio dialog boxes				 	*/
/* (Each line defines a mapping function between a macro used in the .OAS files */
/* and a C struct which appears in the .HT files.)								*/
/*==============================================================================*/

@define TYPES3DS_S

@* get common definitions
@include types.h

@*@define FIXED16(num) ( ((short) num * 256) )
@define FIXED32(num) ( ((long) (num * 65536)))

@define RADIOBUTTONNAMELEN 10

#include "pigtool.h"
#include "oad.h"

/*============================================================================*/
/* create array */

@define TYPEHEADER(displayName,variableName=displayName) @define OASNAME name@t _oadHeader header =	{'OAD ',0, "@+displayName@+",0x00010202};@n@ntypeDescriptor huge tempstruct[] = {
@define TYPEFOOTER };

@define PROPERTY_SHEET_HEADER(name,active=0,szEnableExpression="1",size=0)		{ BUTTON_PROPERTY_SHEET,"@+name@+",0,1,active,0,"",SHOW_AS_N_A,-1,-1,"",{XDATA_IGNORE,0,"@+name@+", szEnableExpression, size }},
@define PROPERTY_SHEET_FOOTER

@* only int32's are allowed until we get better structure alignment
@define TYPEENTRYINT32(name, displayName=name, min, max, def=min, buttons="", showas=SHOW_AS_NUMBER,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_INT32, "@+name@+", min, max, def, 0, buttons, showas, x, y, help, { XDATA_IGNORE,0, "@+displayName@+", szEnableExpression } },

@* only fixed32's are allowed until we get better structure alignment
@define TYPEENTRYFIXED32(name, displayName=name, min, max, def=min,showas=SHOW_AS_N_A,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FIXED32, "@+name@+", min, max, def, 0, "", showas, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },

@define TYPEENTRYVECTOR(name, displayName=name, min, max, def,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FIXED32, "@+name@+X", min, max, def, "", SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },

@define TYPEENTRYBOOLEAN(name,displayName=name, def,showas=SHOW_AS_CHECKBOX,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_INT32, "@+name@+", 0,1, def, 0, "False|True", showas, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },
@define TYPEENTRYBOOLEANTOGGLE(name,displayName=name, def,showas=SHOW_AS_RADIOBUTTONS,buttons="FALSE|TRUE",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_INT32, "@+name@+", 0,1, def, 0, buttons, showas, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression} },

@define TYPEENTRYOBJREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1,def="",showAs=SHOW_AS_N_A)    { BUTTON_OBJECT_REFERENCE,"@+name@+",0,0,0,12,def,showAs, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@define TYPEENTRYCAMERAREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1)        { BUTTON_CAMERA_REFERENCE,"@+name@+",0,0,0,12,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@define TYPEENTRYLIGHTREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_LIGHT_REFERENCE,"@+name@+",0,0,0,12,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@*@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FILENAME,"@+name@+",0,0,0,80,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_FILENAME,"@+name@+",0,0,0,80,"",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}, filespec "\0\0" },

@define TYPEENTRYSTRING_IGNORE(name,displayName=name,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_XDATA,"@+name@+",-1,0xfffff,0,80,"@+name@+",SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}},
@define TYPEENTRYSTRING(name,displayName=name,count,help="",szEnableExpression="1",y=-1,x=-1,labels="",showas=SHOW_AS_N_A) { BUTTON_XDATA,"@+name@+",-1,0xfffff,0,count,labels,showas, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}},

@define TYPEENTRYXDATA(name,displayName=name, chunkName,help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_XDATA,"@+name@+",-1,0xfffff,0,80,chunkName,SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+",szEnableExpression}},
@define TYPEENTRYXDATA_CONVERT(name,displayName=name, chunkName,required,help="",szEnableExpression="1",y=-1,x=-1, conversion=XDATA_COPY)        { BUTTON_XDATA,"@+name@+", -1, 0xfffff, 0, 80, chunkName, SHOW_AS_N_A, x, y, help, {conversion, required, "@+displayName@+", szEnableExpression } },

@define TYPEENTRYCOLOR(name, displayName=name, def, help="",szEnableExpression="1",y=-1,x=-1) { BUTTON_INT32, "@+name@+", 0, 0xFFFFFF, def, 0, "", SHOW_AS_COLOR, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@define TYPEENTRYCAMERA(name,displayName=name, followObj, szEnableExpression="1") { BUTTON_EXTRACT_CAMERA, "@+name@+", 0, 0, 0, 0, followObj, SHOW_AS_N_A, 0, 0, "Indicates the camera will be updated with new values such as look at, roll, and FOV", {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression } },

@define TYPEENTRYWAVEFORM(name,displayName=name, help="", szEnableExpression="1") { BUTTON_WAVEFORM, "@+name@+", 0, 0, 0, 0, "", SHOW_AS_N_A, -1, -1, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@define TYPEENTRYCLASSREFERENCE(name,displayName=name, help="",szEnableExpression="1",y=-1,x=-1,def="")  { BUTTON_CLASS_REFERENCE,"@+name@+",0,0,0,12,def,SHOW_AS_N_A, x, y, help, {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},

@define GROUP_START(name,width=0,szEnableExpression="1",y=-1,x=-1)     { BUTTON_GROUP_START, "@+name@+", 0,width,0, 0,"",SHOW_AS_N_A, x,y, "", {XDATA_IGNORE,0,"@+displayName@+", szEnableExpression }},
@define GROUP_STOP(y=-1,x=-1)                           { BUTTON_GROUP_STOP, "STOP", 0,0,0, 0,"",SHOW_AS_N_A, x,y, "", {XDATA_IGNORE,0,"@+displayName@+"} },

@* entries which are only used by the level converter

@define LEVELCONFLAGSHORTCUT		{ LEVELCONFLAG_SHORTCUT,    "LEVELCONFLAG_SHORTCUT",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGNOINSTANCES    { LEVELCONFLAG_NOINSTANCES,    "LEVELCONFLAG_NOINSTANCES",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGNOMESH         { LEVELCONFLAG_NOMESH,         "LEVELCONFLAG_NOMESH",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGEXTRACTLIGHT   { LEVELCONFLAG_EXTRACTLIGHT,   "LEVELCONFLAG_EXTRACTLIGHT",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGSINGLEINSTANCE { LEVELCONFLAG_SINGLEINSTANCE, "LEVELCONFLAG_SINGLEINSTANCE",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGTEMPLATE       { LEVELCONFLAG_TEMPLATE,       "LEVELCONFLAG_TEMPLATE",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@*@define LEVELCONFLAGEXTRACTCAMERA  { LEVELCONFLAG_EXTRACTCAMERA,  "LEVELCONFLAG_EXTRACTCAMERA",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGEXTRACTCAMERANEW  { LEVELCONFLAG_EXTRACTCAMERANEW,  "LEVELCONFLAG_EXTRACTCAMERANEW",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGROOM           { LEVELCONFLAG_ROOM,         "LEVELCONFLAG_ROOM",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },
@define LEVELCONFLAGCOMMONBLOCK(name)  { LEVELCONFLAG_COMMONBLOCK,"@+name@+",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },@include name@+.inc
@define LEVELCONFLAGENDCOMMON      { LEVELCONFLAG_ENDCOMMON,"LEVELCONFLAG_ENDCOMMON",0,0,0,0,"",SHOW_AS_HIDDEN,-1,-1,"" },

/*============================================================================*/

@include TYPEFILE_OAS@+.oas

/*============================================================================*/
