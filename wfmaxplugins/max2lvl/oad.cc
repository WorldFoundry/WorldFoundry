//============================================================================
// oad.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================

#include "global.hpp"
#include <iostream.h>
#include <math.h>
#include <string.h>
#include "hdump.hpp"

#include "oad.hpp"
#include "level.hpp"
#include "levelcon.hpp"

#include <eval/eval.h>

//============================================================================

extern Interface* gMaxInterface;

//============================================================================
// this loads from the .oad file on disk


bool
QObjectAttributeDataEntry::Load(istream& input, ostream& error)
{
	assert(!_compiledScript);
	assert(!_compiledScriptLength);
	assert(input.good());

	_typeDescriptor temp;
	input.read((char*)&temp, sizeof(_typeDescriptor));

	SetType(temp.type);

	AssertMessageBox(strlen(temp.string) < OAD_STRING_LENGTH,"string len of string exceeds max, len = " << strlen(temp.string) << " in oad entry named " << temp.name);
	AssertMessageBox(temp.len < OAD_STRING_LENGTH,"temp.len = " << temp.len << " in oad entry named " << temp.name);

	SetString(temp.string);
	SetName(temp.name);
	SetMin(temp.min);
	SetMax(temp.max);
	SetDef(temp.def);

    if( GetType() == BUTTON_FILENAME )
		assert(GetDef() == 0);

	SetXDataConversionAction(temp.xdata.conversionAction);
	SetXDataRequired(temp.xdata.bRequired);
	_scriptText = NULL;

	if(!input.good())
		return(false);

	if(
		!GetName().length() ||
		!(GetMin() <= GetMax()) ||
		!(GetDef() >= GetMin()) ||
		!(GetDef() <= GetMax())
	  )
	{
		 AssertMessageBox(0, "Bogus OAD entry found:" << endl << *this);
	}

	DBSTREAM3( cstats << "OADE:Load: Loading entry " << (*this) << endl; )
	assert(GetName().length());
	assert(GetMin() <= GetMax());
	assert(GetDef() >= GetMin());
	assert(GetDef() <= GetMax());

	return(true);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeDataEntry &oad)
{
	s << "QObjectAttributeDataEntry: " << endl;

	s << "  Type:  ";
	switch(oad.GetType())
	 {
		case BUTTON_FIXED16:
			s << "BUTTON_FIXED16";
			break;
		case BUTTON_FIXED32:
            s << "BUTTON_FIXED32";
			break;
		case BUTTON_INT8:
            s << "BUTTON_INT8";
			break;
		case BUTTON_INT16:
            s << "BUTTON_INT16";
			break;
		case BUTTON_INT32:
            s << "BUTTON_INT32";
			break;
		case BUTTON_STRING:
            s << "BUTTON_STRING";
			break;
		case BUTTON_OBJECT_REFERENCE:
            s << "BUTTON_OBJECT_REFERENCE";
			break;
		case BUTTON_FILENAME:
            s << "BUTTON_FILENAME";
			break;
		case LEVELCONFLAG_SHORTCUT:
			s << "LEVELCONFLAG_SHORTCUT";
			break;
		case LEVELCONFLAG_NOINSTANCES:
            s << "LEVELCONFLAG_NOINSTANCES";
			break;
		case LEVELCONFLAG_NOMESH:
			s << "LEVELCONFLAG_NOMESH";
			break;
		case BUTTON_PROPERTY_SHEET:
			s << "BUTTON_PROPERTY_SHEET";
			break;
		case 	LEVELCONFLAG_SINGLEINSTANCE:
			s << "LEVELCONFLAG_SINGLEINSTANCE";
			break;
		case 	LEVELCONFLAG_TEMPLATE:
			s << "LEVELCONFLAG_TEMPLATE";
			break;
		case 	LEVELCONFLAG_EXTRACTCAMERA:
			s << "LEVELCONFLAG_EXTRACTCAMERA";
			AssertMessageBox(0, "LEVELCONFLAG_EXTRACTCAMERA is no longer allowed!");
			break;
		case 	BUTTON_CAMERA_REFERENCE:
			s << "BUTTON_CAMERA_REFERENCE";
			break;
		case 	BUTTON_LIGHT_REFERENCE:
			s << "BUTTON_LIGHT_REFERENCE";
			break;
		case 	LEVELCONFLAG_ROOM:
			s << "LEVELCONFLAG_ROOM";
			break;
		case	LEVELCONFLAG_COMMONBLOCK:
			s << "LEVELCONFLAG_COMMONBLOCK";
			break;
		case	LEVELCONFLAG_ENDCOMMON:
			s << "LEVELCONFLAG_ENDCOMMON";
			break;
		case BUTTON_XDATA:
			s << "BUTTON_XDATA";
			break;

		case BUTTON_EXTRACT_CAMERA:
			s << "BUTTON_EXTRACT_CAMERA";
			break;
		case LEVELCONFLAG_EXTRACTCAMERANEW:
			s << "LEVELCONFLAG_EXTRACTCAMERANEW";
			break;
		case LEVELCONFLAG_EXTRACTLIGHT:
			s << "LEVELCONFLAG_EXTRACTLIGHT";
			break;
		case BUTTON_WAVEFORM:
			s << "BUTTON_WAVEFORM";
			break;
		case BUTTON_CLASS_REFERENCE:
			s << "BUTTON_CLASS_REFERENCE";
			break;
		case BUTTON_GROUP_START:
			s << "BUTTON_GROUP_START";
			break;
		case BUTTON_GROUP_STOP:
			s << "BUTTON_GROUP_STOP";
			break;

		default:
			s << "Unknown";
			cwarn << "Warning: object of unknown type <" << (int)oad.GetType() << "> found" << endl;
			break;
	 }
	s << " <" << (int)oad.GetType() << ">" << endl;
	s << "  Name:  " << oad.GetName() << "   (" << oad.GetName().length() << ")" << endl;

	if(oad.GetType() == BUTTON_FIXED32)
	{
		s << "  Min:   " << WF_SCALAR_TO_FLOAT(oad.GetMin()) << " (" << oad.GetMin()  << ")" << endl;
		s << "  Max:   " << WF_SCALAR_TO_FLOAT(oad.GetMax()) << " (" << oad.GetMax()  << ")" <<endl;

		s << "  Def:   " << WF_SCALAR_TO_FLOAT(oad.GetDef()) << " (" << oad.GetDef()  << ")" <<endl;
	}
	else
	{
		s << "  Min:   " << oad.GetMin() << endl;
		s << "  Max:   " << oad.GetMax() << endl;
        if ( oad.GetType() == BUTTON_FILENAME )
			s << hex;
		s << "  Def:   " << oad.GetDef() << endl;
		s << dec;
	}
	if(oad.GetString().length())
	{
		s << "  String:" << oad.GetString() << endl;
//		s << "  Len:   " << oad.len << endl;
		s << "  Len:   " << oad.GetString().length() << endl;
	}

	if (oad.GetType() == BUTTON_XDATA)
	{
//		s << "  XData ConversionAction: " << oad.GetXDataConversionAction() << endl;
		s << "  XData bRequired:        " << oad.GetXDataRequired() << endl << endl;
	}

	return(s);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeData &oad)
{
	s << "Oad Header Name:" << oad.header.name << endl;

	for(unsigned int i=0;i<oad.entries.size();i++)
	 {
		s << oad.entries[i] << endl;
	 }
	return(s);
}

