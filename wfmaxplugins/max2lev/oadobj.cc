// oadobj.cc
#include "global.hpp"
#include <stdstrm.hp>
#include "oadentry.hpp"
#include "oad.hpp"
#include "../lib/stl/algo.h"
#include <game/levelcon.h>		

#include "max2lev.hpp"

// stupid microsoft
#define snprintf _snprintf
extern max2ifflvlOptions theOptions;

vector<OadEntry>::const_iterator theOadEntry;

int
parseStringIntoParts( const char* _string, char* _xDataParameters[], int nMaxParameters )
{
	enum { SEPERATOR = '|' };
	int _nItems = 0;

	//debug( (char*)_string );

	char* strBegin = (char*)_string;
	char* strSeperator;

	do
	{
		int len;

		strSeperator = strchr( strBegin, SEPERATOR );

		if ( strSeperator )
			len = strSeperator - strBegin;
		else
		{	// Take to end of string
			len = strlen( strBegin );
		}

		_xDataParameters[ _nItems ] = (char*)malloc( len + 1 );
		assert( _xDataParameters[ _nItems ] );
		strncpy( _xDataParameters[ _nItems ], strBegin, len );
		*( _xDataParameters[ _nItems ] + len ) = '\0';
		//debug( "_xDataParameters[%d]: [%s]", _nItems, _xDataParameters[_nItems] );

		++_nItems;

		strBegin = strSeperator + 1;
	}
	while ( strSeperator );

	assert( _nItems > 0 );

	return _nItems;
}


double parseEnumeration( const char* szSymbolName )
{
	char* tblEnumValues[ 30 ];

	int nEnumValues = parseStringIntoParts( theOadEntry->GetEnumeratedValues(), tblEnumValues, 30 );

	for ( int idxEnum=0; idxEnum < nEnumValues; ++idxEnum )
	{
		if ( strcmp( tblEnumValues[ idxEnum ], szSymbolName ) == 0 )
			break;
	}

	for ( int idxCleanup=0; idxCleanup < nEnumValues; ++idxCleanup )
	{
		assert( tblEnumValues[ idxCleanup ] );
		free( tblEnumValues[ idxCleanup ] );
	}

	return ( idxEnum == nEnumValues ) ? -1.0 : double( idxEnum );
}

//==============================================================================

void
HDump(const char* data, int len)
{
	(*debuglog) << "Hdump:" << endl;
	while(len--)
	{
		(*debuglog) << *data++;
	}

	(*debuglog) << endl;
}


//==============================================================================

OadEntry*
GetOadEntryByName(vector< OadEntry >& _tblOad,string name)
{
//	OadEntry tempOE;
//	tempOE.SetString( "lightType" );
//	vector<OadEntry>::iterator entry = find( _tblOad.begin(), _tblOad.end(), tempOE );
//	if ( entry != _tblOad.end() )
//		return &(*entry);

		for( vector<OadEntry>::iterator tempOadEntryIter = _tblOad.begin(); tempOadEntryIter != _tblOad.end(); ++tempOadEntryIter )
		{
			if(tempOadEntryIter->GetName() == name)
				return &(*tempOadEntryIter);
		}
	return NULL;
}


ostream&
operator<<(ostream& out, Class_ID& classid)
{
	out << classid.PartA() << "," << classid.PartB();
	return out;
}

ostream&
operator<<(ostream& out, const Point3& p)
{
	out << "X: " << p.x << ", Y: " << p.y << ", Z: " << p.z;
	return out;
} 


void
process_object_oad( INode* thisNode, _IffWriter* _iff )
{
	assert( thisNode );
	assert( _iff );

	DEBUGLOG((*debuglog) << "\nprocess_object_oad: " << endl;)

	AppDataChunk* appDataChunk = thisNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
	if ( !appDataChunk )
		return;
	string className = appDataChunk ? (char*)(appDataChunk->data) : DEFAULT_CLASS;
//	if ( className == string( "Disabled" ) )
//		return;

	_iff->enterChunk( ID( "STR" ) );
		_iff->enterChunk( ID( "NAME" ) );
			*_iff << "Class Name";
		_iff->exitChunk();
		_iff->enterChunk( ID( "DATA" ) );
			*_iff << className.c_str();
		_iff->exitChunk();
	_iff->exitChunk();

	DEBUGLOG((*debuglog) << "\nprocess_object_oad: GetAppDataChunk" << endl;)
	AppDataChunk* adOad = thisNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD );
	if ( !adOad )
		return;
	assert( adOad );

	DEBUGLOG((*debuglog) << "\nprocess_object_oad: new OAD named " << className.c_str() << endl;)
	Oad* pOad;
	pOad = new Oad( className.c_str() );
	assert( pOad );

	DEBUGLOG((*debuglog) << "\nprocess_object_oad: create vector" << endl;)
	vector< OadEntry > _tblOad;

	typeDescriptor* td = pOad->startOfTypeDescriptors();
	int len = pOad->len();
	typeDescriptor* pEndOfTypeDescriptors = (typeDescriptor*)( ((char*)td) + len );
	int size = 0;
	DEBUGLOG((*debuglog) << "\nprocess_object_oad: push_back loop" << endl;)
	for ( ; size<pOad->len(); size += sizeof( typeDescriptor ), ++td )
	{
		_tblOad.push_back( OadEntry( *td ) );
	}
	assert( td == pEndOfTypeDescriptors );

	DEBUGLOG((*debuglog) << "oadentry dump for type " << className << endl;)
