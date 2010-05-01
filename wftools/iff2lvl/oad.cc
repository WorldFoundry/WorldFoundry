//============================================================================
// oad.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================

#include "global.hp"
//#include <iostream.h>
#include <math.h>
#include <string.h>
#include "hdump.hp"

#include "oad.hp"
#include "level.hp"
#include "levelcon.hp"
#include <iff/iffread.hp>
#include <eval/eval.h>
#include <streams/binstrm.hp>

//============================================================================

//extern Interface* gMaxInterface;

//==============================================================================
// iff read helper function to read an entire chunk into a string
// move these into iffread at some point
string
IFFReadString(IFFChunkIter& iter)
{
    char buffer[1024];
    int len = iter.BytesLeft();
    RangeCheck(0,len,1024);
	iter.ReadBytes(buffer, len);
    buffer[len] = 0;
    string retVal(buffer);
    return retVal;
}
            
int32
IFFReadInt32(IFFChunkIter& iter)
{
    int32 retVal;
    iter.ReadBytes(&retVal,sizeof(int32));
    return retVal;
}

bool
IFFReadBool(IFFChunkIter& iter)
{
    int8 retVal;
    iter.ReadBytes(&retVal,sizeof(int8));
    return retVal;
}

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
	SetShowAs(temp.showAs);

#pragma message ("KTS " __FILE__ ": this fails sometimes, figure out why")
//    if( GetType() == BUTTON_FILENAME )
//		assert(GetDef() == 0);

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

//==============================================================================

