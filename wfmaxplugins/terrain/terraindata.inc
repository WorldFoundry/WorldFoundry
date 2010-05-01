//=============================================================================
// terraindata.inc:  all param block / ui data goes here
//=============================================================================

//--- Parameter map/block descriptors -------------------------------
// The parameter map descriptors define the properties of a parameter
// such as the type (spinner, radio button, check box, etc.), which
// resource ID they refer to, and which index into the virtual array
// they use.

// Parameter block indices

enum
{
	PB_WIDTH,
	PB_LENGTH,
	PB_WIDTHSEGS,
	PB_LENGTHSEGS,
	PB_SMOOTH,
	PB_MULTI_MATERIAL,
	PB_GENUVS,
	PB_RESTRICTTOPOWEROF2,
	PB_FIXEDSEGMENTSIZE
};

// Non-parameter block indices
enum
{
	PB_TI_POS,
//	PB_TI_RADIUS,
	PB_TI_WIDTH,
	PB_TI_LENGTH
};

//=============================================================================
// Type in

static ParamUIDesc descTypeIn[] =
{
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		-99999999.0f,99999999.0f,
		SPIN_AUTOSCALE),

	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_TI_WIDTH,IDC_TI_WIDTHSPINNER,
		MIN_SIZE,MAX_SIZE,
		SPIN_AUTOSCALE),
	// LENGTH
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_TI_LENGTH,IDC_TI_LENGTHSPINNER,
		MIN_SIZE,MAX_SIZE,
		SPIN_AUTOSCALE)

	};
#define PARAMDESC_KEYBOARD_ENTRIES 3

//-----------------------------------------------------------------------------
// this must have PARAMDESC_ENTRIES

static ParamUIDesc paramUIDesc[] =
{
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTH,IDC_WIDTHSPINNER,
		MIN_SIZE,MAX_SIZE,
		SPIN_AUTOSCALE),

	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTH,IDC_LENGTHSPINNER,
		MIN_SIZE,MAX_SIZE,
		SPIN_AUTOSCALE),

	// Width Segments
	ParamUIDesc(
		PB_WIDTHSEGS,
		EDITTYPE_INT,
		IDC_WIDTHSEGMENTS,IDC_WIDTHSEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Length Segments
	ParamUIDesc(
		PB_LENGTHSEGS,
		EDITTYPE_INT,
		IDC_LENGTHSEGMENTS,IDC_LENGTHSEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Smooth
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OBSMOOTH),

	// Multi-Material
	ParamUIDesc(PB_MULTI_MATERIAL,TYPE_SINGLECHEKBOX,IDC_MULTI_MATERIAL),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),

	// Restrict to Power of 2
	ParamUIDesc(PB_RESTRICTTOPOWEROF2,TYPE_SINGLECHEKBOX,IDC_RESTRICTTOPOWEROF2),

	// Fixed Segment Size
	ParamUIDesc(PB_FIXEDSEGMENTSIZE,TYPE_SINGLECHEKBOX,IDC_FIXEDSEGMENTSIZE),
	};

#define PARAMDESC_ENTRIES 9

//------------------------------------------------------------------------------
// this must have PARAMDESC_ENTRIES

static ParamBlockDescID paramBlockDescID[] =
{
	{ TYPE_FLOAT, NULL, TRUE, 0 },		// X Size
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// Y Size
	{ TYPE_INT, NULL, TRUE, 2 },		// width segments
	{ TYPE_INT, NULL, TRUE, 3 },		// length segments
	{ TYPE_BOOL, NULL, TRUE, 4 },		// smooth
	{ TYPE_INT, NULL, FALSE, 5 },		// multi_material
	{ TYPE_INT, NULL, FALSE, 6 },		// gen UV's
	{ TYPE_INT, NULL, FALSE, 7 },		// power of 2
	{ TYPE_INT, NULL, FALSE, 8 }		// lock segment size
};

#define CURRENT_VERSION	0

//=============================================================================
// Parameters
// this must have PARAMDESC_ENTRIES + PARAMDESC_KEYBOARD_ENTRIES

int one = 1;

ParameterEntry cycloneTerrainParameterEntryArray[] =
{
	// Width
	ParameterEntry(
		"Width",
		stdWorldDim,
		TypeEntry(&CycloneTerrainObject::crtWidth)
	),
	// Length
	ParameterEntry(
		"Length",
		stdWorldDim,
		TypeEntry(&CycloneTerrainObject::crtLength)
	),
	// width segments
	ParameterEntry(
		"Width Segments",
		stdSegmentsDim,
		&CycloneTerrainObject::dlgWidthSegments
	),
	// length segments
	ParameterEntry(
		"Length Segments",
		stdSegmentsDim,
		&CycloneTerrainObject::dlgLengthSegments
	),
	// smooth
	ParameterEntry(
		"Smooth",
		stdNormalizedDim,
		&CycloneTerrainObject::dlgSmooth
	),
	// multi-material	(not implemented)
	ParameterEntry(
		"Not Implemented",
		defaultDim,
		&one
	),
	// genuvs	(not implemented)
	ParameterEntry(
		"Not Implemented",
		defaultDim,
		&one
	),
	// powerof2
	ParameterEntry(
		"Restrict to Power Of 2",
		stdNormalizedDim,
		&CycloneTerrainObject::dlgRestrictToPowerOf2
	),
	// fixed segment size
	ParameterEntry(
		"Fixed Segment Size",
		stdNormalizedDim,
		&CycloneTerrainObject::dlgFixedSegmentSize
	),
	// from keyboard dialog
	// pos	(not implemented)
	ParameterEntry(
		"Not Implemented",
		defaultDim,
		TypeEntry(TypeEntryInvalid())
	),
	// length	(not implemented)
	ParameterEntry(
		"Not Implemented",
		defaultDim,
		TypeEntry(TypeEntryInvalid())
	),
	// width	(not implemented)
	ParameterEntry(
		"Not Implemented",
		defaultDim,
		TypeEntry(TypeEntryInvalid())
	)
};

#define PARAMETER_ENTRIES PARAMDESC_ENTRIES+PARAMDESC_KEYBOARD_ENTRIES

//=============================================================================
// previous version handling, enable when there is more than one version
//=============================================================================
// The parameter block descriptor defines the type of value represented
// by the parameter (int, float, Color...) and if it is animated or not.

// This class requires these values to be initialized:
// { - ParameterType,
//   - Not Used, must be set to NULL,
//   - Flag which indicates if the parameter is animatable,
//   - ID of the parameter used to match a corresponding ID in the
//     other version of the parameter.  This is used to automatically
//     convert between and older version of the parameter block
//     structure and a newer one.
//  }

// This one is the older version.
#if 0
static ParamBlockDescID descVer0[] =
{
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 }
};

static ParamBlockDescID descVer1[] =
{
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 }
};

// Array of old versions
static ParamVersionDesc versions[] =
{
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,6,1)
};
#define NUM_OLDVERSIONS	2

// Current version
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,0);
#define CURRENT_VERSION	0
#endif

//=============================================================================

