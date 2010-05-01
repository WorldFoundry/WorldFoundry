// max2lev.cpp	World Foundry
// Copyright 1998,1999,2000 World Foundry Group.  All Rights Reserved.

#include "global.hpp"
#include "max2lev.hpp"
#include "max2lev.h"	                // todo: rename max2lev.h to resource.h
#include "../lib/registry.h"
#include <oas/oad.h>
#include "../lib/iffwrite.hp"
//#include <iffwrite/fixed.hp>
#include "box.h"
//#include <stl/vector.h>
#include "../lib/wf_id.hp"
#include "../lib/loaddll.h"
//#include <lib/eval.h>
extern HINSTANCE hInstance;
#include "path.hpp"
#include <fstream.h>

BOOL CALLBACK OutputOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

max2ifflvlOptions theOptions;
double theTime = 0.0;			        // should propably be in theOptions

Matrix3 objTM;
Point3 location;
Quat rotQuat;							// WARNING!  The Quat(Matrix3) constructor SUCKS.
Point3 objOffsetPos;

void process_object_path( INode* thisNode, _IffWriter* _iff );
void process_object_oad( INode* thisNode, _IffWriter* _iff );
void process_prs( INode* thisNode, _IffWriter* _iff );
void process_bounding_box( INode* thisNode, _IffWriter* _iff );
//void process_model_geometry( INode* thisNode, _IffWriter* _iff );
//EXPORT_MESH_PROC fnMax2Iff;

//================================================================================

#if SW_DEBUGLOG
ostream* debuglog;
#endif

extern "C" bool
max2ifflvl( ostream& s, SceneEnumProc* theSceneEnum, max2ifflvlOptions options )
{
#if SW_DEBUGLOG
	debuglog = new ofstream("c:\\log");
	assert(debuglog->good());
	(*debuglog) << "max2lev log Ver " << LEVELCON_VER << endl;
#endif
	try
	{
		_IffWriter* _iff;
		if ( options.max2iffOptions.bOutputBinary )
			_iff = new IffWriterBinary( s );
		else
			_iff = new IffWriterText( s );
		assert( _iff );

		DEBUGLOG(_iff->log(*debuglog);)
		DEBUGLOG((*debuglog) << "1" << endl;)
		// TODO: exception
		assert( _iff );

		HINSTANCE hMax2IffInst = LoadMaxLibrary( MAX2IFF_PlugIn );
		assert( hMax2IffInst );
		DEBUGLOG((*debuglog) << "2" << endl;)

		//fnMax2Iff = (EXPORT_MESH_PROC)GetProcAddress( hMax2IffInst, "max2iff" );
		//assert( fnMax2Iff );

		DEBUGLOG((*debuglog) << "entering chunk LVL" << endl;)
		_iff->enterChunk( ID( "LVL" ) );

		DEBUGLOG((*debuglog) << "3" << endl;)
		for ( SceneEntry* thisEntry = theSceneEnum->head; thisEntry; thisEntry = thisEntry->next )
		{
			DEBUGLOG((*debuglog) << "4: " << endl;)
			INode* thisNode = thisEntry->node;
			assert( thisNode );

			::objTM = thisNode->GetObjectTM( theTime );

			Point3 objOffsetPos = thisNode->GetObjOffsetPos();
			Quat   objOffsetRot = thisNode->GetObjOffsetRot();

			Matrix3 nodeTM = thisNode->GetNodeTM( theTime );
			location = nodeTM.GetTrans();

			Point3 checkTrans, checkScale;	// Dummies for DecomposeMatrix()
			DecomposeMatrix( nodeTM, checkTrans, rotQuat, checkScale );
			rotQuat = rotQuat * objOffsetRot;
			//location = ::objTM.GetTrans();	// / masterScale;

			DEBUGLOG((*debuglog) << "entering OBJ " << endl;)
			_iff->enterChunk( ID( "OBJ" ) );

				DEBUGLOG((*debuglog) << "entering NAME " << endl;)
				_iff->enterChunk( ID( "NAME" ) );
					_iff->out_string( thisEntry->node->GetName() );
				DEBUGLOG((*debuglog) << "exiting NAME " << endl;)
				_iff->exitChunk();

				DEBUGLOG((*debuglog) << "process_object_path " << endl;)
				process_object_path( thisNode, _iff );
				DEBUGLOG((*debuglog) << "process_prs " << endl;)
				process_prs( thisNode, _iff );
				DEBUGLOG((*debuglog) << "process_bounding_box " << endl;)
				process_bounding_box( thisNode, _iff );
				DEBUGLOG((*debuglog) << "process_object_oad " << endl;)
				process_object_oad( thisNode, _iff );
				//process_model_geometry( thisNode, _iff );

				DEBUGLOG((*debuglog) << "exiting OBJ " << endl;)
			_iff->exitChunk();
		}
		DEBUGLOG((*debuglog) << "5:" << endl;)

		_iff->exitChunk();
		delete _iff;

		FreeLibrary( hMax2IffInst );
	}
	catch ( LVLExporterException& e)
	{
		char errorMessage[1000];
		sprintf(errorMessage,"Exception occured: %d",e.errorCode);
		AssertMessageBox(0,errorMessage);
	}

	DEBUGLOG(delete debuglog;)
	return true;
}