void
QObjectAttributeDataEntry::LoadIFFEntry(IFFChunkIter& oadIter, ostream& error)
{
    DBSTREAM2(cdebug << "OADE::LIFFE:begin, ID = " << oadIter.GetChunkID() << endl; )

    switch(oadIter.GetChunkID().ID())
    {
        case IFFTAG('I','3','2',0):
        case IFFTAG('F','3','2',0):
        case IFFTAG('S','T','R',0):
        {
                DBSTREAM2(cdebug << "OADE::LIFF: I32 tag" << endl; )

                switch(oadIter.GetChunkID().ID())
                {
                    case IFFTAG('I','3','2',0):
                        SetType(BUTTON_INT32);
                        SetString("");
                        break;
                    case IFFTAG('F','3','2',0):
                        SetType(BUTTON_FIXED32);
                        SetString("");
                        break;
                    case IFFTAG('S','T','R',0):
                        SetType(BUTTON_STRING);
                        SetMin(0);
                        SetMax(0);
                        SetDef(0);
                        break;
                    default:
                        assert(0);
                }


                while(oadIter.BytesLeft() > 0)
                {
                    IFFChunkIter* chunkIter = oadIter.GetChunkIter(*levelconRM);
            		DBSTREAM2(cdebug << "OADE::LIFFE: processing tag " <<  chunkIter->GetChunkID()  << endl; )
                    switch(chunkIter->GetChunkID().ID())
                    {

                        case IFFTAG('N','A','M','E'):
                            {
                                string name = IFFReadString(*chunkIter);
                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: Name = " << name << endl; )
                                SetName(name.c_str());
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }
                        case IFFTAG('D','S','N','M'):
                            {
                                string dsnm = IFFReadString(*chunkIter);
                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: dsnm = " << dsnm << endl; )
                                assert(chunkIter->BytesLeft() == 0);
                                // just drop it, we don't need display name
                                break;
                            }
                        case IFFTAG('R','A','N','G'):
                            {
                                int32 min = IFFReadInt32(*chunkIter);
                                int32 max = IFFReadInt32(*chunkIter);
                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: range = " << min << ", " << max << endl; )
                                SetMin(min);
                                SetMax(max);
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }
                        case IFFTAG('D','A','T','A'):
                            {
                                if(oadIter.GetChunkID().ID() == IFFTAG('S','T','R',0))
                                {
                                    string str = IFFReadString(*chunkIter);
                                    DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: string def = " << str << endl; )
                                    SetDef(0);
                                    SetString(str);
                                }
                                else
                                {
                                    int32 def = IFFReadInt32(*chunkIter);
                                    DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: int def = " << def << endl; )
                                    SetDef(def);

                                }
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }
                        case IFFTAG('D','I','S','P'):
                            {
                                int32 showas = IFFReadInt32(*chunkIter);
                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: showas = " << showas << endl; )
                                SetShowAs(showas);
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }

                        case IFFTAG('H','E','L','P'):
                            {
                                string help = IFFReadString(*chunkIter);
                                //SetHelp(help.c_str());
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }
                        case IFFTAG('E','N','V','L'):
                            {
                                string envl = IFFReadString(*chunkIter);
                                SetString(envl.c_str());
                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: envl = " << envl << endl; )
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }

                        case IFFTAG('E','N','B','L'):
                            {
                                IFFReadString(*chunkIter);
                                // just discard, we don't need it
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }

                        case IFFTAG('H','I','N','T'):
                            {
                                string hint = IFFReadString(*chunkIter);
                                if(hint == string("Extract Camera"))
                                {
                                    SetType(LEVELCONFLAG_EXTRACTCAMERANEW);
                                }
                                else if(hint == string("Conversion: Convert"))
                                {
                                    SetType(BUTTON_XDATA);
                                    SetXDataConversionAction(XDATA_SCRIPT);
                                }
                                else if(hint == string("Conversion: Ignore"))
                                {
                                    SetType(BUTTON_XDATA);
                                    SetXDataConversionAction(XDATA_IGNORE);
                                }
                                else if(hint == string("Conversion: ObjectList"))
                                {
                                    SetType(BUTTON_XDATA);
                                    SetString("");
                                    SetXDataConversionAction(XDATA_OBJECTLIST);
                                }
                                else if(hint == string("Conversion: Copy"))
                                {
                                    SetType(BUTTON_XDATA);
                                    SetString("");
                                    SetXDataConversionAction(XDATA_COPY);
                                }

                                else if(hint == string("Object Reference"))
                                {
                                    SetType(BUTTON_OBJECT_REFERENCE);
									SetShowAs(SHOW_AS_N_A);
                                    SetXDataConversionAction(XDATA_IGNORE);
                                }
                                else if(hint == string("Filename"))
                                {
                                    SetType(BUTTON_FILENAME);
                                    SetString("");
                                    SetXDataConversionAction(XDATA_IGNORE);
                                }

                                else if(hint == string("Class Reference"))
                                {
                                    SetType(BUTTON_CLASS_REFERENCE);
                                    SetString("");
                                    SetXDataConversionAction(XDATA_IGNORE);
                                }
                                else if(hint == string("Color"))
                                {
                                    SetShowAs(SHOW_AS_COLOR);
                                }
#define FILESPEC "FILESPEC:"
                                else if(hint.substr(0,strlen(FILESPEC)) == string(FILESPEC))
                                {
                                    // ignore FILESPEC hints
                                }
                                else
                                    AssertMsg(0,"ignoring hint of <" << hint << ">" );
                                assert(chunkIter->BytesLeft() == 0);
                                break;
                            }

                        case IFFTAG('B','O','O','L'):
                            {

                                while(chunkIter->BytesLeft() > 0)
                                {
                                    IFFChunkIter* boolIter = chunkIter->GetChunkIter(*levelconRM);
                                    switch(boolIter->GetChunkID().ID())
                                    {
                                        case IFFTAG('N','A','M','E'):
                                            {
                                                string name = IFFReadString(*boolIter);
                                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: bool: Name = " << name << endl; )
                                                assert(name == string("Required"));
                                                break;
                                            }
                                        case IFFTAG('D','A','T','A'):
                                            {
                                                assert(oadIter.GetChunkID().ID() == IFFTAG('S','T','R',0));
                                                int32 required = IFFReadInt32(*boolIter);
                                                DBSTREAM2(cdebug << "OADE::LIFFE: I32 tag: bool: required = " << required << endl; )
                                                SetXDataRequired(required);
                                                assert(boolIter->BytesLeft() == 0);
                                                break;
                                            }
                                        default:
                                            cout << "chunk " << chunkIter->GetChunkID() << " not supported" << endl;
                                            assert(0);
                                            break;
                                    }
                                    MEMORY_DELETE(*levelconRM,boolIter,IFFChunkIter);
                                }
                            }
                            break;
                        default:
                            cout << "chunk " << chunkIter->GetChunkID() << " not supported" << endl;
                            assert(0);
                            break;
                    }
                    MEMORY_DELETE(*levelconRM,chunkIter,IFFChunkIter);
                }
            assert(oadIter.BytesLeft() == 0);
            break;
        }

        case IFFTAG('S','T','R','U'):
        {
            assert(0);          // this should have been caught in QObjectAttributeDataEntry::LoadIFF
            break;
        }

        case IFFTAG('H','I','N','T'):
            {
                string hint = IFFReadString(oadIter);

                if(hint == string("Extract Room"))
                {
                    SetType(LEVELCONFLAG_ROOM );

                }
                else if(hint == string("Extract Light"))
                {
                    SetType(LEVELCONFLAG_EXTRACTLIGHT );
                }
                else if(hint == string("No Instances"))
                {
                    SetType(LEVELCONFLAG_NOINSTANCES );
                }
                else if(hint == string("ShortCut"))
                {
                    SetType(LEVELCONFLAG_SHORTCUT );
                }

                else
                    AssertMsg(0,"hint = " << hint);

                assert(oadIter.BytesLeft() == 0);
                SetShowAs(SHOW_AS_HIDDEN);
                SetName("LEVELCONFLAG_ROOM");
                SetMin(0);
                SetMax(0);
                SetDef(0);
                SetString("");
                break;
            }

        default:
            cout << "chunk " << oadIter.GetChunkID() << " not supported" << endl;
            assert(0);
            break;
    }

    Validate();
    DBSTREAM2(cdebug << "OADE::LIFFE:ending ID = " << oadIter.GetChunkID() << endl; )
}