//============================================================================

QObjectAttributeData::QObjectAttributeData()
{
}

//============================================================================

#define OAD_CHUNKID 'OAD '

bool
QObjectAttributeData::Load(istream& input, ostream& error)
{
	assert(input.good());

	// first, load the header and validate it
	input.read((char*)&header,sizeof(_oadHeader));
	if(header.chunkId != OAD_CHUNKID)
	 {
		error << "Incorrect header, is not an OAD file" << endl;
	 	return(false);
	 }
	DBSTREAM3( cstats << "Loading Object Header name <" << header.name << ">" << endl; )

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.good())
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
			DBSTREAM3( cdebug << "  added" << endl; )
		 }
		else
		 {
			if(input.good())
			 {
				AssertMessageBox(0, "QObjectAttributeData::Load: OAD entry load failed");
			 }
		 }
	 }
	DBSTREAM3( cdebug << "Loaded Object Header name <" << header.name << "> as :" << *this << endl; )
	return(true);
}

//============================================================================
// load from a stream which has no header

bool
QObjectAttributeData::LoadEntries(istream& input, ostream& error)
{
	assert(input.good());

	header.name[0] = 0;
	header.chunkSize = 0;
	header.chunkId = OAD_CHUNKID;

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.good())
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
		 }
	 }
	return(true);
}

//============================================================================

bool
QObjectAttributeData::ContainsButtonType(buttonType bType)
{
	bool found = false;
	for(unsigned int i=0;i<entries.size() && !found;i++)
	 {
//		cdebug << "QObjectAttributeData::ContainsButtonType: Checking entry <" << i << "> which is type <" << entries[i].type << "> against type <"<< LEVELCONFLAG_NOINSTANCES << ">" << endl;
		if(entries[i].GetType() == bType)
			found = true;
	 }
	return(found);
}

//============================================================================
// construct an int containing a bit field indicating which types are present in this OAD

struct oadFlagEntry
{
	char* fieldName;
	enum extractType
	{
		COPYDEF,				// copy def field into bit
		PRESENT					// set bit if field present
	};
	extractType et;
};


// this array of strings must match the oadFlags array in oad.h

oadFlagEntry oadFlagArray[OADFLAG_SIZEOF] =
{
	{"Template Object",oadFlagEntry::COPYDEF },
	{"Moves Between Rooms",oadFlagEntry::COPYDEF},
	{"LEVELCONFLAG_NOMESH",oadFlagEntry::PRESENT}
};

