//============================================================================
// oad.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================

#include <pclib/boolean.hp>
#include <iostream.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <pclib/hdump.hp>

#include "oad.hp"
#include "level.hp"
#include "levelcon.hp"
#include "file3ds.hp"

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================
// this loads from the .oad file on disk


boolean
QObjectAttributeDataEntry::Load(istream& input, ostream& error)
{
	assert(!_compiledScript);
	assert(!_compiledScriptLength);
	assert(input.rdstate() == ios::goodbit);
	assert(error.rdstate() == ios::goodbit);

	_typeDescriptor temp;
	input.read((char*)&temp, sizeof(_typeDescriptor));

	SetType(temp.type);

	AssertMsg(strlen(temp.string) < OAD_STRING_LENGTH,"string len of string exceeds max, len = " << strlen(temp.string) << " in oad entry named " << temp.name);
	AssertMsg(temp.len < OAD_STRING_LENGTH,"temp.len = " << temp.len << " in oad entry named " << temp.name);
//	AssertMsg(strlen(temp.string) <= temp.len,"string len exceeds len, strlen = " << strlen(temp.string) << ", len = " << temp.len << " in oad entry named " << temp.name);

	SetString(temp.string);
	SetName(temp.name);
	SetMin(temp.min);
	SetMax(temp.max);
	SetDef(temp.def);

	if( ((GetType() == BUTTON_MESHNAME) || (GetType() == BUTTON_FILENAME))  )
		assert(GetDef() == 0);

//	if( ((type == BUTTON_MESHNAME) || (type == BUTTON_FILENAME)) && (def == 0) )
//	{
//		cdebug << "OADE::Load claims required asset <" << name << "> missing from this entry:" << endl;
//		cdebug << *this << endl;
//		exit(1);
//	}

	SetXDataConversionAction(temp.xdata.conversionAction);
	SetXDataRequired(temp.xdata.bRequired);

//	assert(sizeof(typeDescriptor) == sizeof(oadTypeDescriptor));
//	memcpy((void*)&entry,(void const *)&temp,sizeof(oadTypeDescriptor));
//	entry = (typeDescriptor)temp;

	if(input.rdstate() != ios::goodbit)
	 {
		return(boolean::BOOLFALSE);
	 }

	if(
		!GetName().length() ||
		!(GetMin() <= GetMax()) ||
		!(GetDef() >= GetMin()) ||
		!(GetDef() <= GetMax())
	  )
	 {
//		if(strlen(entry.name))
			cerror << "Levelcon Warning: bogus OAD entry found:" << endl << *this << endl;
		 assert(0);
//		return(boolean::BOOLFALSE);
	 }

	DBSTREAM3( cstats << "OADE:Load: Loading entry " << (*this) << endl; )
	assert(GetName().length());
	assert(GetMin() <= GetMax());
	assert(GetDef() >= GetMin());
	assert(GetDef() <= GetMax());

	return(boolean::BOOLTRUE);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeDataEntry &oad)
{
//	assert(s.rdstate() == ios::goodbit);

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
			assert(0);			// no longer allowed
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
		case BUTTON_MESHNAME:
			s << "BUTTON_MESHNAME";
			break;
		case BUTTON_XDATA:
			s << "BUTTON_XDATA";
			break;

		case 	BUTTON_EXTRACT_CAMERA:
			s << "BUTTON_EXTRACT_CAMERA";
			break;
		case 	LEVELCONFLAG_EXTRACTCAMERANEW:
			s << "LEVELCONFLAG_EXTRACTCAMERANEW";
			break;
		case 	BUTTON_WAVEFORM:
			s << "BUTTON_WAVEFORM";
			break;
		case 	BUTTON_CLASS_REFERENCE:
			s << "BUTTON_CLASS_REFERENCE";
			break;
		case 	BUTTON_GROUP_START:
			s << "BUTTON_GROUP_START";
			break;
		case 	BUTTON_GROUP_STOP:
			s << "BUTTON_GROUP_STOP";
			break;

		default:
			s << "Unknown";
			cwarn << "Warning: object of unknown type <" << oad.GetType() << "> found" << endl;
			break;
	 }
	s << " <" << oad.GetType() << ">" << endl;
	s << "  Name:  " << oad.GetName() << "   (" << oad.GetName().length() << ")" << endl;

	if(oad.GetType() == BUTTON_FIXED32)
	{
		s << "  Min:   " << BrScalarToFloat(oad.GetMin()) << " (" << oad.GetMin()  << ")" << endl;
		s << "  Max:   " << BrScalarToFloat(oad.GetMax()) << " (" << oad.GetMax()  << ")" <<endl;

		s << "  Def:   " << BrScalarToFloat(oad.GetDef()) << " (" << oad.GetDef()  << ")" <<endl;
	}
	else
	{
		s << "  Min:   " << oad.GetMin() << endl;
		s << "  Max:   " << oad.GetMax() << endl;
		if ( (oad.GetType() == BUTTON_MESHNAME) && (oad.GetType() == BUTTON_FILENAME) )
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

	return(s);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeData &oad)
{
//	assert(s.rdstate() == ios::goodbit);

	s << "Oad Header Name:" << oad.header.name << endl;

	for(int i=0;i<oad.entries.size();i++)
	 {
		s << oad.entries[i] << endl;
	 }
	return(s);
}

