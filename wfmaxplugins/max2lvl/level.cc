//============================================================================
// level.cc:
// Copyright 1997,1998 Recombinant Limited.  All Rights Reserved.
//============================================================================

#include "global.hpp"
#include "lvlexp.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <math.h>

#include "levelcon.hpp"
#include "pigtool.h"
#include "model.hpp"
#include "object.hpp"
#include "level.hpp"
#include "oad.hpp"
#include "colbox.hpp"
#include "room.hpp"
#include "3d.hpp"
#include "asset.hpp"
#include "path.hpp"

#include <stl/pair.h>
#include <stl/algo.h>

#pragma pack(1)
extern "C" {
#include <oas/oad.h>			// from the velocity project
};
#pragma pack()

#pragma pack(1)

#pragma pack()

#include <game/levelcon.h>

#include "hdump.hpp"
#include <stl/bstring.h>

#include "../lib/wf_id.hp"

//============================================================================

extern Interface* gMaxInterface;
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
	DBSTREAM3( cprogress << "Loading .LC file <" << lcFileName << ">" << endl; )
	LoadLCFile(lcFileName);
	LoadAssetFile(lcFileName);	// used to find assets.txt files
	LoadOADFiles(lcFileName);	// used to find the .oad files
	CreateQObjectList();		// This replaces Load3DStudio()
	DBSTREAM3( cprogress << "Updating references, etc." << endl; )
}

//============================================================================

QLevel::~QLevel()
{
	objects.erase(objects.begin(), objects.end());
	channels.erase(channels.begin(), channels.end());
}

//============================================================================
//-----------------------------------------------------------------------------
// load a single 3DS MAX scene node with its .oad data, and add it to the various lists

#define OAD_CHUNK_NAME "Cave Logic Studios Object Attribute Editor v0.3"
const char CLASS_CHUNK_NAME[] = "Cave Logic Studios Class Object Editor";