//----------------------------------------------------------------------------

int32
QObjectAttributeData::GetOADFlags(void)
{
	int32 flags = 0;
	const QObjectAttributeDataEntry* entry;
	for(int index=0;index<OADFLAG_SIZEOF;index++)
	{
	 	assert(oadFlagArray[index].fieldName);
		entry = GetEntryByName(oadFlagArray[index].fieldName);
		DBSTREAM3( if(entry) )
			DBSTREAM3( cdebug << "QOAD::GetOADFlags: <" << oadFlagArray[index].fieldName << "> field name found " << *entry << endl; )

		if(oadFlagArray[index].et == oadFlagEntry::PRESENT && entry )
		{
			flags |= (1<<index);
			DBSTREAM3( cdebug << "QOAD::GetOADFlags: <" << oadFlagArray[index].fieldName << "> flagged " << *entry << endl; )
		}

		if(oadFlagArray[index].et == oadFlagEntry::COPYDEF && entry && entry->GetDef())				// if exists and is not 0
		{
			flags |= (1<<index);
			DBSTREAM3( cdebug << "QOAD::GetOADFlags: <" << oadFlagArray[index].fieldName << "> flagged " << *entry << endl; )
		}
	}
	return(flags);
}

//============================================================================

size_t
QObjectAttributeData::SizeOfOnDisk(void) const
{
	size_t size = 0;
	bool isInCommonBlock = false;

	for(unsigned int index=0;index<entries.size();index++)
	{
		const QObjectAttributeDataEntry& tempEntry = entries[index];
		DBSTREAM3( cdebug << "QOAD::SizeOfOnDisk: Processing entry <" << tempEntry.GetName() << "> " << endl; )
		if (isInCommonBlock == false)
			size += entries[index].SizeOfOnDisk();
		if (tempEntry.GetType() == LEVELCONFLAG_COMMONBLOCK)
		{
			isInCommonBlock = true;
			DBSTREAM3( cdebug << "\tEntering COMMONBLOCK..." << endl; )
		}
		if (tempEntry.GetType() == LEVELCONFLAG_ENDCOMMON)
		{
			isInCommonBlock = false;
			DBSTREAM3( cdebug << "\tLeaving COMMONBLOCK..." << endl; )
		}
	}
//	DBSTREAM3( cdebug << "QOAD::SizeOfOnDisk: Done with <" << tempEntry.GetName() << ">; Size = " << size << " bytes." << endl; )
	return(size);
}

//============================================================================

// this applies an entry from the .prj file to an OAD in memory

