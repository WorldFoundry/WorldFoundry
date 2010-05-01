@*============================================================================
@* OADdef.s generates the .def files for the scripting system
@*============================================================================
;============================================================================
; TYPEFILE_OAS@+.def: created from OADdef.S and TYPEFILE_OAS@+.oas DO NOT MODIFY
;============================================================================

@define OADDEF_S

@define FIXED32(num) ( ((long) num * 65536.0))

@* maximum value of a long int
@define LONG_MAX    (2147483647L)
@* minimum value of a long int
@define LONG_MIN    (-2147483647L-1)

@define OBJ_NAME_LENGTH 11
@define CAMERA_NAME_LENGTH      11
@define LIGHT_NAME_LENGTH       11

@* get common definitions
@include types.h

@*============================================================================
@* create C struct, if you add or change anything, be sure to update types3ds.s as well

@define TYPEHEADER(displayName,variableName=displayName)
@define TYPEFOOTER

@define PROPERTY_SHEET_HEADER(name,active=0,szEnableExpression="1",size=0)
@define PROPERTY_SHEET_FOOTER

@define TYPEENTRYINT32(name, displayName=name, min, max, def=min,buttons="",showas=SHOW_AS_NUMBER,help="",y=-1,x=-1) (define "OAD_@+name@+" 4)               ; int32: Minumum: min Maximum: max Default: def
@define TYPEENTRYFIXED32(name, displayName=name, min, max, def, showas=SHOW_AS_N_A,help="",y=-1,x=-1) (define "OAD_@+name@+" 4)         ; fixed32: Minumum: min Maximum: max
@*@define TYPEENTRYVECTOR(name, displayName=name, min, max, def,help="",y=-1,x=-1) fixed32 "OAD_@+name@+X";             ; Minumum: min Maximum: max @n fixed32 "OAD_@+name@+Y";@n fixed32 "OAD_@+name@+Z";

@define TYPEENTRYSTRING(name,displayName=name, count,help="",y=-1,x=-1) (define "OAD_@+name@+" count)			; string: length = count
@define TYPEENTRYBOOLEAN(name,displayName=name, def,showas="",y=-1,x=-1) (define "OAD_@+name@+" 4)        ; boolean: Default: def
@define TYPEENTRYBOOLEANTOGGLE(name,displayName=name, def,showas="",buttons="FALSE|TRUE",help="",y=-1,x=-1) (define "OAD_@+name@+" 4)     ; boolan toggle: Default: def

@define TYPEENTRYOBJREFERENCE(name,displayName=name, help="",y=-1,x=-1,def="")    (define "OAD_@+name@+" 4)                               ; Object Reference: index into master object list, = OBJECT_NULL if no reference
@*@define TYPEENTRYCAMERAREFERENCE(name,displayName=name, help="",y=-1,x=-1)
@*@define TYPEENTRYLIGHTREFERENCE(name,displayName=name, help="",y=-1,x=-1)

@define TYPEENTRYFILENAME(name,displayName=name, help="",y=-1,x=-1)		(define "OAD_@+name@+" 4)	;file name: This is a packedAssetID
@define TYPEENTRYXDATA(name,displayName=name, chunkName,help="",y=-1,x=-1)
@define TYPEENTRYXDATA_CONVERT(name,displayName=name, chunkName,required,help="",y=-1,x=-1,conversion=XDATA_COPY)       (define "OAD_@+name@+" 4)   ; xdata_convert:

@define TYPEENTRYCOLOR(name, displayName=name, def, help="",y=-1,x=-1) (define "OAD_@+name@+" 4)  ; Color

@define TYPEENTRYCAMERA(name,displayName=name, followObj)

@define TYPEENTRYWAVEFORM(name,displayName=name)

@define TYPEENTRYCLASSREFERENCE(name,displayName=name, help="",y=-1,x=-1,def="")  (define "OAD_@+name@+" 4)                              ; class reference: index into master class list, = CLASS_NULL if no reference

@define GROUP_START(name,width=0,y=-1,x=-1)
@define GROUP_STOP(y=-1,x=-1)

@* entries which are only used by the level converter
@define LEVELCONFLAGSHORTCUT
@define LEVELCONFLAGNOINSTANCES
@define LEVELCONFLAGEXTRACTCAMERANEW
@define LEVELCONFLAGEXTRACTLIGHT
@define LEVELCONFLAGROOM
@define LEVELCONFLAGENDCOMMON
@define LEVELCONFLAGCOMMONBLOCK(name,help="",y=-1,x=-1) (define OAD_@+name@+PageOffset 4)                 ; common block: offset in page data for this objects page

@*============================================================================
@* create structure declaration

@include TYPEFILE_OAS@+.oas

;============================================================================
