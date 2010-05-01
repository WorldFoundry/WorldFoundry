//============================================================================
// level.cc:
// Copyright 1997,1998 Recombinant Limited.  All Rights Reserved.
//============================================================================

#include "global.hp"
#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
//#include <fstream.h>
//#include <strstrea.h>
#include <math.h>

#include "level.hp"
#include "levelcon.hp"
//#include "pigtool.h"
#include "model.hp"
#include "object.hp"
#include "oad.hp"
#include "colbox.hp"
#include "room.hp"
#include "3d.hp"
#include "asset.hp"
#include "path.hp"
#include <hal/halbase.h>
#include <streams/binstrm.hp>

//#include <stl/pair.h>
//#include <stl/algo.h>

#pragma pack(1)
extern "C" {
#include <oas/oad.h>			// from the velocity project
};
#pragma pack()

#pragma pack(1)

#pragma pack()

#include <oas/levelcon.h>

#include "hdump.hp"
//#include <stl/bstring.h>

//#include "../lib/wf_id.hp"

//============================================================================

//extern Interface* gMaxInterface;
extern QLevel* theLevel;

//============================================================================

QLevel::QLevel(const char* levelFileName, const char* lcFileName /*, SceneEnumProc* theSceneEnum*/ )
{
	DBSTREAM1 ( cprogress << "QLevel::QLevel:" << endl; )
	theTime = 0;	//WBNIV] theSceneEnum->time;
	// kts added 10/1/96 20:28 to make object index 0 invalid
	assert(objects.size() == 0);

	// Set the global pointer to this object so Path methods can refer to it
	theLevel = this;

	Euler newRotation;
	newRotation.a = 0;
	newRotation.b = 0;
	newRotation.c = 0;
	QColBox collision(Point3(0,0,0),Point3(1,1,1));
	QObjectAttributeData newobjOAD;						// make an empty oad
	int32 oadFlags = 0;
	objects.push_back
	(
		new QObject
		(
			"NULL_Object",
			1,						// object type is not relevent for first object
			Point3(0,0,0),
			Point3(0,0,0),
			newRotation,
			collision,
			oadFlags,
			-1,
			newobjOAD
		)
	);
	assert(objects.size() == 1);
	// end kts addition

	extensionsMap[string(".3ds")] = string(".iff");
	extensionsMap[string(".iff")] = string(".iff");
	extensionsMap[string(".max")] = string(".iff");
	extensionsMap[string(".wrl")] = string(".iff");
	extensionsMap[string(".tga")] = string(".tga");
	extensionsMap[string(".bmp")] = string(".bmp");

#if 0
	meshDirectories.push_back(string("levels\\3ds\\"));
	meshDirectories.push_back(string("levels/3ds/"));
#endif

	commonAreaSize = 0;
	DBSTREAM3( cprogress << "Loading level iff file <" << levelFileName << ">" << endl; )
	DBSTREAM3( cprogress << "Loading .LC file <" << lcFileName << ">" << endl; )
	LoadLCFile(lcFileName);
	LoadAssetFile(lcFileName);	// used to find assets.txt files
	LoadOADFiles(lcFileName);	// used to find the .oad files
	LoadLevelIFFFile(levelFileName);
	//CreateQObjectList();		// This replaces Load3DStudio()
	DBSTREAM3( cprogress << "Updating references, etc." << endl; )
}

//============================================================================

QLevel::~QLevel()
{
	objects.erase(objects.begin(), objects.end());
	channels.erase(channels.begin(), channels.end());
}

//============================================================================
// returns path index

int
QLevel::AddPath(const QPath& newPath)
{
	int pathIndex;
	vector<QPath>::iterator where = find(paths.begin(),paths.end(),newPath);
	if(where == paths.end())
	{
		paths.push_back(newPath);					// create new path
		pathIndex = paths.size() - 1;
	}
	else
	{
		pathIndex = where  - paths.begin();		// use existing path
	}
	return pathIndex;

}

//-----------------------------------------------------------------------------
// load a single 3DS MAX scene node with its .oad data, and add it to the various lists

//#define OAD_CHUNK_NAME "Cave Logic Studios Object Attribute Editor v0.3"
//const char CLASS_CHUNK_NAME[] = "Cave Logic Studios Class Object Editor";

#define WF_VECTOR_TO_POINT(out,in) \
	(out).x = WF_SCALAR_TO_FLOAT((in)[0]); \
	(out).y = WF_SCALAR_TO_FLOAT((in)[1]); \
	(out).z = WF_SCALAR_TO_FLOAT((in)[2]);


