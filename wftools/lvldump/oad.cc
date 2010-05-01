//============================================================================
// oad.cc:
// By Kevin T. Seghetti
// Copyright (c) 1995-1999, World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//============================================================================

//#include <pclib/boolean.hp>
#include <iostream>
#include <strstream>
using namespace std;
#include <pigsys/assert.hp>
#include <math.h>
#include <string>

#include "hdump.hp"

#include "oad.hp"
#include "level.hp"
#include "asset.hp"

//============================================================================

bool
QObjectAttributeDataEntry::Load(istream& input, ostream& error)
{
#if 0
	assert(input.good() );
	assert(error.good());

	input.read((char*)&entry, sizeof(_typeDescriptor));

	if(!input.good())
		return(false);

	cdebug << "Loading entry " << (*this) << endl;
	return(true);
#endif
	assert(input.good());
	assert(error.good());

	streampos oldpos = input.tellg();

	input.read((char*)&entry, sizeof(_typeDescriptor));

	if(!input.good())
		return(false);

	assert((oldpos+streampos(sizeof(_typeDescriptor))) == input.tellg());

	cstats << "Loaded entry " << (*this) << endl;
	return(true);

}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeDataEntry &oadEntry)
{
//	assert(s.rdstate() == ios::goodbit);

	s << "      ";

	switch(oadEntry.entry.type)
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
			break;
		case 	BUTTON_CAMERA_REFERENCE:
			s << "BUTTON_CAMERA_REFERENCE";
			break;
		case 	BUTTON_LIGHT_REFERENCE:
			s << "BUTTON_LIGHT_REFERENCE";
			break;
		case LEVELCONFLAG_ROOM:
			s << "LEVELCONFLAG_ROOM";
			break;
		case	LEVELCONFLAG_COMMONBLOCK:
			s << "LEVELCONFLAG_COMMONBLOCK";
			break;
		case	LEVELCONFLAG_ENDCOMMON:
			s << "LEVELCONFLAG_ENDCOMMON";
			break;
		case	BUTTON_MESHNAME:
			s << "BUTTON_MESHNAME";
			break;
		case	BUTTON_XDATA:
			s << "BUTTON_XDATA";
			break;
		case	BUTTON_EXTRACT_CAMERA:
			s << "BUTTON_EXTRACT_CAMERA";
			break;
		case	LEVELCONFLAG_EXTRACTCAMERANEW:
			s << "LEVELCONFLAG_EXTRACTCAMERANEW";
			break;
		case	BUTTON_WAVEFORM:
			s << "BUTTON_WAVEFORM";
			break;
		case	BUTTON_CLASS_REFERENCE:
			s << "BUTTON_CLASS_REFERENCE";
			break;
		case	BUTTON_GROUP_START:
			s << "BUTTON_GROUP_START";
			break;
		case	BUTTON_GROUP_STOP:
			s << "BUTTON_GROUP_STOP";
			break;
		case	LEVELCONFLAG_EXTRACTLIGHT:
			s << "LEVELCONFLAG_EXTRACTLIGHT";
			break;
		default:
			s << "Unknown";
			cwarn << "Warning: object of unknown type <" << int(oadEntry.entry.type) << "> found" << endl;
			break;
	 }
	s << " <" << int(oadEntry.entry.type) << ">";
	s << "  Name: <" << oadEntry.entry.name << ">";

	if (oadEntry.entry.type == BUTTON_MESHNAME)
		s << hex;

//	if(oadEntry.entry.type == BUTTON_FIXED32)
//		s << "  Value: <" << BrScalarToFloat(oadEntry.entry.def) << "> (" << oadEntry.entry.def << ")";
//	else
//		s << "  Value: <" << oadEntry.entry.def << ">";

	s << dec;

	if(oadEntry.entry.len)
	{
		s << endl << "        String: <" << oadEntry.entry.string << ">";
		s << "  Len: <" << oadEntry.entry.len << ">";
	}

//	s << "  Showas:" << oad.entry.showAs << endl;
	return(s);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeData &oad)
{
	assert(s.good());

	s << "    Oad Header Name:" << oad.header.name << endl;

	_typeDescriptor tempEntry;
	int isInCommonBlock = 0;

	for(int index=0;index<oad.entries.size();index++)
	 {
	     QObjectAttributeDataEntry foo = oad.entries[index];
		tempEntry = oad.entries[index].Get();
		s << oad.entries[index] << endl;
//		if (tempEntry.type == LEVELCONFLAG_COMMONBLOCK)
//		 {
//			isInCommonBlock++;
//		 }
//		if (tempEntry.type == LEVELCONFLAG_ENDCOMMON)
//			if(isInCommonBlock)
//				isInCommonBlock--;
//			else
//				assert(0);			// unbalanced COMMONBLOCK nesting
	 }
	return(s);
}