//============================================================================
// this loads from the .iff file on disk

bool
QObjectAttributeDataEntry::LoadIFF(IFFChunkIter& oadIter, ostream& error, vector<QObjectAttributeDataEntry>& entries )
{
    DBSTREAM2(cdebug << "OADE::LIFF: begin, ID = " << oadIter.GetChunkID() << ", bytesleft = " << oadIter.BytesLeft() << endl; )
	assert(!_compiledScript);
	assert(!_compiledScriptLength);
	_scriptText = NULL;

    switch(oadIter.GetChunkID().ID())
    {
        case IFFTAG('S','T','R','U'):
        {
            DBSTREAM2(cdebug << "OADE::LIFF: STRU tag, oadIter.BytesLeft = " << oadIter.BytesLeft() << endl; )
            bool flagFound = false;
			bool entryOutputYet = false;

            SetType(BUTTON_GROUP_START);
            SetShowAs(SHOW_AS_HIDDEN);
            SetMin(0);
            SetMax(0);
            SetDef(0);
            SetString("");

            while(oadIter.BytesLeft() > 0)
            {
                IFFChunkIter* chunkIter = oadIter.GetChunkIter(*levelconRM);
            	DBSTREAM2(cdebug << "OADE::LIFF: processing tag " <<  chunkIter->GetChunkID()  << endl; ) 
                switch(chunkIter->GetChunkID().ID())
                {

                    case IFFTAG('O','P','E','N'):
                        {
                            int32 open = IFFReadBool(*chunkIter);
                            // just discard, we don't need it
                            assert(chunkIter->BytesLeft() == 0);
                            break;
                        }

                    case IFFTAG('N','A','M','E'):
                        {
                            string name = IFFReadString(*chunkIter);
                            SetName(name.c_str());
                            assert(chunkIter->BytesLeft() == 0);
                            break;
                        }
                    case IFFTAG('F','L','A','G'):
                        {
                            string flag = IFFReadString(*chunkIter);
                            assert(chunkIter->BytesLeft() == 0);

                            if(flag == "COMMONBLOCK")
                            {
                                SetType(LEVELCONFLAG_COMMONBLOCK);
                                flagFound = true;
                            }
                            else
                                assert(0);
                            break;
                        }

                    case IFFTAG('S','T','R','U'):
                    {
                        DBSTREAM2(cdebug << "OADE::LIFF: STRU begin" << endl; )

						// first output our parent stru if we haven't yet (defered to find out what type it is via the FLAG chunk
                        if(!entryOutputYet)
                        {
                        	DBSTREAM2(cdebug << "Hey, outputing our start entry: " << *this << endl; )
                            // add oneself
                            entries.push_back(*this);
							entryOutputYet = true;
                        }

						// now output this stru start tag
						// kts don't bother, we don't care what it is							   		

                        // then add all children

                        QObjectAttributeDataEntry entry;
                        entry.LoadIFF(*chunkIter, error,entries);

						// here is where we would output the stru end tag

                        DBSTREAM2(cdebug << "OADE::LIFF: STRU end" << endl; )
                        break;

                    }
                    default:
                        // forward anything else to LoadIFFEntry

						// first output our parent stru if we haven't yet (defered to find out what type it is via the FLAG chunk
                        if(!entryOutputYet)
                        {
                        	DBSTREAM2(cdebug << "Hey, outputing our start entry: " << *this << endl; )
                            // add oneself
                            entries.push_back(*this);
							entryOutputYet = true;
                        }

                        DBSTREAM2(cdebug << "OADE::LIFF: forwarding " << chunkIter->GetChunkID() << endl; )
						QObjectAttributeDataEntry temp;
                        temp.LoadIFFEntry(*chunkIter,error);
                        DBSTREAM2(cdebug << "OADE::LIFF: after forwarded " << chunkIter->GetChunkID() << endl; )

                        temp.Validate();
                        entries.push_back(temp);
                        //entries.push_back(*this);
                        break;
                }
                MEMORY_DELETE(*levelconRM,chunkIter,IFFChunkIter);
            }

            // add end tag
			if(flagFound)
			{
                SetType(LEVELCONFLAG_ENDCOMMON);
				SetName("LEVELCONFLAG_ENDCOMMON");
			}
			else
			{
                SetType(BUTTON_GROUP_STOP);
				SetName("STOP");
			}
            DBSTREAM2(cdebug << "OADE::LIFF: adding end tag" << endl; )
            DBSTREAM2(cdebug << "		this dump: " << *this << endl; )
            entries.push_back(*this);

            break;
        }
        default:
            // forward anything else to LoadIFFEntry
            DBSTREAM2(cdebug << "OADE::LIFF: forwarding tag " << oadIter.GetChunkID() << " to LoadIFFEntry" << endl; )
            LoadIFFEntry(oadIter,error);
            Validate();
            entries.push_back(*this);
            break;
    }


#if 0
// kts we don't always push ourselves onto the stack, so we are not always valid
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
#endif
    DBSTREAM2(cdebug << "OADE::LIFF: ending  ID = " << oadIter.GetChunkID() << endl; )
	return(true);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeDataEntry &oad)
{
	//s << "QObjectAttributeDataEntry at address: " << &oad << endl;
   assert(ValidPtr(&oad));
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
	//s << "string length = " << oad.GetString().length() << endl;
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

	s << "Showas = " << int(oad.GetShowAs()) << endl;
	s << "ScriptText = ";
	if(oad._scriptText)
		s << "valid";
	else
		s << "NULL";
	s << endl;

	return(s);
}