//============================================================================

//QObjectAttributeData::QObjectAttributeData() : entries(10)
QObjectAttributeData::QObjectAttributeData()
{
}

//============================================================================

#define OAD_CHUNKID 'OAD '

boolean
QObjectAttributeData::Load(istream& input, ostream& error)
{
	assert(input.rdstate() == ios::goodbit);
	assert(error.rdstate() == ios::goodbit);

	// first, load the header and validate it
	input.read((char*)&header,sizeof(_oadHeader));
	if(header.chunkId != OAD_CHUNKID)
	 {
		error << "Incorrect header, is not an OAD file" << endl;
	 	return(boolean::BOOLFALSE);
	 }
	DBSTREAM3( cstats << "Loading Object Header name <" << header.name << ">" << endl; )

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.rdstate() == ios::goodbit)
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
			DBSTREAM3( cdebug << "  added" << endl; )
		 }
		else
		 {
			if(input.rdstate() == ios::goodbit)
			 {
				cerror << "LevelCon Error: QObjectAttributeData::Load: OAD entry load failed" << endl;
				exit(1);
			 }
		 }
	 }
	DBSTREAM3( cdebug << "Loaded Object Header name <" << header.name << "> as :" << *this << endl; )
	return(boolean::BOOLTRUE);
}

//============================================================================
// load from a stream which has no header

boolean
QObjectAttributeData::LoadEntries(istream& input, ostream& error)
{
	assert(input.rdstate() == ios::goodbit);
	assert(error.rdstate() == ios::goodbit);

	header.name[0] = 0;
	header.chunkSize = 0;
	header.chunkId = OAD_CHUNKID;

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.rdstate() == ios::goodbit)
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
		 }
	 }
	return(boolean::BOOLTRUE);
}

//============================================================================