void
QLevel::CreateQObjectFromSceneNode(Mesh* thisMesh, INode* thisNode, TimeValue theTime)
{
	int typeIndex = 0;
	int pathIndex = -1;
	char* oadData = NULL;
	size_t oadDataSize = 0;
	string className;


	char* thisObjectName = thisNode->GetName();

	float masterScale = float(GetMasterScale(UNITS_METERS));
    AssertMessageBox( masterScale==1.0, "Units MUST be set to METERS." << endl <<
                                        "(Go to the File menu, choose Preferences," << endl <<
                                        "choose the General tab, and set 1 unit = 1 meter)" );
	Matrix3 objTM = thisNode->GetObjectTM(theTime);
	Point3 objOffsetPos = thisNode->GetObjOffsetPos();
	Matrix3 nodeTM = thisNode->GetNodeTM(theTime);
	Point3 location = nodeTM.GetTrans();

	// WARNING!  The Quat(Matrix3) constructor SUCKS.
	Quat rotQuat;
	Euler rotEuler;
	Point3 checkTrans, checkScale;	// Dummies for DecomposeMatrix()
	DecomposeMatrix(nodeTM, checkTrans, rotQuat, checkScale);
	QuatToEuler(rotQuat, &rotEuler.a);

	DBSTREAM3( cdebug << "Object has rotations:" << endl; )
	DBSTREAM3( cdebug << "     X: " << rotEuler.a << endl; )
	DBSTREAM3( cdebug << "     Y: " << rotEuler.b << endl; )
	DBSTREAM3( cdebug << "     Z: " << rotEuler.c << endl << endl; )

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



	//===== PATH CODE STARTS HERE =====//


	// deal with object paths
	DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: looking for keyframe data" << endl; )

	Control* posControl;
	Control* rotControl;
	posControl = thisNode->GetTMController()->GetPositionController();
	rotControl = thisNode->GetTMController()->GetRotationController();
	char thisPosKey[100], thisRotKey[100];	// Big enough for ANY frigging subclass of IKey

	int numPosKeys(0), numRotKeys(0);
	if (posControl)
		numPosKeys = posControl->NumKeys();
	if (rotControl)
		numRotKeys = rotControl->NumKeys();

	AssertMessageBox(numPosKeys != NOT_KEYFRAMEABLE, "Object <" << thisObjectName << "> says its position is NOT_KEYFRAMEABLE");
	AssertMessageBox(numRotKeys != NOT_KEYFRAMEABLE, "Object <" << thisObjectName << "> says its rotation is NOT_KEYFRAMEABLE");

	if ((numPosKeys > 0) || (numRotKeys > 0))
	{
		DBSTREAM3( cdebug << "Found it:  numPosKeys = " << numPosKeys << ", numRotKeys = " << numRotKeys << endl; )
		IKeyControl* posInterface = GetKeyControlInterface(posControl);
		assert(posInterface);

		Point3 tempPosition;
		Quat tempRotation;
		int ticks = GetTicksPerFrame();

		// If this is a relative path, look up the follow object
		QObjectAttributeDataEntry* oadFollowObject =
			(QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("Object To Follow");
		assert( oadFollowObject );

		// get ptr to instance of FollowObject
		INode* pFollowNode = gMaxInterface->GetINodeByName( oadFollowObject->GetString().c_str() );
		Point3 followPosition(0,0,0);
		Quat followRotation;
		followRotation.Identity();

		// If this path is following another object, get the follow object's initial position and
		// rotation and store them in the Path; otherwise, they'll remain zero.
		if (pFollowNode)
		{
			Matrix3 followObjTM = pFollowNode->GetNodeTM(0);
			followPosition = followObjTM.GetTrans();
			DecomposeMatrix(followObjTM, checkTrans, followRotation, checkScale);
		}

		DBSTREAM3( cdebug << "At time=0, position = " << followPosition << endl <<
		                     "           rotation = " << followRotation.x << ", " << followRotation.y << ", " << followRotation.z << ", " << followRotation.w << endl; )
		QPath tempPath(followPosition, followRotation);
		TimeValue keyTime;

		// If there are no position keys, add one to provide initial position
		if (numPosKeys == 0)
			tempPath.AddPositionKey(0, location);

		for (int keyIndex = 0; keyIndex < numPosKeys; keyIndex++)
		{
			keyTime = posControl->GetKeyTime(keyIndex);
			posInterface->GetKey(keyIndex, (IKey*)thisPosKey);
			switch( posControl->ClassID().PartA() )
			{
				case LININTERP_POSITION_CLASS_ID:
					tempPosition = ((ILinPoint3Key*)thisPosKey)->val;
					break;
				case TCBINTERP_POSITION_CLASS_ID:
					tempPosition = ((ITCBPoint3Key*)thisPosKey)->val;
					break;
				case HYBRIDINTERP_POSITION_CLASS_ID:
					tempPosition = ((IBezPoint3Key*)thisPosKey)->val;
					break;
				default:
					AssertMessageBox(0, "Object <" << thisObjectName << ">" << endl << "Path contains unknown key types");
					break;
			}
			tempPath.AddPositionKey(keyTime, tempPosition);
		}

		// If there are no rotation keys, add one to provide initial rotation
		if (numRotKeys == 0)
			tempPath.AddRotationKey(0, rotQuat);
		else
		{
			IKeyControl* rotInterface = GetKeyControlInterface(rotControl);
			assert(rotInterface);

			Quat rotationAccumulator;	// PRS stores rotation as deltas
			rotationAccumulator.Identity();

			DBSTREAM3( cdebug << "Processing " << numRotKeys << " rotation key(s)...." << endl; )
			for (keyIndex = 0; keyIndex < numRotKeys; keyIndex++)
			{
				//keyTime = rotControl->GetKeyTime(keyIndex) / ticks;
				keyTime = rotControl->GetKeyTime(keyIndex);
				rotInterface->GetKey(keyIndex, (IKey*)thisRotKey);
				switch( rotControl->ClassID().PartA() )
				{
					case LININTERP_ROTATION_CLASS_ID:
						tempRotation = ((ILinRotKey*)thisRotKey)->val;
						break;
					case TCBINTERP_ROTATION_CLASS_ID:
						tempRotation = Quat(((ITCBRotKey*)thisRotKey)->val);
						break;
					case HYBRIDINTERP_ROTATION_CLASS_ID:
						tempRotation = ((IBezQuatKey*)thisRotKey)->val;
						break;
					default:
						AssertMessageBox(0, "Object <" << thisObjectName << ">" << endl << "Path contains unknown rotation key type.");
						break;
				}

				rotationAccumulator *= tempRotation;

				DBSTREAM3(
					Point3 axis;
					float ang;
					AngAxisFromQ( tempRotation, &ang, axis );
					cdebug << endl << "KEY #" << keyIndex << endl;
					cdebug << "At time = " << keyTime << ", key rotation (delta): " << ang << " about " << axis;
					AngAxisFromQ( rotationAccumulator, &ang, axis );
					cdebug << "    Accumulated rotation (absolute): " << ang << " about " << axis;
				)

				tempPath.AddRotationKey(keyTime, rotationAccumulator);
			}
		}

		tempPath.SizeOfOnDisk();

		vector<QPath>::iterator where = find(paths.begin(),paths.end(),tempPath);
		if(where == paths.end())
		{
			paths.push_back(tempPath);					// create new path
			pathIndex = paths.size() - 1;
		}
		else
		{
			pathIndex = where  - paths.begin();		// use existing path
		}
	}



	//===== PATH CODE ENDS HERE =====//



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

	// create collision box
	QColBox collision;
	objTM = thisNode->GetObjTMAfterWSM(theTime);
	objTM.NoTrans();

	if (thisMesh)
	{
		collision.Bound(*thisMesh, objTM, objOffsetPos);

		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: colbox before collision for object " << thisObjectName << ":" << endl; )
		DBSTREAM3( cdebug << collision << endl; )
	}

	// kts temp until velocity can handle thin floors
	if( abs(collision.GetMin().x - collision.GetMax().x) < 0.25 )
	{
		Point3 temp = collision.GetMin();
		temp.x = temp.x - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max x match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().x) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().x) << ">" << endl; )
	}
	if( abs(collision.GetMin().y - collision.GetMax().y) < 0.25 )
	{
		Point3 temp = collision.GetMin();
		temp.y = temp.y - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max y match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().y) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().y) << ">" << endl; )
	}
	if( abs(collision.GetMin().z - collision.GetMax().z) < 0.25 )
	{
		Point3 temp = collision.GetMin();
		temp.z = temp.z - 0.25;
		collision.SetMin(temp);
		DBSTREAM3( cdebug << "Min & Max z match at <" << WF_SCALAR_TO_FLOAT(collision.GetMax().z) << ">, moved min to <" << WF_SCALAR_TO_FLOAT(collision.GetMin().z) << ">" << endl; )
	}

	DBSTREAM3( cdebug << "Coarse Bounding box:" << endl << collision << endl; )

	// create oadflags
	int32 oadFlags = newobjOAD.GetOADFlags();
	DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: oadFlags = <" << hex << oadFlags << ">" << dec << endl; )

	// check for presence of a LEVELCONFLAG_NOINSTANCES entry in the oad
	if(!objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_NOINSTANCES))
	{
		DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: Calling QObject constructor: oadData <" << !!oadData << ">, oadDataSize <" << oadDataSize << ">" << endl; )

		// If this is a light object, we need to patch some OAD entries...
		if (objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_EXTRACTLIGHT))
		{
			DBSTREAM3( cdebug << "QLevel::CreateQObjectFromSceneNode: EXTRACTLIGHT found for object " << thisObjectName << endl; )

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

		// If this object was created using the Slope primitive, we need to patch some OAD entries...
		if ( thisNode->EvalWorldState(theTime).obj->ClassID() == Slope_ClassID )
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
			QObjectAttributeDataEntry* tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("slopeA");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(slopeNormal.x) );
			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("slopeB");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(slopeNormal.y) );
			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("slopeC");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(slopeNormal.z) );
			tempEntry = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("slopeD");
			assert(tempEntry);
			tempEntry->SetDef( WF_FLOAT_TO_SCALAR(slopeD) );
		}
		// Done with sloped surface code

		assert(pathIndex < (int)paths.size());

		objects.push_back
		(
			new QObject
			(
				thisObjectName,
				typeIndex,
				Point3( location.x, location.y, location.z ),
				Point3( 1.0, 1.0, 1.0 ),
				rotEuler,
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

}

//============================================================================

int16
QLevel::NewChannel(void)
{
	channels.push_back( *(new Channel) );
	return channels.size() - 1;
}

//============================================================================
