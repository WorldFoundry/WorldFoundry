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
//==============================================================================

#include <iostream>
using namespace std;
#include <pigsys/assert.hp>
#include <cpplib/stdstrm.hp>

#include "oad.hp"

//============================================================================

bool
QObjectAttributeDataEntry::Load(istream& input, ostream& error)
{
	assert(input.rdstate() == ios::goodbit);
	assert(error.rdstate() == ios::goodbit);

	streampos oldpos = input.tellg();
	cstats << "Reading .oad entry from file offset <" << input.tellg() << ">" << endl;
	cdebug << "sizeof(_typeDescriptor) = <" << sizeof(_typeDescriptor) << ">, offset+sizeof = <" <<
	sizeof(_typeDescriptor)+input.tellg() << ">" << endl;

	input.read((char*)&entry, sizeof(_typeDescriptor));

	if(input.rdstate() != ios::goodbit)
	 {
		cdebug << "failed to read from stream" << endl;
		return(false);
	 }

#pragma message( __FILE__ ": this assertion doesn't compile" )
//        assert((oldpos+sizeof(_typeDescriptor)) == input.tellg());

	cstats << "Loaded entry " << (*this) << endl;
	return(true);
}

//============================================================================

char* szConversionAction[ XDATA_CONVERSION_MAX] =
{
	"XDATA_IGNORE",
	"XDATA_COPY",
	"XDATA_OBJECTLIST",
	"XDATA_CONTEXTUALANIMATIONLIST",
	"XDATA_SCRIPT"
};

ostream& operator<<(ostream& s, QObjectAttributeDataEntry &oad)
{
	//s << "QObjectAttributeDataEntry: " << endl;

	s << "   Type: ";
	switch(oad.entry.type)
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
		case 	BUTTON_PROPERTY_SHEET:
            s << "BUTTON_PROPERTY_SHEET";
			break;
		case 	LEVELCONFLAG_NOMESH:
            s << "LEVELCONFLAG_NOMESH";
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
		case	LEVELCONFLAG_COMMONBLOCK:		/* flags beginning of a common block */
			s << "LEVELCONFLAG_COMMONBLOCK";
			break;
		case	LEVELCONFLAG_ENDCOMMON:			/* flags end of a common block */
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
		case 	BUTTON_CLASS_REFERENCE:
			s << "BUTTON_CLASS_REFERENCE";
			break;
		case	BUTTON_GROUP_START:
			s << "BUTTON_GROUP_START";
			break;
		case	BUTTON_GROUP_STOP:
			s << "BUTTON_GROUP_STOP";
			break;

		default:
			s << "Unknown";
			break;
	}
	s << " <" << (int)oad.entry.type << ">" << endl;

	s << "   Name: " << oad.entry.name << " (length = " << strlen(oad.entry.name) << ")" << endl;
	s << "Display: " << oad.entry.xdata.displayName << endl;
	s << "    Min: " << oad.entry.min << '\t' << double( oad.entry.min ) / 65536.0 << endl;
	if ( oad.entry.type == BUTTON_GROUP_START )
		s << "  Width: " << oad.entry.max << endl;
	else
                s << "    Max: " << oad.entry.max << '\t' << oad.entry.max/65536.0 << endl;
	s << "Default: " << oad.entry.def << endl;
//	if(oad.entry.len)
		s << "  String:" << oad.entry.string << endl;
	s << " ShowAs: " << int(oad.entry.showAs) << endl;

	if ( oad.entry.type == BUTTON_XDATA )
	{
		assert(XDATA_CONVERSION_MAX == 5);			// kts added new data type name to szConversionAction above
		assert( XDATA_IGNORE <= oad.entry.xdata.conversionAction &&
			oad.entry.xdata.conversionAction <= XDATA_CONVERSION_MAX );
		s << "conversionAction: " << szConversionAction[ oad.entry.xdata.conversionAction ] << endl;
		s << "bRequired: " << oad.entry.xdata.bRequired << endl;
	}

		//<< "   Name: "

	if(strlen(oad.entry.helpMessage))
		s << "   Help: " << oad.entry.helpMessage << endl;

	if(oad.entry.x != -1)
		s << "      X: " << oad.entry.x << endl;
	if(oad.entry.y != -1)
		s << "      Y: " << oad.entry.y << endl;

	return(s);
}

//============================================================================

ostream& operator<<(ostream& s, QObjectAttributeData &oad)
{
	assert(s.rdstate() == ios::goodbit);

	s << oad.header.name << endl;
	s << endl;

	for(int i=0;i<oad.entries.size();i++)
	 {
		s << oad.entries[i] << endl;
	 }
	return(s);
}

//============================================================================
#define OAD_CHUNKID 'OAD '

//============================================================================
// load from a stream which has no header

bool
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
	return(true);
}

//============================================================================

bool
QObjectAttributeData::Load(istream& input, ostream& error)
{
	assert(input.rdstate() == ios::goodbit);
	assert(error.rdstate() == ios::goodbit);

	// first, load the header and validate it
	input.read((char*)&header,sizeof(_oadHeader));
	if(header.chunkId != OAD_CHUNKID)
	 {
		error << "Incorrect header, is not an OAD file" << endl;
	 	return(false);
	 }
	cstats << "Loading Object Header name <" << header.name << ">" << endl;

	// now loop through the entries
	QObjectAttributeDataEntry entry;
	while(input.rdstate() == ios::goodbit)
	 {
		if(entry.Load(input,error))
		 {
			entries.push_back(entry);
		 }
	 }
	return(true);
}

//============================================================================
