@*============================================================================
@* OADTYPES.C generates the types C header file from types.oas
@*============================================================================
/*============================================================================*/
/* TYPEFILE_OAS@+.ht: created from OADTYPES.S and TYPEFILE_OAS@+.oas DO NOT MODIFY */
/*============================================================================*/

#include <pigsys/pigsys.hp>
#include <math/scalar.hp>
                 
                 
@define OADTYPES_S

@*@define FIXED16(num) ( ((short) num * 256) )
@define FIXED32(num) ( ((long) num * 65536.0))

/*      maximum value of a long int     */
@define LONG_MAX    (2147483647L)
/* minimum value of a long int  */
@define LONG_MIN    (-2147483647L-1)

@define OBJ_NAME_LENGTH 11
@define CAMERA_NAME_LENGTH      11
@define LIGHT_NAME_LENGTH       11

@* get common definitions
@include types.h

@*============================================================================
@* create C struct, if you add or change anything, be sure to update types3ds.s as well

@define TYPEHEADER(displayName,variableName=displayName) @define OASNAME variableName@t struct _"@+variableName@+" {
@define TYPEFOOTER };

@define PROPERTY_SHEET_HEADER(name,active=0,szEnableExpression="",size=0)
@define PROPERTY_SHEET_FOOTER

@* only int32's are allowed until we get better structure alignment
@*@define TYPEENTRYINT8(name, displayName=name, min, max, def,buttons="",showas,help="",y=-1,x=-1) int8 "@+name@+";               /* Minumum: min Maximum: max Default: def */
@*@define TYPEENTRYINT16(name, displayName=name, min, max, def,buttons="",showas,help="",y=-1,x=-1) int16 "@+name@+";             /* Minumum: min Maximum: max Default: def */
@define TYPEENTRYINT32(name, displayName=name, min, max, def=min,buttons="",showas=SHOW_AS_NUMBER,help="",szEnableExpression="",y=-1,x=-1) int32 "@+name@+";               /* Minumum: min Maximum: max Default: def */

@* only fixed32's are allowed until we get better structure alignment
@*@define TYPEENTRYFIXED16(name, displayName=name, min, max, def, showas=SHOW_AS_N_A,help="",szEnableExpression="",y=-1,x=-1) fixed16 "@+name@+";               /* Minumum: min Maximum: max */
@define TYPEENTRYFIXED32(name, displayName=name, min, max, def, showas=SHOW_AS_N_A,help="",szEnableExpression="",y=-1,x=-1) @\
fixed32 "@+name@+";         /* Minumum: min Maximum: max */ @\
@n    Scalar "Get@+name@+"() const { return Scalar::FromFixed32("@+name@+"); }

@define TYPEENTRYVECTOR(name, displayName=name, min, max, def,help="",szEnableExpression="",y=-1,x=-1) fixed32 "@+name@+X";             /* Minumum: min Maximum: max */@n fixed32 "@+name@+Y";@n fixed32 "@+name@+Z";

@*@define TYPEENTRYSTRING(name,displayName=name, count,help="",szEnableExpression="",y=-1,x=-1) char "@+name@+"[count];
@*@define TYPEENTRYSTRING(name,displayName=name,help="",szEnableExpression="",y=-1,x=-1) error! what to do here?
@define TYPEENTRYSTRING_IGNORE(name,displayName=name,help="",szEnableExpression="",y=-1,x=-1)
@define TYPEENTRYBOOLEAN(name,displayName=name, def,showas="",szEnableExpression="",y=-1,x=-1) int32 "@+name@+";        /* Default: def */
@define TYPEENTRYBOOLEANTOGGLE(name,displayName=name, def,showas="",buttons="FALSE|TRUE",help="",szEnableExpression="",y=-1,x=-1) int32 "@+name@+";     /* Default: def */

@define TYPEENTRYOBJREFERENCE(name,displayName=name, help="",szEnableExpression="",y=-1,x=-1,def="")    int32 "@+name@+";                               /* index into master object list, = OBJECT_NULL if no reference */
@define TYPEENTRYCAMERAREFERENCE(name,displayName=name, help="",szEnableExpression="",y=-1,x=-1)
@define TYPEENTRYLIGHTREFERENCE(name,displayName=name, help="",szEnableExpression="",y=-1,x=-1)

@define TYPEENTRYFILENAME(name,displayName=name,filespec="*.*",help="",szEnableExpression="",y=-1,x=-1)		int32 "@+name@+";	/* This is a packedAssetID */

@define TYPEENTRYXDATA(name,displayName=name, chunkName,help="",szEnableExpression="",y=-1,x=-1)
@define TYPEENTRYXDATA_CONVERT(name,displayName=name, chunkName,required,help="",szEnableExpression="",y=-1,x=-1,conversion=XDATA_COPY)       int32 "@+name@+";

@define TYPEENTRYCOLOR(name, displayName=name, def, help="",szEnableExpression="",y=-1,x=-1) int32 "@+name@+";

@define TYPEENTRYCAMERA(name,displayName=name, followObj, szEnableExpression="")

@define TYPEENTRYWAVEFORM(name,displayName=name,szEnableExpression="")

@define TYPEENTRYCLASSREFERENCE(name,displayName=name, help="",szEnableExpression="",y=-1,x=-1,def="")  int32 "@+name@+";                               /* index into master class list, = CLASS_NULL if no reference */

@define GROUP_START(name,width=0,szEnableExpression="",y=-1,x=-1)
@define GROUP_STOP(y=-1,x=-1)

@* entries which are only used by the level converter

@define LEVELCONFLAGSHORTCUT
@define LEVELCONFLAGNOINSTANCES
@define LEVELCONFLAGNOMESH
@define LEVELCONFLAGEXTRACTLIGHT
@*@define LEVELCONFLAGSINGLEINSTANCE
@*@define LEVELCONFLAGTEMPLATE
@*@define LEVELCONFLAGEXTRACTCAMERA
@define LEVELCONFLAGEXTRACTCAMERANEW
@define LEVELCONFLAGROOM
@define LEVELCONFLAGENDCOMMON
@define LEVELCONFLAGCOMMONBLOCK(name,help="",y=-1,x=-1) int32 name@+PageOffset;                 /* offset in page data for this objects page */

@*============================================================================
@* create structure declaration

#ifndef TYPEFILE_OAS@+_HT
#define TYPEFILE_OAS@+_HT

@include TYPEFILE_OAS@+.oas

/*============================================================================*/
#endif         /* TYPEFILE_OAS@+_HT */
/*============================================================================*/
