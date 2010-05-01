// oadentry.cpp

#include "global.hpp"
#include "oadentry.hpp"



OadEntry::OadEntry( char* & xdata )
{
	memset( &_td, 0, sizeof( _td ) );

	char* pFieldStart = xdata;	// Remember starting place in case we need to skip over the "chunk"

	int size = *((int*)xdata);
	xdata += sizeof( int );

	_td.type = *((int*)xdata);
	xdata += sizeof( int );

	_td.showAs = *((int*)xdata);
	xdata += sizeof( int );

	strcpy( _td.name, (const char*)xdata );
	xdata += strlen( _td.name ) + 1;

	_id = buttonTypeToID();

	if ( _td.type == BUTTON_XDATA )
	{
		_xdata = strdup( xdata );
		assert( _xdata );
	}
	else
	{
		strcpy( _td.string, xdata );
		_xdata = NULL;
	}
	xdata += strlen( xdata ) + 1;

	//assert( xdata == pFieldStart + size );
	xdata = pFieldStart + size;
	//_hasBeenWrittenTo = FALSE;
	_hasBeenWrittenTo = TRUE;

//	(*debuglog) << "xdata dump:" << endl;
//	(*debuglog) << "  size = " << size << endl;
//	(*debuglog) << "  all of it = " << *this << endl;
}


OadEntry::OadEntry( const typeDescriptor& td )
{
	//DEBUGLOG((*debuglog) << "\nOadEntry::OadEntry::typeDescriptor:" << endl;)
	_td = td;
	if ( _td.type == BUTTON_XDATA )
	{
		_xdata = strdup( "" );
		assert( _xdata );
	}
	else
	{
		_xdata = NULL;
	}

	if ( _td.type == BUTTON_FIXED32 )
		sprintf( _td.string, "%g", _td.def / 65536.0 );

	_id = buttonTypeToID();

	_szEnumeratedValues = strdup( _td.string );
	assert( _szEnumeratedValues );
	_hasBeenWrittenTo = FALSE;
	//_hasBeenWrittenTo = TRUE;
	//DEBUGLOG((*debuglog) << *this << endl;)
}


OadEntry::OadEntry( const OadEntry& oe )
{
	*this = oe;

	_szEnumeratedValues = NULL;
	if ( oe._szEnumeratedValues )
	{
		_szEnumeratedValues = strdup( oe._szEnumeratedValues );
		assert( _szEnumeratedValues );
	}

	_xdata = NULL;
	if ( oe._xdata )
	{
		_xdata = strdup( oe._xdata );
		assert( _xdata );
	}
}


OadEntry::OadEntry()
{
	_xdata = NULL;
	_szEnumeratedValues = NULL;
	_hasBeenWrittenTo = FALSE;
}


OadEntry::~OadEntry()
{
	if ( _xdata )
		free( _xdata ), _xdata = NULL;

	if ( _szEnumeratedValues )
		free( _szEnumeratedValues ), _szEnumeratedValues = NULL;
}

#define BUTTON_TYPE_TO_NAME( __buttonType__, __name__ ) \
	case __buttonType__: szButtonName = __name__; break;

const char*
OadEntry::buttonTypeToName() const
{
	const char* szButtonName = NULL;

	switch ( _td.type )
	{
		case BUTTON_FIXED16:
		case BUTTON_INT8:
		case BUTTON_INT16:
		case BUTTON_WAVEFORM:
			assert( 0 );
			break;

		case BUTTON_GROUP_START:
		case BUTTON_GROUP_STOP:
		case BUTTON_EXTRACT_CAMERA:
		case LEVELCONFLAG_EXTRACTCAMERANEW:
		case LEVELCONFLAG_NOINSTANCES:
		case LEVELCONFLAG_NOMESH:
		case LEVELCONFLAG_SINGLEINSTANCE:
		case LEVELCONFLAG_TEMPLATE:
		case LEVELCONFLAG_EXTRACTCAMERA:
		case LEVELCONFLAG_ROOM:
		case LEVELCONFLAG_COMMONBLOCK:
		case LEVELCONFLAG_ENDCOMMON:
			assert( 0 );
			szButtonName = "N/A";
			break;

		case BUTTON_PROPERTY_SHEET:
		{
			break;
		}

		BUTTON_TYPE_TO_NAME( BUTTON_INT32, "Int32" )
		BUTTON_TYPE_TO_NAME( BUTTON_FIXED32, "Fixed32" )
		BUTTON_TYPE_TO_NAME( BUTTON_STRING, "String" )
		BUTTON_TYPE_TO_NAME( BUTTON_OBJECT_REFERENCE, "Object Reference" )
		BUTTON_TYPE_TO_NAME( BUTTON_FILENAME, "Filename" )
		BUTTON_TYPE_TO_NAME( BUTTON_CAMERA_REFERENCE, "Camera Reference" )
		BUTTON_TYPE_TO_NAME( BUTTON_LIGHT_REFERENCE, "Light Reference" )
		BUTTON_TYPE_TO_NAME( BUTTON_MESHNAME, "Filename" )
		BUTTON_TYPE_TO_NAME( BUTTON_CLASS_REFERENCE, "Class Reference" )
		BUTTON_TYPE_TO_NAME( BUTTON_XDATA, "String" )

		default:
			assert( 0 );
			break;
	}

	return szButtonName;
}

#define BUTTON_TYPE_TO_ID( __buttonType__, __id__ ) \
	case __buttonType__: id = __id__; break;