//	DEBUGLOG(
//	{
//	for ( vector<OadEntry>::const_iterator oadEntry = _tblOad.begin(); oadEntry != _tblOad.end(); ++oadEntry )
//		(*debuglog) << *oadEntry << endl;
//	}
//	)
	DEBUGLOG((*debuglog) << "\nprocess_object_oad: calc start & end" << endl;)
	{ // Apply fields stored in AppData overriding the defaults contained in the .oad file
	const char* xdataStart = (const char*)adOad->data;
	char* xdata = const_cast<char*>( xdataStart );
	const char* xdataEnd = xdataStart + adOad->length;


	//DEBUGLOG((*debuglog) << "\nlength  = " << adOad->length << endl;)
	//HDump(xdataStart, adOad->length);

	DEBUGLOG((*debuglog) << "\nprocess_obhject_oad: entering while loop" << endl;)
	while ( xdata < xdataEnd )
	{
		OadEntry oe( xdata );
		DEBUGLOG((*debuglog) << "applying xdata: " << oe << endl;)
		vector<OadEntry>::iterator entry = find( _tblOad.begin(), _tblOad.end(), oe );
		if ( entry != _tblOad.end() )
		{
			theOadEntry = entry;
			DEBUGLOG((*debuglog) << "....comparing to " << theOadEntry << endl;)

			switch ( entry->GetButtonType() )
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
				case BUTTON_PROPERTY_SHEET:
					// ignore because they're not going to get stored [and should have been here]
					break;

				case BUTTON_FIXED32:
				{
#pragma message ("KTS " __FILE__ ": put eval back in")
					//double e = ::eval( oe.GetString(), parseEnumeration );
					double e = atoi( oe.GetString() );
					entry->GetTypeDescriptor()->def = long( e * 65536.0 );
					strcpy( entry->GetTypeDescriptor()->string, oe.GetString() );
					break;
				}

				case BUTTON_INT32:
				{
#pragma message ("KTS " __FILE__ ": put eval back in")
					//double e = ::eval( oe.GetString(), parseEnumeration );
					double e = atoi( oe.GetString() );
					DEBUGLOG((*debuglog) << "\nprocess_obhject_oad:BUTTON_INT32: evaluating button named" << entry->GetDisplayName() << ", string = " << oe.GetString() << ", result = " << e << endl;)
					entry->GetTypeDescriptor()->def = e;
					strcpy( entry->GetTypeDescriptor()->string, oe.GetString() );
					break;
				}

				case BUTTON_STRING:
				case BUTTON_OBJECT_REFERENCE:
				case BUTTON_FILENAME:
				case BUTTON_CAMERA_REFERENCE:
				case BUTTON_LIGHT_REFERENCE:
				case BUTTON_MESHNAME:
				case BUTTON_CLASS_REFERENCE:
				{
					strcpy( entry->GetTypeDescriptor()->string, oe.GetString() );
					break;
				}

				case BUTTON_XDATA:
				{
					assert(oe.GetXData());
					assert(entry->GetXData());
					entry->SetXData(oe.GetXData());
					break;
				}
			} // switch to update database from objects's oad data
		} // if found
		else
		{
			DEBUGLOG((*debuglog) << "\nfailed to find oad " << oe << " in _tblOad" << endl;)
		}
	} // still data
	DEBUGLOG((*debuglog) << "\nprocess_obhject_oad: exit while loop" << endl;)
	assert( xdata == xdataEnd );
	}

    {
	// now handle special cases like light colors
	for ( vector<OadEntry>::iterator oadEntry = _tblOad.begin(); oadEntry != _tblOad.end(); ++oadEntry )
	{
		//DEBUGLOG((*debuglog) << "checking field named " << oadEntry->GetName() << " aganst LCF_EL" << endl;)

		if(!strcmp(oadEntry->GetName(),"LEVELCONFLAG_EXTRACTLIGHT"))
		{
			DEBUGLOG((*debuglog) << "Found it!" << endl;)

			TimeValue theTime = 0;		// theSceneEnum->time
			char* thisObjectName = thisNode->GetName();
			assert(thisObjectName);

			DEBUGLOG((*debuglog) << "thisObjectName = " << thisObjectName << endl;)

		// If this is a light object, we need to patch some OAD entries...
		//if (objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_EXTRACTLIGHT))
		//{
			LightObject *obj = (LightObject*)(thisNode->EvalWorldState(theTime).obj);
			assert(obj);
			DEBUGLOG((*debuglog) << "obj = " << obj << endl;)
			AssertMessageBox(obj->SuperClassID()==LIGHT_CLASS_ID, "Object <" << thisObjectName << ">:" << endl << "This doesn't look like a light to me!");
			Interval valid;
			LightState ls;
			AssertMessageBox( obj->EvalLightState(theTime, valid, &ls)==REF_SUCCEED, "Light <" << thisObjectName << ">" << endl << "This object's data is corrupt!" );


//			OadEntry tempOE;
//			tempOE.SetStrin( "lightType" );
//			vector<OadEntry>::iterator entry = find( _tblOad.begin(), _tblOad.end(), tempOE );
//			if ( entry != _tblOad.end() )
			
//			DEBUGLOG((*debuglog) << "searching for lightType in :" << endl;)
//			for(int index=0;index<_tblOad.size();index++)
//				(*debuglog) << _tblOad[index] << endl;


			OadEntry* tempEntry = GetOadEntryByName(_tblOad,"lightType");
			assert(tempEntry);
			//DEBUGLOG((*debuglog) << "tempEntry lightType = (" << tempEntry <<")" << *tempEntry << endl;)
			ls.tm.NoTrans();
			DEBUGLOG((*debuglog) << "after trans" << endl;)
			ls.tm.NoScale();
			DEBUGLOG((*debuglog) << "after scale" << endl;)
			Point3 lightVector = VectorTransform(ls.tm, Point3(0,0,-1));
			DEBUGLOG((*debuglog) << "after transform" << endl;)
#define TEMP_STRING_LEN 100
			char tempString[TEMP_STRING_LEN];
			switch (ls.type)
			{
				case OMNI_LIGHT:
					DEBUGLOG((*debuglog) << "OMNI_LIGHT" << endl;)
					tempEntry->SetDef( AMBIENT_LIGHT );
					tempEntry->SetString( "Ambient" );
					break;

				case DIR_LIGHT:
					DEBUGLOG((*debuglog) << "DIRECTIONAL_LIGHT" << endl;)
					tempEntry->SetDef( DIRECTIONAL_LIGHT );
					tempEntry->SetString( "Directional" );

					tempEntry = GetOadEntryByName(_tblOad,"lightX");
					assert(tempEntry);
			DEBUGLOG((*debuglog) << "tempEntry: lightX" << endl;)
					_snprintf(tempString,TEMP_STRING_LEN,"%f",lightVector.x);
//					tempEntry->SetString( tempString);
					//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.x) );
					tempEntry = GetOadEntryByName(_tblOad,"lightY");
					assert(tempEntry);
			DEBUGLOG((*debuglog) << "tempEntry: lightY" << endl;)
//					_snprintf(tempString,TEMP_STRING_LEN,"%f",lightVector.y);
					tempEntry->SetString( tempString);
					//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.y) );
					tempEntry = GetOadEntryByName(_tblOad,"lightZ");
					assert(tempEntry);
			DEBUGLOG((*debuglog) << "tempEntry: lightZ" << endl;)
					_snprintf(tempString,TEMP_STRING_LEN,"%f",lightVector.z);
//					tempEntry->SetString( tempString);
					//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.z) );
					break;

				default:
					AssertMessageBox(0, "Light <" << thisObjectName << ">" << endl << "Must be either OMNI or DIRECTIONAL" << endl << "(Current value = " << (short)(ls.type) << ")");
					break;
			}

			DEBUGLOG((*debuglog) << "after switch" << endl;)
			tempEntry = GetOadEntryByName(_tblOad,"lightRed");
			assert(tempEntry);
			DEBUGLOG((*debuglog) << "tempEntry: lightRed, before: " << *tempEntry << endl;)
			_snprintf(tempString,TEMP_STRING_LEN,"%f",ls.color.r);
			tempEntry->SetString( tempString);
			//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.r) );
			DEBUGLOG((*debuglog) << "tempEntry: lightRed, after: " << *tempEntry << endl;)
			tempEntry = GetOadEntryByName(_tblOad,"lightGreen");
			DEBUGLOG((*debuglog) << "tempEntry: lightGreen" << endl;)
			assert(tempEntry);
			_snprintf(tempString,TEMP_STRING_LEN,"%f",ls.color.g);
			tempEntry->SetString( tempString);
			//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.g) );
			tempEntry = GetOadEntryByName(_tblOad,"lightBlue");
			DEBUGLOG((*debuglog) << "tempEntry: lightBlue" << endl;)
			assert(tempEntry);
			_snprintf(tempString,TEMP_STRING_LEN,"%f",ls.color.b);
			tempEntry->SetString( tempString);
			//tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.b) );

			DEBUGLOG((*debuglog) << "already set light in :" << endl;)
			DEBUGLOG(for(int index=0;index<_tblOad.size();index++)
				(*debuglog) << _tblOad[index] << endl; )
		}

	}
	}		                            // so that oadEntry is not re-declared (msvc SUX!)


	// If this object was created using the Slope primitive, we need to patch some OAD entries...
	assert(thisNode);
	TimeValue theTime = 0;		// theSceneEnum->time
	ObjectState os = thisNode->EvalWorldState( theTime );
	assert(os.obj);
	//DEBUGLOG((*debuglog) << "check object named " << thisNode->GetName() << " for slope, class id = " << os.obj->ClassID() << endl;)