//--------------------------------------------------------------------------------

extern "C" bool
max2ifflvl_Query( ostream& s, SceneEnumProc* theSceneEnum )
{
	Interface* ip = GetCOREInterface();
	assert( ip );

	HWND _hwndMax = ip->GetMAXHWnd();
	assert( _hwndMax );

	assert( hInstance );
	if ( !DialogBox( hInstance, MAKEINTRESOURCE( IDD_OUTPUT_OPTIONS ), _hwndMax, OutputOptionsDlgProc ) ) 
		return false;

	max2ifflvl( s, theSceneEnum, theOptions );

	return true;
}

//==============================================================================

void
process_object_path( INode* thisNode, _IffWriter* _iff )
{
	DEBUGLOG((*debuglog) << "process_object_path:  " << endl;)

	Control* posControl;
	Control* rotControl;
	DEBUGLOG((*debuglog) << "process_object_path:  get controllers" << endl;)
	posControl = thisNode->GetTMController()->GetPositionController();
//	assert(posControl);
	rotControl = thisNode->GetTMController()->GetRotationController();
//	assert(rotControl);
	DEBUGLOG((*debuglog) << "process_object_path:  get controllers done" << endl;)
	char thisPosKey[100], thisRotKey[100];	// Big enough for ANY frigging subclass of IKey

	DEBUGLOG((*debuglog) << "process_object_path:  get posnumkeys" << endl;)
	int numPosKeys = posControl?posControl->NumKeys():0;
	DEBUGLOG((*debuglog) << "process_object_path:  get rotnumkeys" << endl;)
	int numRotKeys = rotControl?rotControl->NumKeys():0;

	DEBUGLOG((*debuglog) << "process_object_path:  do assertions" << endl;)
	assert( numPosKeys != NOT_KEYFRAMEABLE );
	assert( numRotKeys != NOT_KEYFRAMEABLE );
//	AssertMessageBox(numPosKeys != NOT_KEYFRAMEABLE, "Object <" << thisNode->GetName() << "> says its position is NOT_KEYFRAMEABLE");
//	AssertMessageBox(numRotKeys != NOT_KEYFRAMEABLE, "Object <" << thisNode->GetName() << "> says its rotation is NOT_KEYFRAMEABLE");

	DEBUGLOG((*debuglog) << "process_object_path: if start " << endl;)
	if ( (numPosKeys > 0) || (numRotKeys > 0) )
	{
		IKeyControl* posInterface = GetKeyControlInterface(posControl);
		assert(posInterface);

		Point3 tempPosition;
		Quat tempRotation;
		int ticks = GetTicksPerFrame();

#if 0
		// If this is a relative path, look up the follow object
		QObjectAttributeDataEntry* oadFollowObject =
			(QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("Object To Follow");
		assert( oadFollowObject );

		// get ptr to instance of FollowObject
		INode* pFollowNode = gMaxInterface->GetINodeByName( oadFollowObject->GetString().c_str() );
#endif
		Point3 followPosition(0,0,0);
		Quat followRotation;
		followRotation.Identity();

#if 0
		// If this path is following another object, get the follow object's initial position and
		// rotation and store them in the Path; otherwise, they'll remain zero.
		if (pFollowNode)
		{
			Matrix3 followObjTM = pFollowNode->GetNodeTM(0);
			followPosition = followObjTM.GetTrans();
			DecomposeMatrix(followObjTM, checkTrans, followRotation, checkScale);
		}
#endif

		Path tempPath( followPosition, followRotation );
		TimeValue keyTime;

		// If there are no position keys, add one to provide initial position
		if ( numPosKeys == 0 )
			tempPath.AddPositionKey( 0, location );

		DEBUGLOG((*debuglog) << "process_object_path:  time loop start" << endl;)
		for ( int idxKey = 0; idxKey < numPosKeys; ++idxKey )
		{
			keyTime = posControl->GetKeyTime( idxKey );
			posInterface->GetKey(idxKey, (IKey*)thisPosKey);


			switch( posControl->ClassID().PartA() )
			{
				case LININTERP_POSITION_CLASS_ID:
					tempPosition = ((ILinPoint3Key*)thisPosKey)->val;	// / masterScale;
					break;
				case TCBINTERP_POSITION_CLASS_ID:
					tempPosition = ((ITCBPoint3Key*)thisPosKey)->val;	// / masterScale;
					break;
				case HYBRIDINTERP_POSITION_CLASS_ID:
					tempPosition = ((IBezPoint3Key*)thisPosKey)->val;	// / masterScale;
					break;
				default:
					assert( 0 );
					//AssertMessageBox(0, "Object <" << thisNode->GetName() << ">" << endl << "Path contains unknown key types");
					break;
			}
			DEBUGLOG((*debuglog) << "process_object_path:  processing key " << idxKey << ", keyTime = " << keyTime << ", position = " << tempPosition.x << "," << tempPosition.y << "," << tempPosition.z << endl;)
			tempPath.AddPositionKey(keyTime, tempPosition);
		}
		DEBUGLOG((*debuglog) << "process_object_path:  time loop done" << endl;)

		// If there are no rotation keys, add one to provide initial rotation
		if (numRotKeys == 0)
			tempPath.AddRotationKey(0, rotQuat);
		else
		{
			assert(rotControl);
			IKeyControl* rotInterface = GetKeyControlInterface(rotControl);
			assert(rotInterface);

			Quat rotationAccumulator;	// PRS stores rotation as deltas
			rotationAccumulator.Identity();

			for (idxKey = 0; idxKey < numRotKeys; idxKey++)
			{
				//keyTime = rotControl->GetKeyTime(idxKey) / ticks;
				keyTime = rotControl->GetKeyTime(idxKey);
				rotInterface->GetKey(idxKey, (IKey*)thisRotKey);
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
						assert( 0 );
						//AssertMessageBox(0, "Object <" << thisNode->GetName() << ">" << endl << "Path contains unknown rotation key type.");
						break;
				}

				rotationAccumulator *= tempRotation;
				tempPath.AddRotationKey(keyTime, rotationAccumulator);
			}
		}

		*_iff << tempPath;
#if 0
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
#endif
	}
	DEBUGLOG((*debuglog) << "process_object_path: done" << endl;)
}