ID
OadEntry::buttonTypeToID() const
{
	ID id = ID( 0UL );

	switch ( _td.type )
	{
		case BUTTON_FIXED16:
		case BUTTON_INT8:
		case BUTTON_INT16:
		case BUTTON_WAVEFORM:
			assert( 0 );
			break;

		case BUTTON_GROUP_START:
		case BUTTON_GROUP_STOP:
		case BUTTON_EXTRACT_CAMERA:
		case LEVELCONFLAG_EXTRACTCAMERANEW:
		case LEVELCONFLAG_NOINSTANCES:
		case LEVELCONFLAG_NOMESH:
		case LEVELCONFLAG_SINGLEINSTANCE:
		case LEVELCONFLAG_TEMPLATE:
		case LEVELCONFLAG_EXTRACTCAMERA:
		case LEVELCONFLAG_ROOM:
		case LEVELCONFLAG_COMMONBLOCK:
		case LEVELCONFLAG_ENDCOMMON:
			//assert( 0 );			// Should not have been stored
			id = ID( "N/A" );
			break;

		case BUTTON_PROPERTY_SHEET:
		{
			break;
		}

		BUTTON_TYPE_TO_ID( BUTTON_INT32, ID( "I32" ) )
		BUTTON_TYPE_TO_ID( BUTTON_FIXED32, ID( "FX32" ) )
		BUTTON_TYPE_TO_ID( BUTTON_STRING, ID( "STR" ) )
		BUTTON_TYPE_TO_ID( BUTTON_OBJECT_REFERENCE, ID( "STR" ) )
		BUTTON_TYPE_TO_ID( BUTTON_FILENAME, ID( "FILE" ) )
		BUTTON_TYPE_TO_ID( BUTTON_CAMERA_REFERENCE, ID( "STR" ) )
		BUTTON_TYPE_TO_ID( BUTTON_LIGHT_REFERENCE, ID( "STR" ) )
		BUTTON_TYPE_TO_ID( BUTTON_MESHNAME, ID( "FILE" ) )
		BUTTON_TYPE_TO_ID( BUTTON_CLASS_REFERENCE, ID( "STR" ) )
		BUTTON_TYPE_TO_ID( BUTTON_XDATA, ID( "STR" ) )

		default:
		{
			break;
		}
	}

	return id;
}

bool
OadEntry::operator==( const OadEntry& oadEntry ) const
{
	return (
		( strcmp( GetName(), oadEntry.GetName() ) == 0 )
		&& ( GetButtonType() == oadEntry.GetButtonType() )
	);
}


const char*
OadEntry::GetName() const
{
	return _td.name;
}


int32
OadEntry::GetMin() const
{
	return _td.min;
}


int32
OadEntry::GetMax() const
{
	return _td.max;
}


int32 
OadEntry::GetDef() const
{
	return _td.def;
}


void 
OadEntry::SetDef(int32 def)
{
	_hasBeenWrittenTo = TRUE;
	_td.def = def;
}


buttonType
OadEntry::GetButtonType() const
{
	return _td.type;
}


const char*
OadEntry::GetDisplayName() const
{
	return _td.xdata.displayName;
}


const char*
OadEntry::GetEnableExpression() const
{
	return _td.xdata.szEnableExpression;
}


visualRepresentation
OadEntry::GetVisualRepresentation() const
{
	return _td.showAs;
}


const char*
OadEntry::GetString() const
{
//	return _td.string;
	return _td.type == BUTTON_XDATA ? _xdata : _td.string;
}

void 
OadEntry::SetString(const char* string)
{
	assert(this);
	_hasBeenWrittenTo = TRUE;
	assert(string);
	assert(strlen(string) < 512);
	strcpy(_td.string,string);
}



const ID
OadEntry::GetID() const
{
	return _id;
}


typeDescriptor*
OadEntry::GetTypeDescriptor()
{
	_hasBeenWrittenTo = TRUE;
	return &_td;
}


const typeDescriptor*
OadEntry::GetTypeDescriptor() const
{
	return &_td;
}

const char* 
OadEntry::GetXData() const
{
	return _xdata;
}
	
void 
OadEntry::SetXData(const char* xdata)
{
	assert( xdata );
	_hasBeenWrittenTo = TRUE;
	if(_xdata)
		free( _xdata );
	_xdata = strdup( xdata );
	assert( _xdata );
}


bool 
OadEntry::DefaultOverriden() const
{
	return _hasBeenWrittenTo;
}



ostream&
operator << ( ostream& s, const OadEntry& oadEntry )
{
	s << "OadEntry Dump:" << endl;
	s << "Name = " << oadEntry.GetName() << endl;
	s << "--------------------" << endl;
	s << "Display Name = " << oadEntry.GetDisplayName() << endl;
	s << "Button Type = " << int( oadEntry.GetButtonType() ) << endl;
	s << "ID =" << oadEntry._id << endl;
	if(oadEntry._xdata)
		s << "xdata = " << oadEntry._xdata << endl;
	if(oadEntry._szEnumeratedValues)				   
		s << "EnumeratedValues = " << oadEntry._szEnumeratedValues << endl;

	s << "min = " << oadEntry._td.min << ", max = " << oadEntry._td.max << ", def = " << oadEntry._td.def << endl;
	s << "help message = " << oadEntry._td.helpMessage << endl;
	s << "string = " << oadEntry._td.string << endl;
	s << "x = " << oadEntry._td.x << ", y = " << oadEntry._td.y << endl;
	s << "showas = " << int(oadEntry._td.showAs) << endl;
	s << "filter = " << oadEntry._td.lpstrFilter << endl;
	s << "hasBeenWrittenTo = " << oadEntry._hasBeenWrittenTo << endl;
	return s;
}