struct PathBasePosition
{
	int32 xPos,yPos,zPos;
};
struct PathBaseRotation
{
	int32 q1,q2,q3,q4;
};



#pragma message ("KTS " __FILE__ ": write shortcut support")

void
QLevel::CreateQObjectFromChunk(IFFChunkIter& chunkIter) 
{
	Point3 objectPosition;
	QColBox collision;
	Euler objectOrientation;
	QObjectAttributeData newobjOAD;
	int typeIndex = -1;
	int pathIndex = -1;

	string className = "disabled";
	string thisObjectName = "(unnamed)";
#define TEMPBUFFERSIZE 1000
	char tempBuffer[TEMPBUFFERSIZE];

	while(chunkIter.BytesLeft())
	{
		IFFChunkIter* objChunkIter = chunkIter.GetChunkIter(*levelconRM);
		switch(objChunkIter->GetChunkID().ID())
		{
					// 'NAME' and 'STR'::'Class Name' must come before the oad data
			case  'EMAN':			    // object name
				assert(objChunkIter->Size() < TEMPBUFFERSIZE);
				objChunkIter->ReadBytes(&tempBuffer,objChunkIter->Size());
				thisObjectName = tempBuffer;	
				break;

			case 'RTS':
			case '23I' :
			case '23XF':	
			case '3CEV':
			case 'RLUE':
			case '3XOB':
			case 'ELIF' :
										// process a single data entry
			{
				if(typeIndex != -1)
				{
					// forward all chunks to Apply
					bool error = newobjOAD.Apply(*objChunkIter,thisObjectName);
					if(error)
						cerror << "OAD Error in object " << thisObjectName << endl;
					//DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: OAD after apply " << endl; )
				}
				else
				{
					string name = "(unnamed)";
					while(objChunkIter->BytesLeft())
					{
						IFFChunkIter* primChunkIter = objChunkIter->GetChunkIter(*levelconRM);
						switch(primChunkIter->GetChunkID().ID())
						{
							case 'EMAN':
								primChunkIter->ReadBytes(&tempBuffer,primChunkIter->Size());
								name = tempBuffer;
								break;
							case 'ATAD':
								assert(primChunkIter->Size() < TEMPBUFFERSIZE);
								if(name == "Class Name")
								{
									primChunkIter->ReadBytes(&tempBuffer,primChunkIter->Size());
									assert(objChunkIter->GetChunkID().ID() == '\0RTS');
									className = tempBuffer;	

									typeIndex = GetClassIndex(className);
									if ( typeIndex == -1 )
									{
										AssertMessageBox(typeIndex != -1, "Object with name <" << thisObjectName << "> has invalid Object Type of <" << className << ">");
									}
									DBSTREAM3( cdebug << "Object Type is " << typeIndex << ", which is " << objectTypes[typeIndex] << endl; )
									newobjOAD = objectOADs[typeIndex];  // copy defaults			
								}
								else if(name == "Position")
								{
									assert(objChunkIter->GetChunkID().ID() == '3CEV');
									assert(primChunkIter->Size() == 12);
									assert(primChunkIter->BytesLeft() >= 12);
									assert(sizeof(objectPosition)== 12);
									primChunkIter->ReadBytes(&tempBuffer,12);

									WF_VECTOR_TO_POINT(objectPosition,(fixed32*)tempBuffer);
									cdebug << "position of object " << thisObjectName << " is " << objectPosition << endl;
								}
								else if(name == "Orientation")
								{
									assert(objChunkIter->GetChunkID().ID() == 'RLUE');
									assert(primChunkIter->Size() == 12);
									assert(primChunkIter->BytesLeft() >= 12);
									//assert(sizeof(objectOrientation)== 12);
									primChunkIter->ReadBytes(&tempBuffer,12);
									objectOrientation.a = WF_SCALAR_TO_FLOAT(((int32*)(tempBuffer))[0]);
									objectOrientation.b = WF_SCALAR_TO_FLOAT(((int32*)(tempBuffer))[1]);
									objectOrientation.c = WF_SCALAR_TO_FLOAT(((int32*)(tempBuffer))[2]);
									cdebug << "orientation of object " << thisObjectName << " is " << objectOrientation << endl;
								}

								else if(name == "Global Bounding Box")
								{
									assert(objChunkIter->GetChunkID().ID() == '3XOB');
									assert(primChunkIter->Size() == 24);
									assert(primChunkIter->BytesLeft() >= 24);
									assert(sizeof(collision)== 24);
									primChunkIter->ReadBytes(&tempBuffer,24);
									Point3 min;
									Point3 max;
									WF_VECTOR_TO_POINT(min,(fixed32*)tempBuffer);
									WF_VECTOR_TO_POINT(max,(((fixed32*)tempBuffer)+3));
									collision = QColBox(min,max);
								}

								else
								{
									primChunkIter->ReadBytes(&tempBuffer,primChunkIter->Size());
									cdebug << "QLevel::CreateQObjectFromChunk: don't know how to parse " << name <<  " yet" << endl;
								}
	//								assert(0);
								break;
							case 'RTS':
								cout << "ignoring prim chunk of 'STR'" << endl;
								break;
							default:
								AssertMsg(0,"unknown chunk <" << primChunkIter->GetChunkID() << ">, " << 'RTS' << " " << primChunkIter->GetChunkID().ID());
								break;
						}
						delete primChunkIter;
					}
				}	
			}
				break;
			case 'HTAP':
			{
				enum
				{
					XPOS,
					YPOS,
					ZPOS,
					EULERA,
					EULERB,
					EULERC,
					CHANNELCOUNT
				};

				const char* channelNames[] = 
				{
				"position.x",
				"position.y",
				"position.z",
				"rotation.a",
				"rotation.b",
				"rotation.c"
				};

				Point3 position;
				Quat rotation;
				Channel channelArray[CHANNELCOUNT];
				while(objChunkIter->BytesLeft())
				{
					IFFChunkIter* pathChunkIter = objChunkIter->GetChunkIter(*levelconRM);
					switch(pathChunkIter->GetChunkID().ID())
					{
						case 'SOPB':
						{
							PathBasePosition basePos;
							pathChunkIter->ReadBytes(&basePos,sizeof(basePos));
							position = Point3(WF_SCALAR_TO_FLOAT(basePos.xPos),WF_SCALAR_TO_FLOAT(basePos.yPos),WF_SCALAR_TO_FLOAT(basePos.zPos));
						}
							break;
						case 'TORB':
						{
							PathBaseRotation baseRot;
							pathChunkIter->ReadBytes(&baseRot,sizeof(baseRot));
							rotation = Quat(WF_SCALAR_TO_FLOAT(baseRot.q1),WF_SCALAR_TO_FLOAT(baseRot.q2),WF_SCALAR_TO_FLOAT(baseRot.q3),WF_SCALAR_TO_FLOAT(baseRot.q4));
						}
							break;
						case 'NAHC':
						{
							// create a channel class to fill in
							Channel* currentChannel = NULL;
							int dim = 0;	// # of entries			

							while(pathChunkIter->BytesLeft())
							{
								IFFChunkIter* channelChunkIter = pathChunkIter->GetChunkIter(*levelconRM);
								switch(channelChunkIter->GetChunkID().ID())
								{
									case 'EMAN':
									{
										channelChunkIter->ReadBytes(&tempBuffer,channelChunkIter->Size());
										for(int index=0;index<CHANNELCOUNT;index++)
										{
											if(!strcmp(channelNames[index],tempBuffer))
											{
												assert(currentChannel == NULL);  // should only have one name per channel				
												currentChannel = &channelArray[index];
											}
										}
										if(currentChannel == NULL)
										{
											cerror << "Levelcon Error in apply channel handling: channel named " << tempBuffer << "unknown"  << endl;
										}
									}
										break;
									case 'EPYT':
									{
										assert(channelChunkIter->Size() == 4);
										int type;
										channelChunkIter->ReadBytes(&type,4);
										assert(type == 0);
									}
										break;
									case 'ETAR':
									{
										assert(channelChunkIter->Size() == 4);
										int rate;
										channelChunkIter->ReadBytes(&rate,4);
										//assert(rate != 0);
									}
										break;
									case '\0MID':
									{
										assert(channelChunkIter->Size() == 4);
										channelChunkIter->ReadBytes(&dim,4);
										assert(dim > 0);
									}
										break;
									case 'ATAD':
									{

										struct KeyEntry
										{
											int32 time;
											int32 value;
										};

										assert(ValidPtr(currentChannel));
										//currentChannel->Validate();
										AssertMsg(channelChunkIter->Size() == (dim*sizeof(KeyEntry)),"size = " << channelChunkIter->Size() << ", dim = " << dim);
                                        assert(dim*sizeof(KeyEntry) < TEMPBUFFERSIZE);
										channelChunkIter->ReadBytes(&tempBuffer,(dim*sizeof(KeyEntry)));
										KeyEntry* keyEntry = (KeyEntry*)tempBuffer;
										for(int keyIndex=0;keyIndex<dim;keyIndex++)
										{
											ChannelEntry newEntry(keyEntry[keyIndex].time, keyEntry[keyIndex].value);
											currentChannel->AddKey(newEntry);			// Add a new entry to this channel
										}
									}
										break;
									default:
										AssertMsg(0,"unknown chunk <" << channelChunkIter->GetChunkID() << ">, " << 'RTS' << " " << pathChunkIter->GetChunkID().ID());
										break;
								}
								delete channelChunkIter;
							}
						}
							break;
						default:
							AssertMsg(0,"unknown chunk <" << pathChunkIter->GetChunkID() << ">, " << 'RTS' << " " << pathChunkIter->GetChunkID().ID());
							break;
					}
				delete pathChunkIter;

				}
				QPath tempPath(position, rotation);
				tempPath.SetXChannel(channelArray[XPOS]);
				tempPath.SetYChannel(channelArray[YPOS]);
				tempPath.SetZChannel(channelArray[ZPOS]);
				tempPath.SetAChannel(channelArray[EULERA]);
				tempPath.SetBChannel(channelArray[EULERB]);
				tempPath.SetCChannel(channelArray[EULERC]);
			
				pathIndex = AddPath(tempPath);
				   	   
	//			DBSTREAM3( cdebug << "At time=0, position = " << followPosition << endl <<
	//		                     	"           rotation = " << followRotation.x << ", " << followRotation.y << ", " << followRotation.z << ", " << followRotation.w << endl; )
			}
				break;

			default:
				AssertMsg(0,"unknown chunk <" << objChunkIter->GetChunkID() << ">, " << '23I' << " " << objChunkIter->GetChunkID().ID());
				break;
		}
		delete objChunkIter;
	//chunkIter->ReadBytes(&tempVertex,sizeof(Vertex3DOnDisk));

	}
	if (className == string("disabled"))
	{
		cdebug << "Not creating object <" << thisObjectName << ">; it is of class \"Disabled\"" << endl; 
		DBSTREAM3( cdebug << "Not creating object <" << thisObjectName << ">; it is of class \"Disabled\"" << endl; )
		return;
	}

#pragma message ("KTS " __FILE__ ": deal with rotations and scaling")
#pragma message ("KTS " __FILE__ ": deal with shortcuts")
#if 0
//	int typeIndex = 0;
//	int pathIndex = -1;
//	char* oadData = NULL;
//	size_t oadDataSize = 0;

	// WARNING!  The Quat(Matrix3) constructor SUCKS.
	Quat rotQuat;
	Euler objectOrientation;
	Point3 checkTrans, checkScale;	// Dummies for DecomposeMatrix()
	DecomposeMatrix(nodeTM, checkTrans, rotQuat, checkScale);
	QuatToEuler(rotQuat, &objectOrientation.a);

	DBSTREAM3( cdebug << "Object has rotations:" << endl; )
	DBSTREAM3( cdebug << "     X: " << objectOrientation.a << endl; )
	DBSTREAM3( cdebug << "     Y: " << objectOrientation.b << endl; )
	DBSTREAM3( cdebug << "     Z: " << objectOrientation.c << endl << endl; )

	nodeTM.NoTrans();
	objOffsetPos = VectorTransform( nodeTM, objOffsetPos );

	QObjectAttributeData newobjOAD;
	bool bShortcut;
	INode* pNode = thisNode;
	do
	{
		assert( pNode );

		AppDataChunk* appDataChunk = pNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
		int xDataSize = -1;

		className = string( appDataChunk ? (char*)(appDataChunk->data) : "disabled" );

		if (className == string("disabled"))
		{
			DBSTREAM3( cdebug << "Not creating object <" << thisObjectName << ">; it is of class \"Disabled\"" << endl; )
			return;
		}

		typeIndex = GetClassIndex(className);
		if ( typeIndex == -1 )
		{
			AssertMessageBox(typeIndex != -1, "Object with name <" << thisObjectName << "> has invalid Object Type of <" << className << ">");
		}

		DBSTREAM3( cdebug << "Object Type is " << typeIndex << ", which is " << objectTypes[typeIndex] << endl; )

		// now find attribute data so that the QObject will have it
		// kts added to allow objects with all defaults
		appDataChunk = pNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD );

		if ( appDataChunk )
		{
			oadDataSize = appDataChunk->length;
			assert(oadDataSize != -1);
			oadData = oadDataSize ? (char*)(appDataChunk->data) : NULL;
		}
		else
		{
			// kts added to allow objects with all defaults
			oadData = NULL;
			oadDataSize = 0;
			DBSTREAM3( cdebug << "LevelCon Warning: OAD Data not found for object <" << className << ">, using defaults from .oad file" << endl; )
		}

		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode:Parsing new OAD chunk from object <" << thisObjectName << ">" << endl; )
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: OAD from object" << endl; )
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: Applying OAD:: " << endl; )
		newobjOAD = objectOADs[typeIndex];
		bool error = newobjOAD.Apply(oadData, oadDataSize, thisObjectName);
		if(error)
			cerror << "OAD Error in object " << thisObjectName << endl;
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: OAD after apply: " << endl; )

		bShortcut = bool( newobjOAD.GetEntryByName("LEVELCONFLAG_SHORTCUT") );
		if ( bShortcut )
		{
			// Retrieve necessary replacement data
			QObjectAttributeDataEntry* oadBaseObject =
				(QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("Base Object");
			assert( oadBaseObject );

			// get ptr to instance of BaseObject
			pNode = gMaxInterface->GetINodeByName( oadBaseObject->GetString().c_str() );
			AssertMessageBox( pNode, "invalid shortcut base object" );
		}
	}
	while ( bShortcut );
#endif

	// path are now handled elsewhere

	// find node name data so we can get the model name
	DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: look for mesh name: " << endl; )
	bool isDebugObject = false;

	// assert(!objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_NOMESH));

	DBSTREAM3( cdebug << "checking for mesh name in object " << thisObjectName << endl; )
	QObjectAttributeDataEntry* oadMeshName = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("Mesh Name");

	if (oadMeshName && oadMeshName->GetString().length())
		;
	else
	{
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: Not Adding Object Mesh of type <" << typeIndex << "> since its OAD contains a LEVELCONFLAG_NOMESH field" << endl; )
		isDebugObject = true;
	}

	DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: colbox before collision for object " << thisObjectName << ":" << endl; )
	DBSTREAM3( cdebug << collision << endl; )

	// kts temp until velocity can handle thin floors
#define	abs(val)	(((val) < 0) ? -(val) : (val))
	if( ( abs(collision.GetMin().x - collision.GetMax().x)) < 0.25 ) 
	{
		Point3 temp = collision.GetMin();
		temp.x = collision.GetMax().x - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max x match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().x) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().x) << ">" << endl; )
	}
	if( abs(collision.GetMin().y - collision.GetMax().y) < 0.25 )
	{
		Point3 temp = collision.GetMin();
		temp.y = collision.GetMax().y - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max y match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().y) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().y) << ">" << endl; )
	}
	if( abs(collision.GetMin().z - collision.GetMax().z) < 0.25 )
	{
		Point3 temp = collision.GetMin();
		temp.z = collision.GetMax().z - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max z match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().z) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().z) << ">" << endl; )
	}

	DBSTREAM3( cdebug << "Coarse Bounding box:" << endl << collision << endl; )