typedef struct
{
	float a;
	float b;
	float c;
} Euler;

void
process_prs( INode* thisNode, _IffWriter* _iff )
{
	assert( thisNode );
	assert( _iff );

	// POSITION
	_iff->enterChunk( ID( "VEC3" ) );
		_iff->enterChunk( ID( "NAME" ) );
			_iff->out_string( "Position" );
		_iff->exitChunk();
		_iff->enterChunk( ID( "DATA" ) );
			*_iff << Fixed( theOptions.max2iffOptions.sizeReal, location[ 0 ] );
			*_iff << Fixed( theOptions.max2iffOptions.sizeReal, location[ 1 ] );
			*_iff << Fixed( theOptions.max2iffOptions.sizeReal, location[ 2 ] );
			*_iff << Comment( "x,y,z" );
		_iff->exitChunk();
	_iff->exitChunk();

#if 0 							        // kts 12/22/99 2:44PM switched to output eulers instead of quaternions
	// ROTATION
	_iff->enterChunk( ID( "QUAT" ) );
		_iff->enterChunk( ID( "NAME" ) );
			*_iff << "Orientation";
		_iff->exitChunk();
		_iff->enterChunk( ID( "DATA" ) );
			*_iff << Fixed( theOptions.sizeQuaternion, rotQuat.x );
			*_iff << Fixed( theOptions.sizeQuaternion, rotQuat.y );
			*_iff << Fixed( theOptions.sizeQuaternion, rotQuat.z );
			*_iff << Fixed( theOptions.sizeQuaternion, rotQuat.w );
			*_iff << Comment( "x,y,z,w" );
		_iff->exitChunk();
	_iff->exitChunk();
#else
	_iff->enterChunk( ID( "EULR" ) );
		_iff->enterChunk( ID( "NAME" ) );
			*_iff << "Orientation";
		_iff->exitChunk();
		_iff->enterChunk( ID( "DATA" ) );
			Euler rotEuler;
			QuatToEuler(rotQuat, &rotEuler.a);
			*_iff << Fixed( fp_1_15_16, rotEuler.a );
			*_iff << Fixed( fp_1_15_16, rotEuler.b );
			*_iff << Fixed( fp_1_15_16, rotEuler.c );
			*_iff << Comment( "a,b,c" );
		_iff->exitChunk();
	_iff->exitChunk();
#endif
}