boolean
QObjectAttributeData::ContainsButtonType(buttonType bType)
{
	boolean found = boolean::BOOLFALSE;
	for(int i=0;i<entries.size() && !found;i++)
	 {
//		cdebug << "QObjectAttributeData::ContainsButtonType: Checking entry <" << i << "> which is type <" << entries[i].type << "> against type <"<< LEVELCONFLAG_NOINSTANCES << ">" << endl;
		if(entries[i].GetType() == bType)
			found = boolean::BOOLTRUE;
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
	boolean isInCommonBlock = boolean::BOOLFALSE;

	for(int index=0;index<entries.size();index++)
	{
		const QObjectAttributeDataEntry& tempEntry = entries[index];
		DBSTREAM3( cdebug << "QOAD::SizeOfOnDisk: Processing entry <" << tempEntry.GetName() << "> " << endl; )
		if (isInCommonBlock == boolean::BOOLFALSE)
			size += entries[index].SizeOfOnDisk();
		if (tempEntry.GetType() == LEVELCONFLAG_COMMONBLOCK)
		{
			isInCommonBlock = boolean::BOOLTRUE;
			DBSTREAM3( cdebug << "\tEntering COMMONBLOCK..." << endl; )
		}
		if (tempEntry.GetType() == LEVELCONFLAG_ENDCOMMON)
		{
			isInCommonBlock = boolean::BOOLFALSE;
			DBSTREAM3( cdebug << "\tLeaving COMMONBLOCK..." << endl; )
		}
	}
//	DBSTREAM3( cdebug << "QOAD::SizeOfOnDisk: Done with <" << tempEntry.GetName() << ">; Size = " << size << " bytes." << endl; )
	return(size);
}

//============================================================================
//struct oadinPrj
//{
//	long buttonType;
//	char[?] name;
//	union				// based on type
//	{
//		long def;
//		char[?] oadString;
//	}
//}

// this applies an entry from the .prj file to an OAD in memory
void
QObjectAttributeData::Apply(const char* buffer,int32 length, const char* objName)				// override current oad with fields from prj file
{
	DBSTREAM3( cdebug << "QObjectAttributeData::Apply(char*) called on object named " << objName << endl; )
	DBSTREAM3( HDump((void*)buffer,length,4," ",cdebug); )

	while(length > 0)
	 {
		// first read type
		long buttonType = *((long*)buffer);
		buffer+=4;
		length -= 4;

		// now read name
		assert(strlen(buffer) < 32);
		const char* name = buffer;
		length -= strlen(buffer)+1;
		buffer += strlen(buffer)+1;

		int32 def = 0;
		int32 size = 0;
		const char* oadString = NULL;

		switch(buttonType)
		 {
			case BUTTON_INT16:
			case BUTTON_FIXED16:
				size = 2;
				cerror << "Levelcon Error: object " << objName << " in the .prj file has bad data, re-edit it in 3ds and try again" << endl;
				assert(0);
				break;
			case BUTTON_FIXED32:
			case BUTTON_INT32:
			case LEVELCONFLAG_COMMONBLOCK:
			case BUTTON_FILENAME:
			case BUTTON_MESHNAME:
				def = *((int32*)buffer);
				size = 4;
				break;
			case BUTTON_INT8:
				AssertMsg(0,"Bogus button type found, probably bad data");
				size = 1;
				break;
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
				break;								// not used by us
			case LEVELCONFLAG_SINGLEINSTANCE:
				break;
			case LEVELCONFLAG_TEMPLATE:
				break;
			case LEVELCONFLAG_EXTRACTCAMERA:		// this field burns no space
				assert(0);					// no longer allowed
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
				break;
			case BUTTON_EXTRACT_CAMERA:
				break;
			case LEVELCONFLAG_EXTRACTCAMERANEW:		// this field burns no space
				break;
			case BUTTON_WAVEFORM:
				break;
			case BUTTON_CLASS_REFERENCE:
				size = strlen(buffer)+1;
				oadString = buffer;
				break;
			case BUTTON_GROUP_START:
				break;
			case BUTTON_GROUP_STOP:
				break;
			default:
				cerror << "QObjectAttributeDataEntry::Apply:Unknown oad type <" << buttonType << ">" << endl;
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

//		cdebug << "  Old Entry: " << overRide.entries[overRideIndex] << endl;
		for(int entryIndex=0;entryIndex < entries.size();entryIndex++)
		 {
			if(										// if a field isn't named, it we be thrown away
				entries[entryIndex].GetName().length() &&
				strlen(name) &&
			    !strcmp(entries[entryIndex].GetName().c_str(),name)
			  )
			 {
				cdebug << "  Applying to entry: " << entries[entryIndex] << endl;
//				entries[entryIndex] = overRide.entries[overRideIndex];
				// kts only copy def and oadString
				QObjectAttributeDataEntry& td = entries[entryIndex];
				td.SetDef(def);
				cdebug << "after setdef" << endl;

				if(td.GetMin() > td.GetMax() || td.GetDef() < td.GetMin() || td.GetDef() > td.GetMax())
				 {
					cerror << "Levelcon Error: invalid oad entry in object <" << objName << ">, field <" << td.GetName() << ">" << endl;
					cerror << "Min = " << td.GetMin() << ", max = " << td.GetMax() << ", def = " << td.GetDef() << endl;
					goodLevel = boolean::BOOLFALSE;
				 }
				cdebug << "calling set string" << endl;
#pragma message ("kts put these asserts back in")
//				assert(td.min <= td.max);
//				assert(td.def >= td.min);
//				assert(td.def <= td.max);
//				strncpy(td.string,oadString,td.len);
				if(oadString)
					td.SetString(oadString);

				if( ((td.GetType() == BUTTON_MESHNAME) || (td.GetType() == BUTTON_FILENAME)) && (td.GetDef() == 0) )
	 			{
					cdebug << "copying mode type" << endl;

					const QObjectAttributeDataEntry* modelType = GetEntryByName("Model Type");
					assert(!(modelType && td.GetType() == BUTTON_MESHNAME));
					if (modelType && modelType->GetDef())
					{
						cerror << "Levelcon Error in apply: required asset <" << td.GetName() << "> missing from object ??" << endl;
//						exit(1);
					}
	 			}

				cdebug << "  Resulting in :" << entries[entryIndex] << endl;
				entryIndex = entries.size();
			 }
		 }
		// if not found, it means it is old, useless data, so ignore
	 }
}

//============================================================================

const QObjectAttributeDataEntry*
QObjectAttributeData::GetEntryByName(const char* name) const
{
	QObjectAttributeData* that = (QObjectAttributeData*)this;
	for(int index=0;index<entries.size();index++)
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
	for(int index=0;index<entries.size();index++)
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
	for(int index=0;index<entries.size();index++)
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
	boolean equal = boolean::BOOLTRUE;

	if(entries.size() == left.entries.size())
	 {
		for(int index=0;index<entries.size();index++)
			if(!(entries[index] == left.entries[index]))
				equal = boolean::BOOLFALSE;
	 }
	else
		equal = boolean::BOOLFALSE;

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
QObjectAttributeData::SetFixed32(const string& name,br_scalar value,ostream& error)
{
	QObjectAttributeDataEntry* entry;
	DBSTREAM3( cdebug << "QObjectAttributeData::SetFixed32:QObjectAttributeData::SetFixed32: Updating <" << name << "> to contain <" << BrScalarToFloat(value) << ">" << endl; )
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
		exit(1);
		return(-1);
	 }
	return(entry->GetDef());
}

//============================================================================

size_t
QObjectAttributeDataEntry::SizeOfOnDisk(void) const
{
	size_t size = 0;

	DBSTREAM3( cdebug << "QOADE::SizeOfOnDisk: Entry <" << GetName() << "> has type <" << GetType() << "> " << endl; )

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
//			cerror << "filenames not implemented" << endl;
//			break;
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_FILENAME (" << size << " bytes)" << endl; )
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
		case BUTTON_MESHNAME:
			size = 4;
			DBSTREAM3( cdebug << "\tand is a BUTTON_MESHNAME (" << size << " bytes)" << endl; )
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
			cerror << "QObjectAttributeDataEntry::SizeOfOnDisk:Unknown oad type <" << GetType() << ">" << endl;
			DBSTREAM3( cdebug << "\tand is of UNKNOWN TYPE!" << endl; )
			break;
	}

	DBSTREAM3( cdebug << "QOADE::SizeOfOnDisk: Size was " << size << endl << "\tDone with entry." << endl; )
	return(size);
}

//============================================================================

boolean
QObjectAttributeData::SaveStruct(ostream& output, const QLevel& level,
								 const char* name, ostream& error)
{
	bool isInCommonBlock = false;
	int totalBytesWritten = 0;

	DBSTREAM3( cdebug << "QObjectAttributeData::SaveStruct: saving " << entries.size() << " entries" << endl; )
	for(int entryIndex=0;entryIndex < entries.size(); entryIndex++)
	{
		const QObjectAttributeDataEntry& td = entries[entryIndex];
		DBSTREAM3( cdebug << "QObjectAttributeData::SaveStruct: td type = <" << td.GetType() << ">" << ", Name = <" << td.GetName() << "> " << endl; )
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

					if (isInCommonBlock == boolean::BOOLFALSE)
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
			case BUTTON_MESHNAME:
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
				cerror << "Levelcon Error: LEVELCONFLAG_EXTRACTCAMERA no longer supported" << endl;
				assert(0);
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
//				DBSTREAM3( cdebug << "QOAD::SaveStruct::LEVELCONFLAG_EXTRACTCAMERANEW:" << endl; )
//				// read position of this object
//				kfmesh3ds* thisKFMesh = NULL;
//				GetObjectMotionByName3ds(level.GetLevelFile()->GetDatabase(), (char*)name, &thisKFMesh);
//				PRINT_ERRORS_EXIT(stderr);
//				if(!thisKFMesh)
//				{
//					cerror << "Extract camera this object named <" << name << "> not found" << endl;
//					break;
//				}
//				assert(thisKFMesh);
//				QPoint thisPoint(BrFloatToScalar(thisKFMesh->pos[0].x),
//							 BrFloatToScalar(thisKFMesh->pos[0].y),
//							 BrFloatToScalar(thisKFMesh->pos[0].z)
//							);

				// read position of target object
//				const QObjectAttributeDataEntry* targetEntry = _GetEntryByName("Target");
//				if(!targetEntry)
//				{
//					cerror << "Extract Camera Error: cannot find .oad field named Target in object <" << name << ">"  << endl;
//					exit(1);
//					break;
//				}
//				assert(targetEntry);
//				kfmesh3ds* targetKFMesh = NULL;
//				GetObjectMotionByName3ds(level.GetLevelFile()->GetDatabase(), (char*)targetEntry->GetString().c_str(), &targetKFMesh);
//				PRINT_ERRORS_EXIT(stderr);
//				if(!targetKFMesh)
//				{
//					cerror << "Extract camera target object named <" << targetEntry->GetString() << "> not found in object <" << name << ">"  << endl;
//					break;
//				}
//				assert(targetKFMesh);
//				QPoint targetPoint(BrFloatToScalar(targetKFMesh->pos[0].x),
//							 BrFloatToScalar(targetKFMesh->pos[0].y),
//							 BrFloatToScalar(targetKFMesh->pos[0].z)
//							);

				// read position of follow object
//				const QObjectAttributeDataEntry* followEntry = _GetEntryByName("Follow");
//				if(!followEntry)
//				{
//					cerror << "Extract Camera Error: cannot find .oad field named Follow in object <" << name << ">"  << endl;
//					exit(1);
//					break;
//				}
//				assert(followEntry);
//				kfmesh3ds* followKFMesh = NULL;
//				GetObjectMotionByName3ds(level.GetLevelFile()->GetDatabase(), (char*)followEntry->GetString().c_str(), &followKFMesh);
//				PRINT_ERRORS_EXIT(stderr);
//				if(!followKFMesh)
//				{
//					cerror << "Extract camera follow object named <" << followEntry->GetString() << "> not found in object <" << name << ">"  << endl;
//					break;
//				}
//				assert(followKFMesh);
//				QPoint followPoint(BrFloatToScalar(followKFMesh->pos[0].x),
//							 BrFloatToScalar(followKFMesh->pos[0].y),
//							 BrFloatToScalar(followKFMesh->pos[0].z)
//							);

//				DBSTREAM3( cdebug << "  this position is :" << thisPoint << endl; )
//				DBSTREAM3( cdebug << "  target position is :" << targetPoint << endl; )
//				DBSTREAM3( cdebug << "  follow position is :" << followPoint << endl; )

				// now do position, which is based on the position booleans

//				if(GetOADValue("Position X",error))
//				{
//					SetFixed32("CamPositionX",thisPoint.x()-followPoint.x(),error);
//					DBSTREAM3( cdebug << "Writing out Relative camera X position <" << )
//						DBSTREAM3( thisPoint.x()-followPoint.x() )
//						DBSTREAM3( << ">" << endl; )
//				}
//				else
//				{
//					DBSTREAM3( cdebug << "Writing out Absolute camera X position" << endl; )
//					SetFixed32("CamPositionX",thisPoint.x(),error);
//				}

//				if(GetOADValue("Position Y",error))
//				{
//					SetFixed32("CamPositionY",thisPoint.y()-followPoint.y(),error);
//					DBSTREAM3( cdebug << "Writing out Relative camera Y position <" )
//						DBSTREAM3( << thisPoint.y()-followPoint.y() << ">" )
//						DBSTREAM3( << endl; )
//				}
//				else
//				{
//					DBSTREAM3( cdebug << "Writing out Absolute camera Y position" << endl; )
//					SetFixed32("CamPositionY",thisPoint.y(),error);
//				}

//				if(GetOADValue("Position Z",error))
//				{
//					SetFixed32("CamPositionZ", thisPoint.z()-followPoint.z(),error);
//					DBSTREAM3( cdebug << "Writing out Relative camera Z position <" << )
//							DBSTREAM3( thisPoint.z()-followPoint.z() )
//							DBSTREAM3( << ">" << endl; )
//				}
//				else
//				{
//					DBSTREAM3( cdebug << "Writing out Absolute camera Z position" << endl; )
//					SetFixed32("CamPositionZ",thisPoint.z(),error);
//				}

				// now do lookat fields, which are based on focus flag
//				if(GetOADValue("Focus",error))
//				{			// relative
//					DBSTREAM3( cdebug << "Writing out Relative camera focus" << endl; )
//					SetFixed32("LookAtX",targetPoint.x()-followPoint.x(),error);
//					SetFixed32("LookAtY",targetPoint.y()-followPoint.y(),error);
//					SetFixed32("LookAtZ",targetPoint.z()-followPoint.z(),error);
//				}
//				else
//				{			// absolute
//					DBSTREAM3( cdebug << "Writing out Absolute camera focus" << endl; )
//					SetFixed32("LookAtX",targetPoint.x()-thisPoint.x(),error);
//					SetFixed32("LookAtY",targetPoint.y()-thisPoint.y(),error);
//					SetFixed32("LookAtZ",targetPoint.z()-thisPoint.z(),error);
//				}
				break;
			}
			case LEVELCONFLAG_COMMONBLOCK:
			{
				isInCommonBlock = boolean::BOOLTRUE;
				int32 tempDef = td.GetDef();
				WriteBytes(output,(char*)&tempDef,4);
				totalBytesWritten += 4;
				DBSTREAM3( cdebug << "QOAD::SaveStruct: wrote out " << totalBytesWritten << " so far..." << endl; )
				break;
			}
			case LEVELCONFLAG_ENDCOMMON:
				isInCommonBlock = boolean::BOOLFALSE;
				break;
			case BUTTON_XDATA:
				DBSTREAM3( cdebug <<"QOAD::SaveStruct: XDATA: xdata = " << td.GetXDataConversionAction() << endl; )
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
					if(typeIndex == -1)
					{
						cerror << "LevelCon Error: Object reference type of <" << td.GetString() << "> in object <" << name << "> not found in .LC file" << endl;
						exit(1);
					}
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
				error << "QObjectAttributeData::SaveStruct:Unknown oad type <" << td.GetType() << ">" << endl;
				break;
		}
	}
	return(boolean::BOOLTRUE);
}

//============================================================================