//		if ( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID )


	if ( os.obj->ClassID() == Slope_ClassID )
	{
		BOOL needDel;
		NullView nullView;
		//Mesh* thisMesh = ((GeomObject*)os.obj)->GetRenderMesh( theTime, thisNode, nullView, needDel );
		//Mesh* mesh =     ((GeomObject*)os.obj)->GetRenderMesh(    time,  pObject, nullView, needDel );

		GeomObject* go = (GeomObject*)os.obj;
		assert(go);
		AssertMsg(go->SuperClassID() == GEOMOBJECT_CLASS_ID, "Class ID = " << go->SuperClassID());
		Mesh* thisMesh = go->GetRenderMesh( theTime, thisNode, nullView, needDel );
		assert(thisMesh);

		Matrix3 objTM = thisNode->GetObjectTM(theTime);
		Point3 objOffsetPos = thisNode->GetObjOffsetPos();
		Matrix3 nodeTM = thisNode->GetNodeTM(theTime);
		Point3 location = nodeTM.GetTrans();

		DEBUGLOG((*debuglog) << "  at if:" << endl;)
		if ( thisMesh )
		{
			thisMesh->buildNormals();
			Point3 slopeNormal(0,0,0);
			Point3 faceNormal(0,0,0);
			Face   slopeFace;
			for (int faceIndex=0; faceIndex < thisMesh->numFaces; faceIndex++)
			{
				// Find a normal which is off-axis in two directions.  This is a normal to the
				// sloped surface.
				faceNormal = thisMesh->getFaceNormal(faceIndex);

				if ( ((faceNormal.x != 0) && (faceNormal.y != 0)) ||
						((faceNormal.y != 0) && (faceNormal.z != 0)) ||
						((faceNormal.z != 0) && (faceNormal.x != 0)) )
				{
					slopeNormal = VectorTransform(objTM, faceNormal);
					slopeFace = thisMesh->faces[faceIndex];
				}
			}

			// Find D coefficient for the plane equation
			Point3 slopeVertex = VectorTransform(objTM, thisMesh->verts[slopeFace.v[0]]);
			float slopeD = 0 - ((slopeNormal.x * slopeVertex.x) +
								(slopeNormal.y * slopeVertex.y) +
								(slopeNormal.z * slopeVertex.z) );

			// Plug plane equation coefficients into OAD entries
			char tempString[100];
			OadEntry* tempEntry = GetOadEntryByName(_tblOad,"slopeA");
			assert(tempEntry);
			snprintf(tempString,100,"%f",slopeNormal.x);
			tempEntry->SetString(tempString);
			tempEntry = GetOadEntryByName(_tblOad,"slopeB");
			assert(tempEntry);
			snprintf(tempString,100,"%f",slopeNormal.y);
			tempEntry->SetString(tempString);
			tempEntry = GetOadEntryByName(_tblOad,"slopeC");
			assert(tempEntry);
			snprintf(tempString,100,"%f",slopeNormal.z);
			tempEntry->SetString(tempString);
			tempEntry = GetOadEntryByName(_tblOad,"slopeD");
			assert(tempEntry);
			snprintf(tempString,100,"%f",slopeD);
			tempEntry->SetString(tempString);

			DEBUGLOG((*debuglog) << "  slopeNormal = " << slopeNormal << ", slopeD = " << slopeD << endl;)

		}
		else
			AssertMsg(0,"Object " << thisNode->GetName() << " of type Slope must have a mesh");
	}
	// Done with sloped surface code

	// ok, now print it into the iff file
	DEBUGLOG((*debuglog) << "\nprocess_object_oad: entering vector loop" << endl;)
	for ( vector<OadEntry>::const_iterator oadEntry = _tblOad.begin(); oadEntry != _tblOad.end(); ++oadEntry )
	{
		theOadEntry = oadEntry;

		DEBUGLOG( (*debuglog) << "\nchecking oad " << *oadEntry << endl; )
//		strstream str;
//		str << *oadEntry << '\n';
//		OutputDebugString( str.str() );

		ID thisChunkID = oadEntry->GetID();

		DEBUGLOG( (*debuglog) << "defaultOverridden = " << oadEntry->DefaultOverriden() << ", thisChunkID = " << thisChunkID << endl; )

		if( oadEntry->DefaultOverriden() &&  thisChunkID && ( thisChunkID != ID( "N/A" ) ) )
		{
			DEBUGLOG( (*debuglog) << "good one" << endl; )

			_iff->enterChunk( ID( thisChunkID ) );
				_iff->enterChunk( ID( "NAME" ) );
					*_iff << oadEntry->GetName();
				_iff->exitChunk();

					// yuk, I know
					switch ( oadEntry->GetButtonType() )
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
						case BUTTON_PROPERTY_SHEET:
							//assert( 0 );			// Should not have been stored
							break;

						case BUTTON_INT32:
						{
							visualRepresentation showAs = oadEntry->GetVisualRepresentation();

							switch ( showAs) 
							{
								// these store their data in def
								case SHOW_AS_N_A:
								case SHOW_AS_NUMBER:
								case SHOW_AS_SLIDER:        
								case SHOW_AS_TOGGLE:        
								case SHOW_AS_HIDDEN:
								case SHOW_AS_CHECKBOX:
								case SHOW_AS_COLOR:         
								case SHOW_AS_RADIOBUTTONS:  
									_iff->enterChunk( ID( "DATA" ) );
									//*_iff << unsigned long( ::eval( oadEntry->GetString(), NULL ) );
									*_iff << unsigned long( oadEntry->GetTypeDescriptor()->def );
									*_iff << Comment( oadEntry->GetEnumeratedValues() );
									_iff->exitChunk();
									break;
								// these store their data in string, need to look up the index
								case SHOW_AS_DROPMENU:      

									break;
								case SHOW_AS_COMBOBOX:		
								case SHOW_AS_MAILBOX:			
								default:
									assert(0);
									break;
							}

							_iff->enterChunk( ID( "STR" ) );
							*_iff << oadEntry->GetString();
							_iff->exitChunk();
							break;
						}

						case BUTTON_FIXED32:
							_iff->enterChunk( ID( "DATA" ) );
#pragma message ("KTS " __FILE__ ": put eval back in")
							//*_iff << Fixed( theOptions.max2iffOptions.sizeReal, ::eval( oadEntry->GetString(), NULL ) );

							//*_iff << Fixed( theOptions.max2iffOptions.sizeReal,WF_SCALAR_TO_FLOAT(oadEntry->GetDef()));
							float tempFloat;
							sscanf(oadEntry->GetString(),"%f",&tempFloat);
							*_iff << Fixed( theOptions.max2iffOptions.sizeReal,tempFloat);


							_iff->exitChunk();

							_iff->enterChunk( ID( "STR" ) );
							*_iff << oadEntry->GetString();
							_iff->exitChunk();
							break;

						case BUTTON_STRING:
						case BUTTON_OBJECT_REFERENCE:
						case BUTTON_FILENAME:
						case BUTTON_CAMERA_REFERENCE:
						case BUTTON_LIGHT_REFERENCE:
						case BUTTON_MESHNAME:
						case BUTTON_CLASS_REFERENCE:
						case BUTTON_XDATA:
							_iff->enterChunk( ID( "STR" ) );
							_iff->out_string( oadEntry->GetString() );
							_iff->exitChunk();
							break;

						default:
							assert( 0 );
					}
			_iff->exitChunk();
		}
	}
	DEBUGLOG((*debuglog) << "\nprocess_obhject_oad: exit vector loop" << endl;)
	assert( pOad );
	delete pOad;
	DEBUGLOG((*debuglog) << "\nprocess_obhject_oad: done" << endl;)
}