//============================================================================
#define OAD_CHUNKID 'OAD '

bool
QObjectAttributeData::Load(istream& input, ostream& error)
{
	assert(input.good());
	assert(error.good());

	// first, load the header and validate it
	input.read((char*)&header,sizeof(_oadHeader));
	if(header.chunkId != OAD_CHUNKID)
	 {
		error << "Incorrect header, is not an OAD file" << endl;
	 	return(false);
	 }
	cdebug << "Loading Object Header name <" << header.name << ">" << endl;

	// Make sure entries is empty!
	while (entries.size() > 0)
		entries.pop_back();

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.good())
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
		 }
	 }
	cdebug << "  Object named <" << header.name << "> loaded" << endl << endl;

	return(true);
}

//============================================================================
// load from a stream which has no header

bool
QObjectAttributeData::LoadEntries(istream& input, ostream& error)
{
	assert(input.good());
	assert(error.good());

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
	for(int i=0;i<entries.size() && !found;i++)
	 {
		cdebug << "QObjectAttributeData::ContainsButtonType: Checking entry <" << i << "> which is type <" << entries[i].Get().type << "> against type <"<< LEVELCONFLAG_NOINSTANCES << ">" << endl;
		if(entries[i].Get().type == bType)
			found = true;
	 }
	return(found);
}

//============================================================================
// construct an int containing a bit field indicating which types are present in this OAD

int32
QObjectAttributeData::GetOADFlags(void)
{
	int32 flags = 0;
	for(int index=0;index<entries.size();index++)
	 {
		assert(entries[index].Get().type < 32);			// since we are using an int16
		flags |= (1<<entries[index].Get().type);
	 }
	return(flags);
}

//============================================================================

size_t
QObjectAttributeData::SizeOfOnDisk(void)
{
	size_t size = 0;
	_typeDescriptor tempEntry;
	bool isInCommonBlock = false;

	cdebug << "OAD::SizeOfOnDisk:" << endl;
	for(int index=0;index<entries.size();index++)
	 {
		cdebug << "OAD:SOOD: index " << index << ", isInCommonBlock = " << isInCommonBlock << ", size = " << size << endl;
		tempEntry = entries[index].Get();
		if (isInCommonBlock == false)
			size += entries[index].SizeOfOnDisk();
		if (tempEntry.type == LEVELCONFLAG_COMMONBLOCK)
			isInCommonBlock = true;
		if (tempEntry.type == LEVELCONFLAG_ENDCOMMON)
			isInCommonBlock = false;
	 }
	return(size);
}

//============================================================================

void
QObjectAttributeData::Apply(const QObjectAttributeData& overRide)				// override current oad with fields from new oad
{
	QObjectAttributeDataEntry newEntry;
	// loop through new oad, lookup up new fields in old oad, and replace or add as necessary
	for(int overRideIndex=0;overRideIndex<overRide.entries.size();overRideIndex++)
	 {
		newEntry = overRide.entries[overRideIndex];
		for(int entryIndex=0;entryIndex < entries.size();entryIndex++)
		 {
			if(!strcmp(entries[entryIndex].Get().name,overRide.entries[overRideIndex].Get().name))
			 {
				entries[entryIndex] = overRide.entries[overRideIndex];
				entryIndex = entries.size();
			 }
		 }
		// if not found, it means it is old, useless data, so ignore
	 }
}

//============================================================================

const QObjectAttributeDataEntry*
QObjectAttributeData::GetEntryByName(const char* name)
{
	for(int index=0;index<entries.size();index++)
	 {
		if(!strcmp(entries[index].Get().name,name))
		 {
			return(&entries[index]);
		 }
	 }
	return(NULL);
}

//============================================================================