void
process_bounding_box( INode* thisNode, _IffWriter* _iff )
{
	assert( thisNode );
	assert( _iff );

	// create collision box
	Box collision;
	ObjectState os = thisNode->EvalWorldState( 0 );

	// Make sure this is a geometry object (not a camera)
	if ( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID )
	{
		BOOL needDel;
		NullView nullView;
		TimeValue time = 0;
		Mesh* mesh = ((GeomObject*)os.obj)->GetRenderMesh( time, thisNode, nullView, needDel );
		if ( mesh )
		{
			Point3 objOffsetPos = thisNode->GetObjOffsetPos();
			Matrix3 nodeTM = thisNode->GetNodeTM(theTime);
			nodeTM.NoTrans();
			objOffsetPos = VectorTransform( nodeTM, objOffsetPos );

			Matrix3 objTM = thisNode->GetObjectTM(time);
			objTM.NoTrans();

			collision.Bound( *mesh, objTM);
			collision += objOffsetPos;
			// kts temp until velocity can handle thin floors
			Point3 min = collision.GetMin();
			if ( collision.GetMin().x == collision.GetMax().x )
				min.x -= 0.25;
			if ( collision.GetMin().y == collision.GetMax().y )
				min.y -= 0.25;
			if ( collision.GetMin().z == collision.GetMax().z )
				min.z -= 0.25;
			collision.SetMin( min );

			_iff->enterChunk( ID( "BOX3" ) );
				_iff->enterChunk( ID( "NAME" ) );
					_iff->out_string( "Global Bounding Box" );
				_iff->exitChunk();

				_iff->enterChunk( ID( "DATA" ) );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMin().x );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMin().y );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMin().z );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMax().x );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMax().y );
					*_iff << Fixed( theOptions.max2iffOptions.sizeReal, collision.GetMax().z );
					*_iff << Comment( "min(x,y,z)-max(x,y,z)" );;
				_iff->exitChunk();
			_iff->exitChunk();

		}
	}
}

#if 0
void
process_model_geometry( INode* thisNode, _IffWriter* _iff )
{
	assert( thisNode );
	assert( _iff );

	// Creates a 'MODL' subchunk
	assert( fnMax2Iff );
	fnMax2Iff( thisNode, _iff, &( theOptions.max2iffOptions ) );
}
#endif