bool
QObjectAttributeData::Apply(const char* buffer,int32 length, const char* objName)				// override current oad with fields from prj file
{
	bool error = false;
	DBSTREAM3( cdebug << "QObjectAttributeData::Apply(char*) called on object named " << objName << endl; )
	//DBSTREAM3( HDump((void*)buffer,length,4," ",cdebug); )
	DBSTREAM3( cdebug << "	before apply:" << endl << *this << endl; )
	while(length > 0)
	{
		DBSTREAM3( cdebug << "	top of loop" << endl; )
		// read size of this entry (in case we want to skip past it)
		long entrySize = *((long*)buffer);
		buffer+=4;
		length -= 4;

		// next read type
		long buttonType = *((long*)buffer);
		buffer+=4;
		length -= 4;

		// 'showAs' data
		long showAs = *((long*)buffer);
		buffer+=4;
		length -= 4;

		// now read name
		DBSTREAM1
		(
			if (strlen(buffer) <= 0)
				cdebug << "Warning:  Object <" << objName << "> contains an OAD entry with no name." << endl;
		)

		assert(strlen(buffer) < 32);
		const char* name = buffer;
		length -= strlen(buffer)+1;
		buffer += strlen(buffer)+1;

		int32 def = 0;
		int32 size = 0;
		const char* oadString = NULL;
		const char* scriptText = NULL;

		DBSTREAM3( cdebug << "	switching: buttonType = " << buttonType << ", name = " << name << endl; )
		switch ( buttonType )
		{
			case LEVELCONFLAG_SHORTCUT:
			{
                AssertMessageBox(0, "Object <" << objName << "> contains LEVELCONFLAG_SHORTCUT" );
				break;
			}

			case LEVELCONFLAG_COMMONBLOCK:
				def = *((int32*)buffer);
				size = 4;
				break;

			case BUTTON_FIXED32:
			{
				size = strlen( buffer ) + 1;
				double d = eval( buffer, NULL );
				def = long( d * 65536.0 );
				//def = atof( buffer ) * 65536.0;
				AssertMsg( def == long( atof( buffer ) * 65536.0 ), "buffer = <" << (char*)buffer << ">, def = " << def << ", atof = " << long( atof( buffer ) * 65536.0 ) );
				break;
			}

			case BUTTON_INT32:
			{
				DBSTREAM3( cdebug << "BUTTON_INT32:" << endl; )
				size = strlen( buffer ) + 1;
				DBSTREAM3( cdebug << "BUTTON_INT32: showas = " << showAs << ", buffer = <" << buffer << ">, size = " << size << endl; )
				switch ( showAs )
				{
					case SHOW_AS_N_A:
					case SHOW_AS_NUMBER:
					case SHOW_AS_HIDDEN:
					case SHOW_AS_COLOR:
					case SHOW_AS_CHECKBOX:
					case SHOW_AS_SLIDER:	// Should probably catch these formally and remove from .oad's
					default:
					{
						if(strlen(buffer))
						{
							//def = atol( buffer );
							def = long( eval( buffer, NULL ) );
							assert( def == atol( buffer ) );
						}
						else
							def = 0;
						break;
					}

					case SHOW_AS_DROPMENU:
					case SHOW_AS_RADIOBUTTONS:	// Cycle button, actually
					{
						oadString = buffer;
						break;
					}

					case SHOW_AS_MAILBOX:
					{
						assert( 0 );
						break;
					}

					case SHOW_AS_COMBOBOX:
					{
						assert( 0 );
						break;
					}

					case SHOW_AS_TOGGLE:
					{	// No longer coded
						assert( 0 );
						break;
					}
				}
				break;
			}

			case BUTTON_FILENAME:
			case BUTTON_STRING:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;
			case BUTTON_OBJECT_REFERENCE:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;
			case LEVELCONFLAG_NOINSTANCES:
				break;
			case LEVELCONFLAG_NOMESH:
				break;
			case BUTTON_PROPERTY_SHEET:
				size = 4;
				break;								// not used by us
			case LEVELCONFLAG_SINGLEINSTANCE:
				break;
			case LEVELCONFLAG_TEMPLATE:
				break;
			case BUTTON_CAMERA_REFERENCE:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;
			case BUTTON_LIGHT_REFERENCE:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;
			case LEVELCONFLAG_ROOM:
			case LEVELCONFLAG_ENDCOMMON:
				break;
			case BUTTON_XDATA:
				size = strlen(buffer)+1;
				scriptText = buffer;
				def = -1;
				break;
			case BUTTON_EXTRACT_CAMERA:
				break;
			case LEVELCONFLAG_EXTRACTCAMERANEW:		// this field burns no space
				break;
			case LEVELCONFLAG_EXTRACTLIGHT:
				break;
			case BUTTON_WAVEFORM:
				break;
			case BUTTON_CLASS_REFERENCE:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;

			case BUTTON_GROUP_START:
			case BUTTON_GROUP_STOP:
				break;

			case BUTTON_INT16:
			case BUTTON_FIXED16:
			case BUTTON_INT8:
			case LEVELCONFLAG_EXTRACTCAMERA:		// this field burns no space
				cerror << "QObjectAttributeDataEntry::Apply:Unknown oad type <" << buttonType << ">" << endl;
				AssertMessageBox(0, "Object " << objName << " in the .MAX file has bad data.  Re-edit it and try again.");
				break;
		}
		buffer += size;
		length -= size;

		DBSTREAM3( cdebug << "QObjectAttributeData::Apply: Entry:" << endl; )
		DBSTREAM3( cdebug << "    Type  = " << buttonType << endl; )
		DBSTREAM3( cdebug << "    Name = " << name << endl; )
		DBSTREAM3( cdebug << "    def = " << def << endl; )
		DBSTREAM3( if(oadString)
			cdebug << "    oadString = " << oadString << endl; )
		DBSTREAM3( cdebug << "    length = " << length << endl; )

		for(unsigned int entryIndex=0;entryIndex < entries.size();entryIndex++)
		{
//			DBSTREAM3( cdebug << "    entryIndex = " << entryIndex << endl; )
			const char* existingName = entries[entryIndex].GetName().c_str();
			if(										// if a field isn't named, it we be thrown away
				entries[entryIndex].GetName().length() &&
				strlen(name) &&
			    !strcmp(entries[entryIndex].GetName().c_str(), name) &&
				buttonType == entries[entryIndex].GetType()
			  )
			{
				DBSTREAM3( cdebug << "  Applying to entry: " << entries[entryIndex] << endl; )
				// kts only copy def and oadString
				QObjectAttributeDataEntry& td = entries[entryIndex];

				if ( buttonType == BUTTON_INT32
				&& ( ( showAs == SHOW_AS_DROPMENU ) || ( showAs == SHOW_AS_RADIOBUTTONS ) )
				)
				{	// Look up oadString in td's string enumeration
					// look up oadString in td.string
					int idxButton = 0;
					char szButtonNames[ 128 ];
					strcpy( szButtonNames, td.GetString().c_str() );
					char* pButtonName = szButtonNames;
					char* pEndButtonNames = szButtonNames + strlen( szButtonNames );
					while ( pButtonName != pEndButtonNames )
					{
						char* pSeparator = strchr( pButtonName, '|' );
						if ( pSeparator )
							*pSeparator = '\0';

						if ( strcmp( oadString, pButtonName ) == 0 )
						{
							def = idxButton;	// TODO: +td.GetMin()
							oadString = NULL;
							break;
						}

						pButtonName = pSeparator ? pSeparator + 1 : pEndButtonNames;
						++idxButton;
					}
					//TODO: assert not at end of list
				}

				td.SetDef(def);
				DBSTREAM3( cdebug << "after setdef" << endl; )

				AssertMessageBox( td.GetMin() <= td.GetMax() && td.GetDef() >= td.GetMin() && td.GetDef() <= td.GetMax(),
					"Invalid oad entry in object <" << objName << ">, field <" << td.GetName() << ">" << endl <<
					"Min = " << td.GetMin() << ", max = " << td.GetMax() << ", def = " << td.GetDef() );
				DBSTREAM3( cdebug << "calling set string" << endl; )
#pragma message ("kts put these asserts back in")
//				assert(td.min <= td.max);
//				assert(td.def >= td.min);
//				assert(td.def <= td.max);
//				strncpy(td.string,oadString,td.len);

				if(oadString)
					td.SetString(oadString);

                if( (td.GetType() == BUTTON_FILENAME) && (td.GetDef() == 0) )
	 			{
					DBSTREAM3( cdebug << "copying model type" << endl; )

					const QObjectAttributeDataEntry* modelType = GetEntryByName("Model Type");
					if(
						(td.GetName() == string("mesh name")) &&
                        ((modelType != NULL) && (modelType->GetDef()==MODEL_TYPE_MESH || modelType->GetDef()==MODEL_TYPE_SCARECROW))
                      )
					{
						assert(modelType->GetDef() < MODEL_TYPE_MAX);
						cerror << "Levelcon Error in apply: required asset <" << td.GetName() << "> missing from object (see next line)" << endl;
						error = true;
					}
	 			}

#if 1
// kts added 3/30/01 2:29PM
				// If this is a Script, copy the pointer to the actual script text
				if(td.GetType() == BUTTON_XDATA && 
					(
					(td.GetXDataConversionAction() == XDATA_SCRIPT)
					||(td.GetXDataConversionAction() == XDATA_COPY)
					)
			  	)
				{
					DBSTREAM3( cdebug << "Found a Script at <" << hex << (long)scriptText << dec << ">" << endl; )
					if(scriptText && strlen(scriptText))
					{
						DBSTREAM3( cdebug << "Found a Script in object " << objName << " which says " << scriptText << endl;  )
						td.SetScriptText(scriptText);
					}
				}
#else
				if(td.GetType() == BUTTON_XDATA && (td.GetXDataConversionAction() == XDATA_SCRIPT))						// if def > 0 then write out this xdata, if present
				{
					DBSTREAM3( cdebug << "Found a Script at <" << hex << (long)scriptText << dec << ">" << endl; )
					td.SetScriptText(scriptText);
				}
#endif

				DBSTREAM3( cdebug << "  Resulting entry:" << entries[entryIndex] << endl; )
				entryIndex = entries.size();
			}
		}
		DBSTREAM1( cprogress << "Done Applying entries." << endl; )
	}
	DBSTREAM1( cprogress << "left loop." << endl; )
	DBSTREAM1( cprogress << "length = " << length << endl; )
	DBSTREAM1( cprogress << "objName = " << objName << endl; )

    AssertMessageBox( length == 0, "Object <" << objName << "> has bad OAD data!" << endl << "(Length mismatch; ask a programmer.)" );
	DBSTREAM3( cdebug << "QObjectAttributeData::Apply(char*) done" << endl; )
	return error;
}