void
ReadBytes(istream& input, char* data, int count)
{
	cdebug << "ReadBytes: reading <" << count << "> bytes";
	assert(input.good());
	long pos = long( input.tellg() );
	cdebug << "RB: Pos = " << pos << endl;
	for(int index=0;index<count;index++)
		input.get(data[index]);
	if( long(input.tellg()) != (pos+count))
	  cdebug <<"Error!: Read <" << long(input.tellg())-pos << "> bytes, when should have read <" << count << "> bytes" << endl;
	//	assert(int(input.tellg()) == (pos+count));
	assert(input.good());
}

//============================================================================

void
WriteBytes(ostream& output, char* data, int count)
{
	for(int index=0;index<count;index++)
		output << data[index];
}

//============================================================================

void
QObjectAttributeData::SetFixed32(const string name,int32 value,ostream& error)
{
	const QObjectAttributeDataEntry* entry;
	entry = GetEntryByName(name.c_str());
	if(!entry)
	 {
		error << "Cannot find .oad field named <" << name << endl;
		return;
	 }
}

//============================================================================

size_t
QObjectAttributeDataEntry::SizeOfOnDisk(void)
{
	size_t size = 0;

	switch(entry.type)
	 {
		case BUTTON_INT16:
		case BUTTON_FIXED16:
			size = 2;
			assert(0);
			break;
		case BUTTON_FIXED32:
		case BUTTON_INT32:
		case LEVELCONFLAG_COMMONBLOCK:
			size = 4;
			break;
		case BUTTON_INT8:
			assert(0);
			size = 1;
			break;
		case BUTTON_STRING:
			size = entry.len;
			assert(!(entry.len  & 1));			// len must be even
			break;
		case BUTTON_OBJECT_REFERENCE:
			size = 4;
			break;
		case BUTTON_FILENAME:
//			size = entry.len;
//			assert(!(entry.len  & 1));			// len must be even
//			cerror << "filenames not implemented" << endl;
			size = 4;					// maps to asset
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
			break;
		case BUTTON_CAMERA_REFERENCE:
//			size = 4;
			break;
		case BUTTON_LIGHT_REFERENCE:
//			size = 4;
			break;
		case BUTTON_CLASS_REFERENCE:
			size = 4;
			break;
		case BUTTON_MESHNAME:
			size = 4;
			break;
		case LEVELCONFLAG_ROOM:
		case LEVELCONFLAG_ENDCOMMON:
		case BUTTON_EXTRACT_CAMERA:
		case LEVELCONFLAG_EXTRACTCAMERANEW:
		case BUTTON_WAVEFORM:
		case BUTTON_GROUP_START:
		case BUTTON_GROUP_STOP:
		case LEVELCONFLAG_EXTRACTLIGHT:
			break;
		case BUTTON_XDATA:
			assert(XDATA_CONVERSION_MAX == 5);
			if(
				entry.xdata.conversionAction == XDATA_COPY ||
				entry.xdata.conversionAction == XDATA_OBJECTLIST ||
				entry.xdata.conversionAction == XDATA_CONTEXTUALANIMATIONLIST ||
				entry.xdata.conversionAction == XDATA_SCRIPT
			  )
			 {
				cdebug << "OADE:SOOD: button xdata true!" << endl;
				size = 4;
			 }
			break;
		default:
			cerror << "QObjectAttributeDataEntry::SizeOfOnDisk:Unknown oad type <" << int(entry.type) << ">" << endl;
			break;
	 }
	return(size);
}

//============================================================================