//============================================================================

ostream& operator<<(ostream& s, const QObjectAttributeData &oad)
{
	s << "Oad Header Name:" << oad.header.name << endl;

    s << "entries = " << oad.entries.size() << endl;
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

//==============================================================================

bool
QObjectAttributeData::LoadIFF(binistream& input, ostream& error)
{
    string name;
	assert(input.good());
    IFFChunkIter oadIter(input);

	assert(oadIter.GetChunkID().ID() == IFFTAG('T','Y','P','E'));
	QObjectAttributeDataEntry entry;

    DBSTREAM2(cdebug << "OAD::LIFF: begin, ID = " << oadIter.GetChunkID() << ", bytesleft = " << oadIter.BytesLeft() << endl; )

	while(oadIter.BytesLeft() > 0)
	{
		IFFChunkIter* chunkIter = oadIter.GetChunkIter(*levelconRM);
//		ciffread << "chunkid = " << chunkIter->GetChunkID() << std::endl;
		switch(chunkIter->GetChunkID().ID())
		{
			case IFFTAG('N','A','M','E'):
			{
                name = IFFReadString(*chunkIter);
				strncpy(header.name,name.c_str(),72-4);
				assert(chunkIter->BytesLeft() == 0);
				break;
			}
            default:                        // anything other than name we just pass down
                DBSTREAM2(cdebug << "OAD::LIFF: forwarding " << chunkIter->GetChunkID() << endl; )
                if(entry.LoadIFF(*chunkIter,error,entries))         // will push itself onto entries
                 {
                    //entries.push_back(entry);
                    DBSTREAM2(cdebug << "OAD::LIFF: LoadIFF returned ok: " << chunkIter->GetChunkID() << endl; )
                 }
                else
                 {
                    if(input.good())
                     {
                        AssertMessageBox(0, "QObjectAttributeData::Load: OAD entry load failed");
                     }
                 }
				break;
		}
        DBSTREAM2(cdebug << "OAD::LIFF: deleting chunk iter for " << chunkIter->GetChunkID() << endl; )
		MEMORY_DELETE(*levelconRM,chunkIter,IFFChunkIter);
	}

	DBSTREAM3( cdebug << "Loaded Object Header name <" << name << "> as :" << *this << endl; )
	return(true);
}

//============================================================================
// load from a stream which has no header

bool
QObjectAttributeData::LoadEntries(istream& input, ostream& error)
{
    assert(0);
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
// this applies an entry from an .iff file

bool
QObjectAttributeData::Apply(IFFChunkIter& objChunkIter, string objName)				
{
	DBSTREAM3( cdebug << "QObjectAttributeData::Apply: called on object <" << objName << ">" << endl; )				
	bool error = false;
#define TEMPBUFFERSIZE 10000
	char buffer[TEMPBUFFERSIZE];

	switch(objChunkIter.GetChunkID().ID())
	{
		case 'RTS':
		case '23I' :
		case '23XF':	
		case '3CEV':
		case 'TAUQ':
		case '3XOB':
		case 'ELIF' :
									// process a single data entry
		{
			int32 def = 0;
			string name = "(unnamed)";
			string oadString = "";
			string scriptText = "";
			QObjectAttributeDataEntry* oadEntry = NULL;
			while(objChunkIter.BytesLeft())
			{
				IFFChunkIter* primChunkIter = objChunkIter.GetChunkIter(*levelconRM);
				switch(primChunkIter->GetChunkID().ID())
				{
					// 'NAME' must come before the data (either 'STR' or 'DATA'
					case 'EMAN':
					{
						primChunkIter->ReadBytes(&buffer,primChunkIter->Size());
						name = buffer;

						oadEntry = _GetEntryByName(name);
						if(oadEntry)
						{
							cdebug << "QObjectAttributeData::Apply: entry named " << name << " found in oaddata:" << *oadEntry << endl;
						}
						 //	;
						else
                  {
							cdebug << "QObjectAttributeData::Apply: entry named " << name << " not found in oaddata" << endl;
							cwarn << "QObjectAttributeData::Apply: entry named " << name << " not found in oaddata, not applying" << endl;
                     delete primChunkIter;
                     objChunkIter.SkipBytes(objChunkIter.BytesLeft());
                     return 0;
                  }
					}
						break;
					case 'ATAD':
                  cdebug << "processing prim chunk of " << objChunkIter.GetChunkID().ID() << endl;
						if(oadEntry)
						{
                     cdebug <<  " oadentry  = " << *oadEntry << endl;
							assert(primChunkIter->Size() < TEMPBUFFERSIZE);
							primChunkIter->ReadBytes(&buffer,primChunkIter->Size());
							buffer[primChunkIter->Size()] = 0;  // zero terminate, in case it is a string			

							switch(oadEntry->GetType())
							{
								case LEVELCONFLAG_SHORTCUT:
								{
                					AssertMessageBox(0, "Object <" << objName << "> contains LEVELCONFLAG_SHORTCUT" );
									break;
								}

								case LEVELCONFLAG_COMMONBLOCK:
									def = *((int32*)buffer);
									break;

								case BUTTON_FIXED32:
								{
									//double d = eval( buffer, NULL );
									//def = long( d * 65536.0 );
									////def = atof( buffer ) * 65536.0;
									//AssertMsg( def == long( atof( buffer ) * 65536.0 ), "buffer = <" << (char*)buffer << ">, def = " << def << ", atof = " << long( atof( buffer ) * 65536.0 ) );
									def = *((int32*)buffer);
									
									break;
								}

								case BUTTON_INT32:
								{
									cdebug << "int32: " << objName << "," << name << ", def = " << def << ", showas = " << int(oadEntry->GetShowAs()) << endl;
									
									switch ( oadEntry->GetShowAs() )
									{
										// these store their data in def
										case SHOW_AS_N_A:
										case SHOW_AS_NUMBER:
										case SHOW_AS_SLIDER:        
										case SHOW_AS_HIDDEN:
										case SHOW_AS_CHECKBOX:
										case SHOW_AS_COLOR:         
										case SHOW_AS_RADIOBUTTONS:  
											def = *((int32*)buffer);
											cdebug << "doing it: " << objName << "," << name << ", def = " << def << endl;
											break;
										// these store their data in string, need to look up the index
										case SHOW_AS_DROPMENU:      
											AssertMsg(0,"I32 with a showas of " << int(oadEntry->GetShowAs()) << " in object " << objName << ", entry named " << name << ", should not have a DATA chunk");
//										{
//											def = *((int32*)buffer);
//											oadString = string(buffer);
//											//oadEntry->SetString(buffer);
//											break;
//										}
											break;
										case SHOW_AS_TOGGLE:        
										case SHOW_AS_COMBOBOX:		
										case SHOW_AS_MAILBOX:			
										default:
											assert(0);
											break;
									}
								}

								case BUTTON_FILENAME:
								case BUTTON_STRING:
									oadString = string(buffer);
									cdebug << "DATA: writing string <" << buffer << "> to field <" << name << "> in object <" << objName << ">, oadEntry = " << *oadEntry << endl;
									break;
								case BUTTON_OBJECT_REFERENCE:
									// do nothing, happens in string handler
									//oadEntry->SetString(buffer);
									oadString = string(buffer);
									cdebug << "DATA: object reference in object named " << objName << ", ref = <" << oadString << "<" << endl;
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
								case BUTTON_CAMERA_REFERENCE:
//									oadEntry->SetString(buffer);
									oadString = string(buffer);
									break;
								case BUTTON_LIGHT_REFERENCE:
//									oadEntry->SetString(buffer);
									oadString = string(buffer);
									break;
								case LEVELCONFLAG_ROOM:
								case LEVELCONFLAG_ENDCOMMON:
									break;
                        case BUTTON_XDATA:
									scriptText = string(buffer);
									oadString = oadEntry->GetString();
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
//									oadEntry->SetString(buffer);
									oadString = string(buffer);
									break;

								case BUTTON_GROUP_START:
								case BUTTON_GROUP_STOP:
									break;

								case BUTTON_INT16:
								case BUTTON_FIXED16:
								case BUTTON_INT8:
								case LEVELCONFLAG_EXTRACTCAMERA:		// this field burns no space
									cerror << "QObjectAttributeDataEntry::Apply:Unknown oad type <" << oadEntry->GetType() << ">" << endl;
									AssertMessageBox(0, "Object " << objName << " in the .MAX file has bad data.  Re-edit it and try again.");
									break;
								default:
									AssertMsg(0,"oadEntry Type " << oadEntry->GetType() << "not supported");
							}
							DBSTREAM3( if(oadString.size())
								cdebug << "    oadString = " << oadString << endl; )
						}
						else
							cdebug << "QObjectAttributeData::Apply: oadEntry bad for object named " << name<< endl;
							//cdebug << "QObjectAttributeData::Apply:don't know how to parse " << name <<  " yet" << endl;

//								assert(0);
						break;
					case 'RTS':
                  cdebug << "processing prim chunk of " << objChunkIter.GetChunkID().ID() << endl;
						if(oadEntry)
						{
                     cdebug <<  " oadentry  = " << *oadEntry << endl;
							AssertMsg(primChunkIter->Size() < TEMPBUFFERSIZE, "size = " << primChunkIter->Size() << ", TEMPBUFFERSIZE = " << TEMPBUFFERSIZE);
							primChunkIter->ReadBytes(&buffer,primChunkIter->Size());
							buffer[primChunkIter->Size()] = 0;  // zero terminate, in case it is a string			

							switch(oadEntry->GetType())
							{
								case LEVELCONFLAG_SHORTCUT:
								{
                					AssertMessageBox(0, "Object <" << objName << "> contains LEVELCONFLAG_SHORTCUT" );
									break;
								}

								case LEVELCONFLAG_COMMONBLOCK:
									assert(0);
								case BUTTON_FIXED32:  			
									oadString = string(buffer);
									//oadEntry->SetString(buffer);
									break;

								case BUTTON_INT32:    // the string is the enumeration
									cdebug << "STR: writing string <" << buffer << "> to field <" << name << "> in object <" << objName << ">, oadEntry = " << *oadEntry << endl;
									oadString = string(buffer);
//									oadEntry->SetString(buffer);
									break;
								case BUTTON_FILENAME:
								case BUTTON_STRING:
									//oadEntry->SetString(buffer);
									oadString = string(buffer);
									cdebug << "STR: writing string <" << buffer << "> to field <" << name << "> in object <" << objName << ">, oadEntry = " << *oadEntry << endl;
									break;
								case BUTTON_OBJECT_REFERENCE:
									cdebug << "STR:object reference in object named " << objName << ", ref = " << oadString << endl;
									oadString = string(buffer);
									//oadEntry->SetString(buffer);
									cdebug << "STR: writing string <" << buffer << "> to field <" << name << "> in object <" << objName << ">" << endl;
									break;
								case LEVELCONFLAG_NOINSTANCES:
								case LEVELCONFLAG_NOMESH:
								case BUTTON_PROPERTY_SHEET:
								case LEVELCONFLAG_SINGLEINSTANCE:
								case LEVELCONFLAG_TEMPLATE:
									assert(0);
									break;

								case BUTTON_CAMERA_REFERENCE:
									assert(0);
									break;
								case BUTTON_LIGHT_REFERENCE:
								case LEVELCONFLAG_ROOM:
								case LEVELCONFLAG_ENDCOMMON:
									assert(0);
									break;
								case BUTTON_XDATA:
									scriptText = string(buffer);
									oadString = oadEntry->GetString();
                           cerr << __FILE__ << ":" << __LINE__ << ": kts make max .lev exporter put scripts in STR chunk instead of DATA chunk" << endl;
									break;
								case BUTTON_EXTRACT_CAMERA:
								case LEVELCONFLAG_EXTRACTCAMERANEW:		// this field burns no space
								case LEVELCONFLAG_EXTRACTLIGHT:
								case BUTTON_WAVEFORM:
									assert(0);
									break;
								case BUTTON_CLASS_REFERENCE:
									oadString = string(buffer);
									//oadEntry->SetString(buffer);
									cdebug << "STR: writing string <" << buffer << "> to field <" << name << "> in object <" << objName << ">" << endl;
									break;

								case BUTTON_GROUP_START:
								case BUTTON_GROUP_STOP:
								case BUTTON_INT16:
								case BUTTON_FIXED16:
								case BUTTON_INT8:
								case LEVELCONFLAG_EXTRACTCAMERA:		// this field burns no space
									cerror << "QObjectAttributeDataEntry::Apply:Unknown oad type <" << oadEntry->GetType() << ">" << endl;
									AssertMessageBox(0, "Object " << objName << " in the .MAX file has bad data.  Re-edit it and try again.");
									break;
								default:
									AssertMsg(0,"oadEntry Type " << oadEntry->GetType() << "not supported");
							}
							DBSTREAM3( if(oadString.size())
								cdebug << "    oadString = " << oadString << endl; )
						}
						else
							cdebug << "QObjectAttributeData::Apply: oadEntry bad for object named " << name << " in chunk STR" << endl;
							//cdebug << "QObjectAttributeData::Apply:don't know how to parse " << name <<  " yet" << endl;

//								assert(0);
						break;
					default:
						AssertMsg(0,"unknown chunk <" << primChunkIter->GetChunkID() << ">, " << 'RTS' << " " << primChunkIter->GetChunkID().ID());
						break;
				}
				delete primChunkIter;
			}

			assert(oadEntry);

			DBSTREAM3( cdebug << "QObjectAttributeData::Apply: Entry:" << endl; )
			DBSTREAM3( cdebug << "    Type  = " << int(oadEntry->GetType()) << endl; )
			DBSTREAM3( cdebug << "    Name = " << name << endl; )
			DBSTREAM3( cdebug << "    def = " << def << endl; )
			DBSTREAM3( cdebug << "    whole entry = " << *oadEntry << endl; )
//							DBSTREAM3( if(oadString)
//								cdebug << "    oadString = " << oadString << endl; )
			//DBSTREAM3( cdebug << "    length = " << length << endl; )

			DBSTREAM3( cdebug << "  Applying to entry: " << *oadEntry << endl; )
			// kts only copy def and oadString

			if ( oadEntry->GetType() == BUTTON_INT32
			&& ( ( oadEntry->GetShowAs() == SHOW_AS_DROPMENU ) || ( oadEntry->GetShowAs() == SHOW_AS_RADIOBUTTONS ) )
			)
			{	// Look up oadString in oadEntry's string enumeration
				// look up oadString in oadEntry->string
				DBSTREAM3( cdebug << "DROPMENU or RADIOBUTTONS found on " << *oadEntry << endl; )
				int idxButton = 0;
				char szButtonNames[ 128 ];
				strcpy( szButtonNames, oadEntry->GetString().c_str() );
				char* pButtonName = szButtonNames;
				char* pEndButtonNames = szButtonNames + strlen( szButtonNames );
				while ( pButtonName != pEndButtonNames )
				{
					char* pSeparator = strchr( pButtonName, '|' );
					if ( pSeparator )
						*pSeparator = '\0';


					DBSTREAM3( cdebug << "comparing <" << oadString << "> with <" << pButtonName << ">" << endl; )
					if ( oadString == pButtonName )
					{
						DBSTREAM3( cdebug << "  match found" << endl; )
						def = idxButton;	// TODO: +oadEntry->GetMin()
						oadString = "";
						break;
					}

					pButtonName = pSeparator ? pSeparator + 1 : pEndButtonNames;
					++idxButton;
				}
				//TODO: assert not at end of list
			}

			oadEntry->SetDef(def);
			DBSTREAM3( cdebug << "after setdef of " << def << endl; )

			AssertMessageBox( oadEntry->GetMin() <= oadEntry->GetMax(), "Invalid oad entry in object <" << objName << ">, field <" << oadEntry->GetName() << ">" << endl << "Min = " << oadEntry->GetMin() << ", max = " << oadEntry->GetMax() << ", def = " << oadEntry->GetDef() );  
			AssertMessageBox( oadEntry->GetDef() >= oadEntry->GetMin(), "Invalid oad entry in object <" << objName << ">, field <" << oadEntry->GetName() << ">" << endl << "Min = " << oadEntry->GetMin() << ", max = " << oadEntry->GetMax() << ", def = " << oadEntry->GetDef() );
			AssertMessageBox( oadEntry->GetDef() <= oadEntry->GetMax(), "Invalid oad entry in object <" << objName << ">, field <" << oadEntry->GetName() << ">" << endl << "Min = " << oadEntry->GetMin() << ", max = " << oadEntry->GetMax() << ", def = " << oadEntry->GetDef() );

				assert(oadEntry->GetMin() <= oadEntry->GetMax());
				assert(oadEntry->GetDef() >= oadEntry->GetMin());
				assert(oadEntry->GetDef() <= oadEntry->GetMax());

			//DBSTREAM3( cdebug << "calling set string" << endl; )
			oadEntry->SetString(oadString);

			if(oadString.length())
				oadEntry->SetString(oadString);

            if( 
               ((oadEntry->GetType() == BUTTON_FILENAME) && (oadEntry->GetDef() == 0)) ||
               ((oadEntry->GetType() == BUTTON_STRING) && (oadEntry->GetDef() == 0)) 
              )
	 		{
                if(oadEntry->GetType() == BUTTON_STRING)
                    assert(oadEntry->GetShowAs() == SHOW_AS_FILENAME);

				DBSTREAM3( cdebug << "copying model type" << endl; )

				const QObjectAttributeDataEntry* modelType = GetEntryByName("Model Type");
				if(
					(oadEntry->GetName() == string("mesh name")) &&
                    ((modelType != NULL) && (modelType->GetDef()==MODEL_TYPE_MESH || modelType->GetDef()==MODEL_TYPE_SCARECROW))
                    )
				{
					assert(modelType->GetDef() < MODEL_TYPE_MAX);
					cerror << "Levelcon Error in apply: required asset <" << oadEntry->GetName() << "> missing from object (see next line)" << endl;
					error = true;
				}
	 		}

			// If this is a Script, copy the pointer to the actual script text
			if(oadEntry->GetType() == BUTTON_XDATA && 
				(
				(oadEntry->GetXDataConversionAction() == XDATA_SCRIPT)
				||(oadEntry->GetXDataConversionAction() == XDATA_COPY)
				)
			  )
			{
				//DBSTREAM3( cdebug << "Found a Script at <" << hex << (long)scriptText << dec << ">" << endl; )
				if(scriptText.size())
				{
					cdebug << "Found a Script in object " << objName << " which says " << scriptText.c_str() << endl; 
					oadEntry->SetScriptText(strdup(scriptText.c_str()));
				}
			}

			DBSTREAM3( cdebug << "  Resulting entry:" << *oadEntry << endl; )
		}
			break;
		default:
			AssertMsg(0,"QObjectAttributeData::Apply: unknown chunk <" << objChunkIter.GetChunkID() << ">, " << '23I' << " " << objChunkIter.GetChunkID().ID());
			break;
	}


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
		if(
			name.length() &&
			entries[index].GetName() == name
		  )
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
			assert(GetShowAs() == SHOW_AS_FILENAME);            // only filenames allowed
            size = 4;
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
                assert(td.GetShowAs() == SHOW_AS_FILENAME);            // only filenames allowed
				if (!isInCommonBlock)
				{
					DBSTREAM3( cdebug << "SaveStruct::BUTTON_STRING: saving 4-byte asset ID <" << hex << td.GetDef() << dec << ">" << endl; )
					int32 tempDef = td.GetDef();
					WriteBytes(output,(char*)&tempDef,4);
					totalBytesWritten += 4;
				}
				DBSTREAM3( else )
					DBSTREAM3( cdebug << "\tInside of commonblock: Skipping <" << td.GetName() << ">." << endl; )
				break;

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