//============================================================================

const QObjectAttributeDataEntry*
QObjectAttributeData::GetEntryByName(const char* name) const
{
	QObjectAttributeData* that = (QObjectAttributeData*)this;
	for(unsigned int index=0;index<entries.size();index++)
	 {
		if(!strcmp(entries[index].GetName().c_str(),name))
		 {
		 	QObjectAttributeDataEntry* temp = &that->entries[index];
			return(temp);
		 }
	 }
	return(NULL);
}

//============================================================================

const QObjectAttributeDataEntry*
QObjectAttributeData::GetEntryByType(int type) const
{
	QObjectAttributeData* that = (QObjectAttributeData*)this;
	for(unsigned int index=0;index<entries.size();index++)
	 {
		if(entries[index].GetType() == type)
		 {
		 	QObjectAttributeDataEntry* temp = &that->entries[index];
			return(temp);
		 }
	 }
	return(NULL);
}

//============================================================================

QObjectAttributeDataEntry*
QObjectAttributeData::_GetEntryByName(const string& name)
{
	for(unsigned int index=0;index<entries.size();index++)
	 {
		if(!strcmp(entries[index].GetName().c_str(),name.c_str()))
		 {
			return(&entries[index]);
		 }
	 }
	return(NULL);
}

//============================================================================