#if 1
	// create oadflags
	int32 oadFlags = newobjOAD.GetOADFlags();
	DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: oadFlags = <" << hex << oadFlags << ">" << dec << endl; )

	// check for presence of a LEVELCONFLAG_NOINSTANCES entry in the oad
	if(!objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_NOINSTANCES))
	{
		//DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: Calling QObject constructor: oadData <" << !!oadData << ">, oadDataSize <" << oadDataSize << ">" << endl; )

		// If this is a light object, we need to patch some OAD entries...
#if 0		                            // this happens in the iff exporter
		if (objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_EXTRACTLIGHT))
		{
			LightObject *obj = (LightObject*)(thisNode->EvalWorldState(theTime).obj);
			AssertMessageBox(obj->SuperClassID()==LIGHT_CLASS_ID, "Object <" << thisObjectName << ">:" << endl << "This doesn't look like a light to me!");
			Interval valid;
			LightState ls;
			AssertMessageBox( obj->EvalLightState(theTime, valid, &ls)==REF_SUCCEED, "Light <" << thisObjectName << ">" << endl << "This object's data is corrupt!" );

			QObjectAttributeDataEntry* tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightType");
			assert(tempEntry);
			ls.tm.NoTrans();
			ls.tm.NoScale();
			Point3 lightVector = VectorTransform(ls.tm, Point3(0,0,-1));
			switch (ls.type)
			{
				case OMNI_LIGHT:
					tempEntry->SetDef( AMBIENT_LIGHT );
					break;

				case DIR_LIGHT:
					tempEntry->SetDef( DIRECTIONAL_LIGHT );
					tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightX");
					assert(tempEntry);
					tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.x) );
					tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightY");
					assert(tempEntry);
					tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.y) );
					tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightZ");
					assert(tempEntry);
					tempEntry->SetDef( WF_FLOAT_TO_SCALAR(lightVector.z) );
					break;

				default:
					AssertMessageBox(0, "Light <" << thisObjectName << ">" << endl << "Must be either OMNI or DIRECTIONAL" << endl << "(Current value = " << (short)(ls.type) << ")");
					break;
			}

			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightRed");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.r) );
			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightGreen");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.g) );
			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("lightBlue");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(ls.color.b) );
		}
