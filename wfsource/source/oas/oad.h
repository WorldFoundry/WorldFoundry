/*============================================================================*/
/* oad.h: structures for object attribute descriptor files */
/* included by the 3d Studio plug-ins so they can read .oad files */
/*============================================================================*/

#ifndef OAD_H
#define OAD_H

#define FIXED16(n) ( (short)(n * 256) )
#define FIXED32(n) ( (long)(n * 65536) )
#define RADIOBUTTONNAMELEN 10


/*============================================================================*/
/* kts include pigtool.h instead */

#ifndef SYS_INT32
#include <oas/pigtool.h>
#endif

/*============================================================================*/
/* WARNING: do not change the order or delete any fields from this enum, or */
/*   all 3dstudio level data will become invalid */

//typedef enum
//{
#define        BUTTON_FIXED16           0
#define        BUTTON_FIXED32           1
#define        BUTTON_INT8              2
#define        BUTTON_INT16             3
#define        BUTTON_INT32             4
#define        BUTTON_STRING            5
#define        BUTTON_OBJECT_REFERENCE  6
#define        BUTTON_FILENAME          7
#define        BUTTON_PROPERTY_SHEET    8
        /* level converter specific flags not used by the 3DS plug-ins */
#define        LEVELCONFLAG_NOINSTANCES 9              /* prevents the addition of object instances */
#define        LEVELCONFLAG_NOMESH      10              /* prevents this object from having a mesh reference */
#define        LEVELCONFLAG_SINGLEINSTANCE 11   /* only allow one instance of this type */
	/* user (game) defined flags, not used by levelcon, but passed through, follow */
	/* velocity specific flags */
#define        LEVELCONFLAG_TEMPLATE    12              /* indicates an object is not to be added to the level, */
																			/* but instead is added to a list of objects to use when */
																			/* creating objects, for example, bullets */
#define        LEVELCONFLAG_EXTRACTCAMERA 13            /* causes level converter to extract camera */
																			/* information into the fields following this */
																			/* flag */
#define        BUTTON_CAMERA_REFERENCE    14
#define        BUTTON_LIGHT_REFERENCE     15
#define        LEVELCONFLAG_ROOM          16                    /* indicates this object should be treated as a room */
#define        LEVELCONFLAG_COMMONBLOCK   17            /* flags beginning of a common block */
#define        LEVELCONFLAG_ENDCOMMON     18            /* flags end of a common block */
#define        BUTTON_MESHNAME            19
#define        BUTTON_XDATA               20                    // levelcon reads the string field for this type
																			// to determine what to do with the text
																			// the string should be the name of a DLL which
																			// handles the text chunk and returns the data
																			// the def field is used to indicate to levelcon whether
																			// it should output the xdata into the commonblock

#define        BUTTON_EXTRACT_CAMERA      21
#define        LEVELCONFLAG_EXTRACTCAMERANEW 22 /* NEW CAMERA EXTRACTION causes level converter to extract camera */
																			/* information into the fields following this */
																			/* flag */
#define        BUTTON_WAVEFORM            23
#define        BUTTON_CLASS_REFERENCE     24
#define        BUTTON_GROUP_START         25
#define        BUTTON_GROUP_STOP          26
#define		   LEVELCONFLAG_EXTRACTLIGHT  27	/* Causes level converter to extract data from a 3DS MAX light object */
#define        LEVELCONFLAG_SHORTCUT      28
//} buttonType;
typedef char buttonType;

//typedef enum
//{
#define        SHOW_AS_N_A              0
#define        SHOW_AS_NUMBER           1                      // numbers
#define        SHOW_AS_SLIDER           2                      // numbers
#define        SHOW_AS_TOGGLE           3
#define        SHOW_AS_DROPMENU         4
#define        SHOW_AS_RADIOBUTTONS     5
#define        SHOW_AS_HIDDEN           6                      // anything
#define        SHOW_AS_COLOR            7                      // int32
#define			SHOW_AS_CHECKBOX		8
#define			SHOW_AS_MAILBOX			9
#define			SHOW_AS_COMBOBOX		10
#define			SHOW_AS_TEXTEDITOR		11
#define			SHOW_AS_FILENAME		12


#define			SHOW_AS_VECTOR			0x80
//} visualRepresentation;
typedef char visualRepresentation;

/*============================================================================*/
/* WARNING: if you change this array, you must update oadFlagNames in oad.cc in levelcon */

typedef enum
{                                           /* name used in levelcon: */
	OADFLAG_TEMPLATE_OBJECT,                                /*      "Template Object"     */
	OADFLAG_PERMANENT_TEXTURE,              /*      "Moves Between Rooms" */
	OADFLAG_NOMESH_OBJECT,
	OADFLAG_SIZEOF
} oadFlags;

/*============================================================================*/

typedef struct _oadHeader
{
	long chunkId;
	long chunkSize;
	char name[72-4];
	long version;
} oadHeader;

/*============================================================================*/

//typedef enum
//{
#define	XDATA_IGNORE						0
#define	XDATA_COPY                          1
#define	XDATA_OBJECTLIST                    2
#define	XDATA_CONTEXTUALANIMATIONLIST       3
#define	XDATA_SCRIPT                        4
#define	XDATA_CONVERSION_MAX                5
//} EConversionAction;
typedef char EConversionAction;


#if defined( _MSC_VER )
#pragma pack( push, 1 )
#else
#pragma pack( 1 )
#endif

typedef struct _typeDescriptor
{
	buttonType type;                                // float, int, fixed, string
	char name[64];                                  // label (and structure name)

	int32 min;								// ranged numbers
	int32 max;								// ranged numbers
	int32 def;								// number

	int16 len;                              // string only
	char string[512];

	visualRepresentation showAs;

	int16 x, y;

	char helpMessage[ 128 ];

	union
    {
		struct
		{
			EConversionAction conversionAction;
			int32 bRequired;
			char displayName[ 64 ];
			char szEnableExpression[ 128 ];
			int32 rollUpLength;
		} xdata;
		struct
		{
			char pad[ 255 ];
		} pad;
	};

	char lpstrFilter[ 512 ];

} typeDescriptor;

#if defined( _MSC_VER )
#pragma pack( pop )
#else
#pragma pack( )
#endif

/*============================================================================*/

typedef struct _oadFile
{
	oadHeader header;
	typeDescriptor types[1];
} oadFile;

/*============================================================================*/
#endif
/*============================================================================*/