void
WriteBytes(ostream& output, char* data, int count)
{
	for(int index=0;index<count;index++)
		output << data[index];
	DBSTREAM3( cdebug << "WriteBytes: writing " << count << endl; )
}

//============================================================================

int
QObjectAttributeDataEntry::operator==(const QObjectAttributeDataEntry& left) const
{
	return(!memcmp(this,&left,sizeof(QObjectAttributeDataEntry)));
}

//============================================================================

void
QObjectAttributeDataEntry::SetCompiledScript(const char* script, int32 length)
{
	assert(length);
	assert((length & 3) == 0);
	assert(script);
	assert(!_compiledScript);			// make sure we haven't already done this
	_compiledScript = (char*)malloc(length);
	assert(_compiledScript);
	memcpy(_compiledScript,script,length);
	_compiledScriptLength = length;
	assert(_compiledScriptLength);
}

//============================================================================

int
QObjectAttributeData::operator==(const QObjectAttributeData& left) const
{
	bool equal = true;

	if(entries.size() == left.entries.size())
	 {
		for(unsigned int index=0;index<entries.size();index++)
			if(!(entries[index] == left.entries[index]))
				equal = false;
	 }
	else
		equal = false;

	return
	 (
		header.chunkId == left.header.chunkId &&
		header.chunkSize == left.header.chunkSize &&
		!strcmp(header.name,left.header.name) &&
		equal
	 );
}

//============================================================================

void
QObjectAttributeData::SetFixed32(const string& name,fixed32 value,ostream& error)
{
	QObjectAttributeDataEntry* entry;
	DBSTREAM3( cdebug << "QObjectAttributeData::SetFixed32:QObjectAttributeData::SetFixed32: Updating <" << name << "> to contain <" << WF_SCALAR_TO_FLOAT(value) << ">" << endl; )
	entry = _GetEntryByName(name);
	if(!entry)
	 {
		error << "Cannot find .oad field named <" << name << endl;
		return;
	 }
	assert(entry);
	entry->SetDef(value);
}

//============================================================================

int32
QObjectAttributeData::GetOADValue(const string& name,ostream& error)
{
	QObjectAttributeDataEntry* entry;
	DBSTREAM3( cdebug << "QObjectAttributeData::GetOADValue: Reading <" << name << ">" << endl; )
	entry = _GetEntryByName(name);
	if(!entry)
	 {
		error << "QObjectAttributeData::GetOADValue:Cannot find .oad field named <" << name << ">" << endl;
		assert(0);
		return(-1);
	 }
	return(entry->GetDef());
}

//============================================================================