#endif

		// slops are now handled elsewhere 

		assert(pathIndex < (int)paths.size());

		objects.push_back
		(
			new QObject
			(
				thisObjectName,
				typeIndex,
				Point3( objectPosition.x, objectPosition.y, objectPosition.z ),
				Point3( 1.0, 1.0, 1.0 ),
				objectOrientation,
				collision,
				oadFlags,
				pathIndex,
				newobjOAD
			)
		);

		DBSTREAM3( cdebug << "Push back succeeded." << endl; )


	}
	DBSTREAM3( else )
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: Not Adding Object instance of type <" << typeIndex << "> since its OAD contains a LEVELCONFLAG_NOINSTANCES field" << endl; )
#endif
}

//============================================================================

int16
QLevel::NewChannel(void)
{
	channels.push_back( *(new Channel) );
	return channels.size() - 1;
}

//============================================================================

void
QLevel::CreateQObjectList( /*SceneEnumProc* theSceneEnum*/ )
{
#pragma message ("KTS " __FILE__ ": finish this")
	assert(0);
#if 0
	DBSTREAM1( cprogress << "Creating QObject list from the current Scene." << endl; )

	Interface* ip = GetCOREInterface();
	assert( ip );

	TimeValue time = 0;		// theSceneEnum->time

	INode* _pRoot = ip->GetRootNode();
	assert( _pRoot );
	for ( int i=0; i<_pRoot->NumberOfChildren(); ++i )
	{
		INode* pObject = _pRoot->GetChildNode( i );
		assert( pObject );

			Mesh* mesh = ((GeomObject*)os.obj)->GetRenderMesh( time, pObject, nullView, needDel );
				CreateQObjectFromSceneNode( mesh, pObject, time );
	}
#endif
}