bool
QObjectAttributeDataEntry::LoadStruct(istream& input, ostream& error)		// load from the binary oad produced for the game
{
	int stringIndex;
	cdebug << "Reading OAD Entry <" << entry.name << "> from offset <" << input.tellg() << ">" << endl;
	switch(entry.type)
	 {
		case BUTTON_INT16:
		case BUTTON_FIXED16:
			ReadBytes(input,(char*)&entry.def,2);
			break;
			case BUTTON_FIXED32:
			case BUTTON_INT32:
				ReadBytes(input,(char*)&entry.def,4);
				break;
			case BUTTON_INT8:
				input >> ((char*)(&entry.def));
				break;
			case BUTTON_STRING:
				for(stringIndex=0;stringIndex<entry.len;stringIndex++)
				 {
					input >> entry.string[stringIndex];
				 }
				break;
			case BUTTON_OBJECT_REFERENCE:
			 {
				ReadBytes(input,(char*)&entry.def,4);
				break;
			 }
			case BUTTON_FILENAME:
//				assert(entry.len < 512);
//				for(stringIndex=0;stringIndex<entry.len;stringIndex++)
//				 {
//					input >> entry.string[stringIndex];
//				 }
				ReadBytes(input,(char*)&entry.def,4);
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
				break;
			case BUTTON_CAMERA_REFERENCE:
				break;
			case BUTTON_LIGHT_REFERENCE:
				break;

			case LEVELCONFLAG_COMMONBLOCK:
				ReadBytes(input,(char*)&entry.def,4);
				break;
			case LEVELCONFLAG_ENDCOMMON:
				break;
			case BUTTON_MESHNAME:
				ReadBytes(input,(char*)&entry.def,4);
				break;
			case BUTTON_XDATA:
				assert(XDATA_CONVERSION_MAX == 5);
				if(
					entry.xdata.conversionAction == XDATA_COPY ||
					entry.xdata.conversionAction == XDATA_OBJECTLIST ||
					entry.xdata.conversionAction == XDATA_CONTEXTUALANIMATIONLIST ||
					entry.xdata.conversionAction == XDATA_SCRIPT
				  )
				 {
					ReadBytes(input,(char*)&entry.def,4);
					cdebug << "OADE:LS: button xdata true!, contains " << entry.def << endl;
				 }
				break;
			case LEVELCONFLAG_ROOM:
				break;
			case BUTTON_EXTRACT_CAMERA:
				break;
			case LEVELCONFLAG_EXTRACTCAMERANEW:
				break;
			case BUTTON_WAVEFORM:
				break;
			case BUTTON_CLASS_REFERENCE:
				ReadBytes(input,(char*)&entry.def,4);
				break;
			case BUTTON_GROUP_START:
				break;
			case BUTTON_GROUP_STOP:
				break;
			case LEVELCONFLAG_EXTRACTLIGHT:
				break;
			default:
				error << "QObjectAttributeData::LoadStruct:Unknown oad type <" << int(entry.type) << ">" << endl;
				return(false);
				break;
	 }
	return(true);
}

//============================================================================

const int32 MAX_COMMON_NEST = 32;

bool
QObjectAttributeData::LoadStruct(istream& input, ostream& error,char* commonBlockAddr,int32 commonBlockSize)		// load from the binary oad produced for the game
{
	assert(commonBlockAddr);
	cdebug << "QOAD::LS: start: commonBlockAddr = " << (void*)commonBlockAddr << ", commonBlockSize = " << commonBlockSize << endl;
//	strstream* commonRefArray[MAX_COMMON_NEST];
	strstream* commonBlockStream;
	int stackIndex = 0;							// common block nesting counter

	cdebug << "QObjectAttributeData::LoadStruct: loading " << entries.size() << " entries" << endl;

	for(int entryIndex=0;entryIndex < entries.size(); entryIndex++)
	 {
		cdebug << "QObjectAttributeData::LoadStruct: loading object  entry " << entryIndex << endl;
		if(stackIndex)				// are in a common block
		 {
			cdebug << "QOAD::LS: in common block" << endl;
			assert(stackIndex);
			if (entries[entryIndex].Get().type == LEVELCONFLAG_ENDCOMMON)
			 {
				cdebug << "QOAD::LS: ending common block" << endl;
				stackIndex--;
				assert(stackIndex >= 0);
				delete commonBlockStream;
				commonBlockStream = NULL;
			 }
			else
			 {
				assert(commonBlockStream);
				if(!entries[entryIndex].LoadStruct(*commonBlockStream,error))
					return(false);
				if(entries[entryIndex].Get().type == LEVELCONFLAG_COMMONBLOCK)
			 	 {
					assert(0);
			 	 }
			 }
		 }
		else				// are not in a common block
		 {
			cdebug << "QOAD::LS: not in common block" << endl;
			if(!entries[entryIndex].LoadStruct(input,error))
				return(false);
			if(entries[entryIndex].Get().type == LEVELCONFLAG_COMMONBLOCK)
			 {
				int32 offset = entries[entryIndex].Get().def;
				cdebug << "QOAD::LS: entering common block with offset of " << offset << endl;
				assert(offset < commonBlockSize);
				commonBlockStream = new strstream(commonBlockAddr+offset,commonBlockSize-offset,ios::in|ios::binary );
				assert(commonBlockStream);
				stackIndex++;
			 }
		 }

	 }
	cdebug << "QObjectAttributeData::LoadStruct: Done " << endl;
	return(true);
}

//============================================================================