size_t
QObjectAttributeDataEntry::SizeOfOnDisk(void) const
{
	size_t size = 0;

	DBSTREAM3( cdebug << "QOADE::SizeOfOnDisk: Entry <" << GetName() << "> has type <" << (int)GetType() << "> " << endl; )

	switch(GetType())
	{
		case BUTTON_INT16:
		case BUTTON_FIXED16:
			size = 2;
			assert(0);
			break;
		case BUTTON_FIXED32:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_FIXED32 (" << size << " bytes)" << endl; )
			break;
		case BUTTON_INT32:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_INT32 (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_COMMONBLOCK:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_COMMONBLOCK (" << size << " bytes)" << endl; )
			break;
		case BUTTON_INT8:
			assert(0);
			size = 1;
			break;
		case BUTTON_STRING:
			assert(0);
//			size = len;
//			assert(!(len  & 1));			// len must be even
//			DBSTREAM3( cdebug << "\tand is a BUTTON_STRING (" << size << " bytes)" << endl; )
//			break;
		case BUTTON_OBJECT_REFERENCE:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_OBJECT_REFERENCE (" << size << " bytes)" << endl; )
			break;
		case BUTTON_FILENAME:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_FILENAME (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_SHORTCUT:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_SHORTCUT (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_NOINSTANCES:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_NOINSTANCES (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_NOMESH:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_NOMESH (" << size << " bytes)" << endl; )
			break;
		case BUTTON_PROPERTY_SHEET:
			DBSTREAM3( cdebug << "\tand is a BUTTON_PROPERTY_SHEET (" << size << " bytes)" << endl; )
			break;								// not used by us
		case LEVELCONFLAG_SINGLEINSTANCE:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_SINGLEINSTANCE (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_TEMPLATE:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_TEMPLATE (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_EXTRACTCAMERA:		// this field burns no space
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_EXTRACTCAMERA (" << size << " bytes)" << endl; )
			assert(0);			// no longer allowed
			break;
		case BUTTON_CAMERA_REFERENCE:
			DBSTREAM3( cdebug << "\tand is a BUTTON_CAMERA_REFERENCE (" << size << " bytes)" << endl; )
//			size = 4;
			break;
		case BUTTON_LIGHT_REFERENCE:
			DBSTREAM3( cdebug << "\tand is a BUTTON_LIGHT_REFERENCE (" << size << " bytes)" << endl; )
//			size = 4;
			break;
		case LEVELCONFLAG_ROOM:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_ROOM (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_ENDCOMMON:
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_ENDCOMMON (" << size << " bytes)" << endl; )
			break;
		case BUTTON_XDATA:
			if(GetXDataConversionAction() == XDATA_COPY ||
			   GetXDataConversionAction() == XDATA_OBJECTLIST ||
			   GetXDataConversionAction() == XDATA_SCRIPT ||
			   GetXDataConversionAction() == XDATA_CONTEXTUALANIMATIONLIST)					// if flag output
				size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_XDATA (" << size << " bytes)" << endl; )
			break;
		case BUTTON_EXTRACT_CAMERA:
			DBSTREAM3( cdebug << "\tand is a BUTTON_EXTRACT_CAMERA (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_EXTRACTCAMERANEW:		// this field burns no space
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_EXTRACTCAMERANEW (" << size << " bytes)" << endl; )
			break;
		case LEVELCONFLAG_EXTRACTLIGHT:			// this field burns no space
			DBSTREAM3( cdebug << "\tand is a LEVELCONFLAG_EXTRACTLIGHT (" << size << " bytes)" << endl; )
			break;
		case BUTTON_WAVEFORM:
			DBSTREAM3( cdebug << "\tand is a BUTTON_WAVEFORM (" << size << " bytes)" << endl; )
			break;
		case BUTTON_CLASS_REFERENCE:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_CLASS_REFERENCE (" << size << " bytes)" << endl; )
			break;
		case BUTTON_GROUP_START:
			DBSTREAM3( cdebug << "\tand is a BUTTON_GROUP_START (" << size << " bytes)" << endl; )
			break;
		case BUTTON_GROUP_STOP:
			DBSTREAM3( cdebug << "\tand is a BUTTON_GROUP_STOP (" << size << " bytes)" << endl; )
			break;
		default:
			cerror << "QObjectAttributeDataEntry::SizeOfOnDisk:Unknown oad type <" << (int)GetType() << ">" << endl;
			DBSTREAM3( cdebug << "\tand is of UNKNOWN TYPE!" << endl; )
			break;
	}

	DBSTREAM3( cdebug << "QOADE::SizeOfOnDisk: Size was " << size << endl << "\tDone with entry." << endl; )
	return(size);
}

//============================================================================

bool
QObjectAttributeData::SaveStruct(ostream& output, const QLevel& level,
								 const char* name, ostream& error)
{
	bool isInCommonBlock = false;
	int totalBytesWritten = 0;

	DBSTREAM3( cdebug << "QObjectAttributeData::SaveStruct: saving " << entries.size() << " entries" << endl; )
	for(unsigned int entryIndex=0;entryIndex < entries.size(); entryIndex++)
	{
		const QObjectAttributeDataEntry& td = entries[entryIndex];
		DBSTREAM3( cdebug << "QObjectAttributeData::SaveStruct: td type = <" << (int)td.GetType() << ">" << ", Name = <" << td.GetName() << "> " << endl; )
		DBSTREAM3( cdebug << endl; )

		switch(td.GetType())
		{
			case BUTTON_INT16:
			case BUTTON_FIXED16:
				assert(0); 		// not allowed
				break;
			case BUTTON_FIXED32:
			case BUTTON_INT32:
				if (isInCommonBlock == false)
				{
					DBSTREAM3( cdebug << "QOAD::SaveStruct::BUTTON_FIXED32/INT32: writting value of " << td.GetDef() << endl; )
					int32 tempDef =td.GetDef();
					WriteBytes(output,(char*)&tempDef,4);
					totalBytesWritten += 4;
					DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
//					cdebug << "SaveStruct: Writing 4 bytes..." << endl;
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.GetName() << ">." << endl; )
				break;
			case BUTTON_INT8:
				assert(0);
				break;
			case BUTTON_STRING:
				assert(0);
//				if (!isInCommonBlock)
//				{
//					for(int stringIndex=0;stringIndex<td.len;stringIndex++)
//						output << td.string[stringIndex];
//				}
//				else
//					DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.name << ">." << endl; )
				break;
			case BUTTON_OBJECT_REFERENCE:
			{
				if (!isInCommonBlock)
				{
					int32 index = 0;				// kts changed 10/30/96 8:09PM from -1
					if(td.GetString().length() != 0)
					{
						index = level.FindObjectIndex(td.GetString());
						DBSTREAM3( cdebug << "QOAD::SaveStruct::OBJECT_REFERENCE: looking up object <" << td.GetString() << ">, index = <" << index << ">" << endl; )
						if(index < 0)
						{
							error << "object <" << td.GetString() << "> refered to by object <" << name << "> not found" << endl;
							DBSTREAM3( cdebug << "Object Name List:" << endl; )
							DBSTREAM3( level.PrintObjectList(cdebug); )
						}
					}
					DBSTREAM3( else )
						DBSTREAM3( cdebug << "LevelCon Warning: object reference <" << td.GetName() << "> contains empty string in object <" << name << ">" << endl; )

					if (isInCommonBlock == false)
					{
						DBSTREAM3( cdebug << "QOAD::SaveStruct::OBJECT_REFERENCE: writting out index of <" << index << ">" << endl; )
						WriteBytes(output,(char*)&index,4);
						totalBytesWritten += 4;
						DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
					}
					DBSTREAM3( else )
						DBSTREAM3( cdebug << "QOAD::SaveStruct::OBJECT_REFERENCE: is in common block, doing nothing" << endl; )
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.GetName() << ">." << endl; )
				break;
			}
			case BUTTON_FILENAME:
			{
				if (!isInCommonBlock)
				{
					DBSTREAM3( cdebug << "SaveStruct::BUTTON_FILENAME: saving 4-byte asset ID <" << hex << td.GetDef() << dec << ">" << endl; )
					int32 tempDef = td.GetDef();
					WriteBytes(output,(char*)&tempDef,4);
					totalBytesWritten += 4;
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.GetName() << ">." << endl; )
				break;
			}
			case LEVELCONFLAG_SHORTCUT:
				assert( 0 );
				break;
			case LEVELCONFLAG_NOINSTANCES:
				break;
			case LEVELCONFLAG_NOMESH:
				break;
			case BUTTON_PROPERTY_SHEET:
				break;								// not used by us
			case LEVELCONFLAG_SINGLEINSTANCE:
				break;
			case LEVELCONFLAG_TEMPLATE:
				break;
			case LEVELCONFLAG_EXTRACTCAMERA:
			{
				AssertMessageBox(0, "LEVELCONFLAG_EXTRACTCAMERA no longer supported");
				break;
			}
			case BUTTON_CAMERA_REFERENCE:
				break;
			case BUTTON_LIGHT_REFERENCE:
				break;
			case LEVELCONFLAG_ROOM:
				break;
			case LEVELCONFLAG_EXTRACTCAMERANEW:
			{
				break;
			}
			case LEVELCONFLAG_EXTRACTLIGHT:
			{
				break;
			}
			case LEVELCONFLAG_COMMONBLOCK:
			{
				isInCommonBlock = true;
				int32 tempDef = td.GetDef();
				WriteBytes(output,(char*)&tempDef,4);
				totalBytesWritten += 4;
				DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
				break;
			}
			case LEVELCONFLAG_ENDCOMMON:
				isInCommonBlock = false;
				break;
			case BUTTON_XDATA:
				DBSTREAM3( cdebug <<"QOAD::SaveStruct: XDATA: xdata = " << (int)td.GetXDataConversionAction() << endl; )
				if(td.GetXDataConversionAction() == XDATA_COPY ||
				   td.GetXDataConversionAction() == XDATA_SCRIPT ||
				   td.GetXDataConversionAction() == XDATA_OBJECTLIST ||
				   td.GetXDataConversionAction() == XDATA_CONTEXTUALANIMATIONLIST)					// if output flag
				{
					if (!isInCommonBlock)
					{
//						DBSTREAM3( cdebug <<"QOAD::SaveStruct: XDATA: writting " << td.len << endl; )
						int32 tempDef = td.GetDef();
						WriteBytes(output,(char*)&tempDef,4);
						totalBytesWritten += 4;
						DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
					}
					DBSTREAM3(else)
						DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.GetName() << ">." << endl; )
				}
				break;
			case BUTTON_EXTRACT_CAMERA:
				break;
			case BUTTON_WAVEFORM:
				break;
			case BUTTON_CLASS_REFERENCE:
			{
				int32 typeIndex = -1;
				if(td.GetString().length() != 0)
				{
					typeIndex = level.GetClassIndex(td.GetString());
					AssertMessageBox(typeIndex != -1, "Object reference type of <" << td.GetString() << "> in object <" << name << "> not found in .LC file");
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "LevelCon Warning: class reference <" << td.GetName() << "> contains empty string in object <" << name << ">" << endl; )

				if (!isInCommonBlock)
				{
					DBSTREAM3( cdebug << "QOAD::SaveStruct::CLASS_REFERENCE: writting out index of <" << typeIndex << ">" << endl; )
					WriteBytes(output,(char*)&typeIndex,4);
					totalBytesWritten += 4;
					DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "QOAD::SaveStruct::CLASS_REFERENCE: is in common block, doing nothing" << endl; )
				break;
			}
			case BUTTON_GROUP_START:
				break;
			case BUTTON_GROUP_STOP:
				break;

			default:
				error << "QObjectAttributeData::SaveStruct:Unknown oad type <" << (int)td.GetType() << ">" << endl;
				break;
		}
	}
	return(true);
}

//============================================================================