//============================================================================

void
QLevel::LoadLevelIFFFile(const char* iffFileName)
{
	DBSTREAM3( cprogress << "Loading level iff file" << endl; )

	binistream input(iffFileName);
	AssertMessageBox(input.good(), "Problem reading file " << iffFileName << ", Probably not found");

#pragma message ("KTS " __FILE__ ": update iff I/O to be endian independant")
    IFFChunkIter lvlIter(input);
	assert(lvlIter.GetChunkID().ID() == '\0LVL');

	while(lvlIter.BytesLeft() > 0)
	{
		IFFChunkIter* chunkIter = lvlIter.GetChunkIter(*levelconRM);
//		ciffread << "chunkid = " << chunkIter->GetChunkID() << endl;
		switch(chunkIter->GetChunkID().ID())
		{
			case '\0JBO':
			{
				CreateQObjectFromChunk(*chunkIter);
				assert(chunkIter->BytesLeft() == 0);
				break;
			}

			default:
				AssertMsg(0,"chunk of " << chunkIter->GetChunkID() << " not recognized" << endl);
				break;
		}
		MEMORY_DELETE(*levelconRM,chunkIter,IFFChunkIter);
		//delete chunkIter;
//		meshIter.NextChunk();
//		ciffread << "meshIter bytes left = " << meshIter.BytesLeft() << endl;
	}
}

//============================================================================
