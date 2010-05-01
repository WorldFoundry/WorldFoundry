///////////////////////////////////////////////////////////////////////////////
//	iffconv.cpp	VRMLConversion class from 3ds2vrml, modified for use 	     //
//					with 3DS MAX.											 //
//																			 //
//	Created 01/03/97 by Phil Torre											 //
//  Updated ??? by William B. Norris IV                                      //
//	Updated material export 9/10/97 18:44 by Kevin T. Seghetti I             //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "global.hpp"

#include <decomp.h>
#include <modstack.h>
#include <istdplug.h>
#include <stdmat.h>
#include "max2iff.hpp"
#include "iffconv.hpp"
extern HINSTANCE hInstance;
#include "max2iff.h"
#include <registry.h>
#define PSX_RGB(r,g,b) (((r&0xFF)<<16)|((g&0xFF)<<8)|(b&0xFF))

//============================================================================

void
IffConversion::writeGeometry()
{
}


void
IffConversion::writeMaterials()
{
}


//=============================================================================
// bits controlling what type of material this is,
// set up so that 0 is the simplest default: flat shaded color
// WARNING: keep this in sync with pigs2/gfx/material.hp

namespace material {
	enum
	{
		FLAT_SHADED = 0,
		GOURAUD_SHADED = 1,

		SOLID_COLOR = 0,
		TEXTURE_MAPPED = 2,

		LIGHTING_LIT = 0,
		LIGHTING_PRELIT = 4,

		SINGLE_SIDED = 0,
		DOUBLE_SIDED = 8,
	};
}

//=============================================================================

void
IffConversion::writeMeshFrame( int _theTime, _IffWriter* iff )
{
	assert( _pNode );
	ObjectState os = _pNode->EvalWorldState( _theTime );
	assert( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID );

	BOOL bNeedDel;
	NullView nullView;
	mesh = ((GeomObject*)os.obj)->GetRenderMesh( _theTime, _pNode, nullView, bNeedDel );
	assert( mesh );

	Mtl* inodeMtl = _pNode->GetMtl();
	if ( !inodeMtl )
		_pNode->SetMtl( NewDefaultStdMat() );		// This node has no material, so make one up

	int nmats = inodeMtl ? inodeMtl->NumSubMtls() : 0;
	if ( nmats == 0 )
		nmats = 1;	// This is a single-material object (no submaterials)

	iff->enterChunk( ID( "VRTX" ) );
	// Read PointList and dump a Coordinate3 node

	char tempArray[200];
	strstream temp(tempArray,200,ios::out);

	temp << "animataion at time " << float(_theTime)/float(TIME_TICKSPERSEC);
	temp << ", Scale multiplier = " << _options.geometryScale << '\0';
	*iff << Comment( tempArray );
	*iff << Comment( " u,v,color,x,y,z" );


	// This builds the offset between where the pivot was originally and where it is now

	Matrix3 objOffsetMatrix(1);
	Point3 objOffsetPos = _pNode->GetObjOffsetPos();
	objOffsetMatrix.PreTranslate(objOffsetPos);
	Quat objOffsetRot = _pNode->GetObjOffsetRot();
	PreRotateMatrix(objOffsetMatrix, objOffsetRot);
	ScaleValue objOffsetScale = _pNode->GetObjOffsetScale();
	ApplyScaling(objOffsetMatrix, objOffsetScale);

	// If this object is a group member, we need to transform from object local space to
	// group local space...
	Matrix3 groupOffsetMatrix(1);
	Point3 groupOffsetPos(0,0,0);

	// ...and we need to do it recursively, in case those clever artists start nesting groups!
	INode* innerNode = _pNode;

	while( innerNode->IsGroupMember() )
	{
		char* innerName = innerNode->GetName();
		INode* outerNode = innerNode->GetParentNode();
		AssertMessageBox( outerNode, "Object <" << innerNode->GetName() << "> thinks it is a group member," << endl <<
									 "but doesn't know who its parent is." );
		Matrix3 outerOffsetMatrix(1);
		Point3 outerOffsetPos = outerNode->GetNodeTM( _theTime ).GetTrans();
		Point3 innerPivotPos = innerNode->GetNodeTM( _theTime ).GetTrans();
		outerOffsetPos = innerPivotPos - outerOffsetPos;
		Quat outerOffsetRot = outerNode->GetObjOffsetRot();
		PreRotateMatrix(outerOffsetMatrix, outerOffsetRot);
		outerOffsetPos = VectorTransform( outerOffsetMatrix, outerOffsetPos );
		outerOffsetMatrix.Translate(outerOffsetPos);

		ScaleValue outerOffsetScale = outerNode->GetObjOffsetScale();
		ApplyScaling(outerOffsetMatrix, outerOffsetScale);

		// Accumulate outerOffsetPos and outerOffsetMatrix into groupOffsetPos and groupOffsetMatrix
		groupOffsetMatrix = groupOffsetMatrix * outerOffsetMatrix;
		groupOffsetPos += outerOffsetPos;

		innerNode = outerNode;	// Follow the tree upwards 'til we hit an orphan group head
	}


	for( int pointListIdx=0; pointListIdx < _pointList.size(); ++pointListIdx )
	{
		double u = 0.0;
		double v = 0.0;
		if ( mesh->getNumTVerts() > 0 )
		{
			UVVert thisUVVert = mesh->tVerts[_pointList[pointListIdx].uvIdx];
			u = thisUVVert.x;
			v = 1.0 - thisUVVert.y;		// kts invert 11/14/97 14:26
		}

		*iff << Fixed( _options.sizeReal, u );
		*iff << Fixed( _options.sizeReal, v );

		{
		int r = _pointList[pointListIdx]._vc.x * 255;
		int g = _pointList[pointListIdx]._vc.y * 255;
		int b = _pointList[pointListIdx]._vc.z * 255;
		*iff << unsigned long( PSX_RGB( r, g, b ) );
		}

		Point3 thisVertex = mesh->verts[_pointList[pointListIdx].vertexIdx];
		thisVertex = VectorTransform( objOffsetMatrix, mesh->verts[_pointList[pointListIdx].vertexIdx]);
		thisVertex += objOffsetPos;

		// If this object is a group member, continue transforming from "object local" to
		// "group local" space.
		if ( _pNode->IsGroupMember() )
		{
			thisVertex = VectorTransform( groupOffsetMatrix, thisVertex );
			thisVertex += groupOffsetPos;
		}

		// The NoRot() method of Matrix3 seems to have a bug, in that it not only nulls
		// the rotations, but it resets scale to 1,1,1 also.  So, we get to do this one
		// transformation at a time...
		AffineParts affineParts;
		decomp_affine(_pNode->GetObjectTM(_theTime), &affineParts);

		// Apply translation from AffineParts if switched on
		if ( _options.bOutputGlobalPosition )
			thisVertex += affineParts.t;

		if ( _options.bOutputGlobalScale )
			thisVertex *= affineParts.k;

		if ( _options.bOutputGlobalRotation )
  		{
			Matrix3 rotMatrix;
			affineParts.q.MakeMatrix(rotMatrix);
			thisVertex = VectorTransform( rotMatrix, thisVertex );
		}

		*iff << Fixed( _options.sizeReal, thisVertex.x * _options.geometryScale );
		*iff << Fixed( _options.sizeReal, thisVertex.y * _options.geometryScale );
		*iff << Fixed( _options.sizeReal, thisVertex.z * _options.geometryScale );
		*iff << Comment( PrintF( "Vertex #%d", pointListIdx )() );
	}
	iff->exitChunk();

//	if ( bNeedDel )
//		 mesh->DeleteThis();
}

//=============================================================================

void
IffConversion::writeMesh()
{
//	Point3 tmpPoint, tmpPoint2;

	// Figure out what today's master scale conversion factor is
//	double masterScale = GetMasterScale( UNITS_METERS );

	{ // NAME
	char* meshname = _pNode->GetName();
	_iff->enterChunk( ID( "NAME" ) );
		*_iff << meshname;
	_iff->exitChunk();
	}

//	{ // ORIGIN (ORGN)
//	// tmpPoint = objTM.GetTrans();		// get translation row from Transformation Matrix
//	// tmpPoint /= masterScale;			// INCHES-TO-METERS HACK
//	tmpPoint = Point3( 0, 0, 0 );		// Ignore all PRS values

//	_iff->enterChunk( ID( "ORGN" ) );
//		*_iff << Fixed( _options.sizeReal, tmpPoint.x );
//		*_iff << Fixed( _options.sizeReal, tmpPoint.y );
//		*_iff << Fixed( _options.sizeReal, tmpPoint.z );
//	_iff->exitChunk();
//	}

// kts always write the starting frame into the geometry
//	if ( options.bOutputAnimation )
//	{
//		for ( _theTime = 0; _theTime < 10; _theTime += 1 )
//			writeMeshFrame( _theTime );
//	}
//	else
		writeMeshFrame( 0, _iffGeometry );

	// write Material node
	int idxSubMaterial;
	int nMaterials = 0;

	const int MAX_SUB_MATERIAL_SLOTS = 256;
	int subMatMap[MAX_SUB_MATERIAL_SLOTS];
	bool subMatUsed[MAX_SUB_MATERIAL_SLOTS];

	bool hasSubMaterials = false;

	Mtl* thisMtl = _pNode->GetMtl();
	assert(thisMtl);
	if ( thisMtl )
	{
		const int TEXTURE_NAME_LEN = 256;

		// Walk the face list and build a list of which submaterials are actually being used
		// by something
		for (idxSubMaterial=0; idxSubMaterial < MAX_SUB_MATERIAL_SLOTS; ++idxSubMaterial)
		{
			subMatMap[idxSubMaterial] = -1;
			subMatUsed[idxSubMaterial] = false;
		}

		nMaterials = thisMtl->NumSubMtls();

		if(nMaterials)
			hasSubMaterials = true;
		nMaterials = max( 1, nMaterials );		// if this is a single-material object

		for (int faceIdx=0; faceIdx < _faceList.size(); ++faceIdx)
		{
			RemappedFace thisFace = _faceList[faceIdx];
			subMatUsed[thisFace.matIdx%nMaterials] = true;		// flag this subMat to be written out
		}

		// kts warning: if you change this struct, both the game engine and textile need to be updated!
		_iffMaterial->enterChunk( ID( "MATL" ) );


		// once per material
		int outputMaterialIndex = 0;
		for ( idxSubMaterial=0; idxSubMaterial < nMaterials; ++idxSubMaterial )
		{
			if (!hasSubMaterials || subMatUsed[idxSubMaterial])
			{
				// remap the index so the face list will match
				subMatMap[idxSubMaterial] = outputMaterialIndex++;

				// first gather all the data we need about this material
				long flags = 0;

				Mtl* thisSubMtl = nMaterials == 1 ? thisMtl : thisMtl->GetSubMtl( idxSubMaterial );
				assert( thisSubMtl );

				if ( ((StdMat*)thisSubMtl)->GetTwoSided() )
					flags |= material::DOUBLE_SIDED;


				if ( ((StdMat*)thisSubMtl)->GetShading() != SHADE_CONST )
					flags |= material::GOURAUD_SHADED;

				Color tempColor = thisSubMtl->GetDiffuse();

				// look up texture name
				char textureName[ TEXTURE_NAME_LEN ];
				memset( textureName, '\0', sizeof( textureName ) );
	//			for(int index=0;index<TEXTURE_NAME_LEN;index++)
	//				textureName[index] = 0;

				bool greyOutFlatColor = false;
				Texmap* thisTexmap = thisSubMtl->GetSubTexmap(ID_DI);	// get DIFFUSE map, always!
				if ( thisTexmap )
				{
					assert(thisTexmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0));	// must be a BitmapTex
					char* foo = ((BitmapTex*)thisTexmap)->GetMapName();
					assert( strlen( foo ) < TEXTURE_NAME_LEN );
					strcpy( textureName, foo );

					char name[TEXTURE_NAME_LEN];
					char ext[_MAX_EXT];

					_splitpath(textureName,NULL,NULL,name,ext);
					_makepath(textureName,NULL,NULL,name,ext);

					flags |= material::TEXTURE_MAPPED;

					float amount = ((StdMat*)thisSubMtl)->GetTexmapAmt(ID_DI, TimeValue(0));

					if(amount == 1.0)
						greyOutFlatColor = true;
					else		            // don't do anything if set to 0.5
						AssertMessageBox( amount == 0.5, "Object <" << _pNode->GetName() << "> has a material with an incorrect map value setting (only 50 & 100 are allowed, this is set to " << amount);
				}

				float opacity = ((StdMat*)thisSubMtl)->GetTexmapAmt( ID_OP, TimeValue( 0 ) );

				// finally, write it all out
				// first write out flags
				*_iffMaterial << unsigned long( flags );
				*_iffMaterial << Comment( "flags: [FLAT_SHADED=0, GOURAUD_SHADED=1] [SOLID_COLOR=0, TEXTURE_MAPPED=2] [SINGLE_SIDED=0, TWO_SIDED=8]" );

				// color used for flat shading
				if(greyOutFlatColor)
					*_iffMaterial << unsigned long( PSX_RGB( 128, 128, 128 ) );
				else
					*_iffMaterial << unsigned long( PSX_RGB( FLto255(tempColor.r), FLto255(tempColor.g), FLto255(tempColor.b) ) );
				*_iffMaterial << Comment(  "Colour" );

	// kts removed temporarily 5/20/98 6:07PM
	//			*_iffMaterial << Fixed( _options.sizeReal, opacity );
	//			*_iffMaterial << Comment( "Opacity" );

				{ // now output texture name
				*_iffMaterial << Comment( PrintF( "Material #%d [%s]", idxSubMaterial, textureName )() );
				istrstream stream( textureName, TEXTURE_NAME_LEN );
				*_iffMaterial << stream;
				}

				*_iffMaterial << Comment();
			}
		}

		_iffMaterial->exitChunk();
	}


	// Read face list and dump an FACE chunk
	_iffGeometry->enterChunk( ID( "FACE" ) );

	for (int faceIdx=0; faceIdx < _faceList.size(); ++faceIdx)
	{
		RemappedFace thisFace = _faceList[faceIdx];

		*_iffGeometry << unsigned short( thisFace.v[0] );
		*_iffGeometry << unsigned short( thisFace.v[2] );
		*_iffGeometry << unsigned short( thisFace.v[1] );
		if (hasSubMaterials)
		{
			unsigned short materialIndex = unsigned short( subMatMap[thisFace.matIdx % nMaterials] );
			RangeCheck(0,materialIndex, nMaterials);
			*_iffGeometry << materialIndex;
		}
		else
			*_iffGeometry << unsigned short( 0 );
		*_iffGeometry << Comment( "" );
	}
	_iffGeometry->exitChunk();

}

//=============================================================================

void
IffConversion::writeAnimation()
{
	const int timeInterval = int(float(TIME_TICKSPERSEC)*0.1);  // 10 per second

	Interface* ip = GetCOREInterface();
	assert( ip );

	Interval animRange = ip->GetAnimRange();

	_iff->enterChunk( ID( "AHDR" ) );
		*_iff << Comment( "header must come before any VRTX chunks" );
		*_iff << Comment( "Header: int16: frameCount" );

		char tempArray[200];
		strstream temp(tempArray,200,ios::out);
		temp << "timeInterval =  " << timeInterval << ", startTime = " << animRange.Start() << ",endTime = " << animRange.End() <<
		"animRange.End()-animRange.Start()%timeInterval) = " << (animRange.End()-animRange.Start())%timeInterval <<	'\0';
		_iff->out_comment( tempArray );
		int frameCount = ((animRange.End()-animRange.Start())/timeInterval);
		if((animRange.End()-animRange.Start())%timeInterval)
			frameCount++;
		if((animRange.End()-animRange.Start())<timeInterval)
			frameCount=1;
		*_iff << unsigned short( frameCount );
	_iff->exitChunk();

	// this will output 10 frames over one second
	int actualFrameCount = 0;
	for ( TimeValue _theTime = animRange.Start(); _theTime < animRange.End(); _theTime += timeInterval )
	{
		writeMeshFrame( _theTime, _iffAnimation );
		actualFrameCount++;
	}
	assertEq(actualFrameCount,frameCount);
}

//=============================================================================

bool
IffConversion::objectIsClass( const char* szClassTag ) const
{
	bool bObjectIsClass = false;

	return bObjectIsClass;
}


void
IffConversion::writeHandles()
{
	assert( _pNode );

	ObjectState os = _pNode->EvalWorldState( TimeValue( 0 ) );

	Object* pObject = os.obj;
	assert( pObject );

	Object* pObjectCheck;
	pObjectCheck = _pNode->GetObjectRef();
	if ( pObjectCheck )
	{
		IDerivedObject* pdObject = (IDerivedObject*)pObjectCheck;
		assert( pdObject );

		int nModifiers = pdObject->NumModifiers();
		bool chunkEntered = false;
		for ( int idxModifier=0; idxModifier<nModifiers; ++idxModifier )
		{
			Modifier* pModifier = pdObject->GetModifier( idxModifier );
			assert( pModifier );

			if ( pModifier->ClassID() == Handle_ClassID )
			{
				// Start a new chunk if we haven't already
				if (!chunkEntered)
				{
					_iffHandle->enterChunk( ID( "HNDL" ) );
					chunkEntered = true;
				}

				IParamArray* pb = pModifier->GetParamBlock();
				assert( pb );

				Interval ivalid;
				bool bSuccess;
				int lid;
				bSuccess = pb->GetValue( PB_INDEX_ID, TimeValue( 0 ),
					lid, ivalid );
				assert( bSuccess );

				int MAXVertexIndex=-1, remappedVertexIndex=-1;
				bSuccess = pb->GetValue( PB_INDEX_SELECTED_VERTEX, TimeValue( 0 ),
					MAXVertexIndex, ivalid );
				assert( bSuccess );

				for (unsigned int searchIdx=0; searchIdx < _pointList.size(); ++searchIdx)
					if (_pointList[searchIdx].vertexIdx == MAXVertexIndex)
					{
						remappedVertexIndex = searchIdx;
						searchIdx = _pointList.size();	// terminate loop
					}

				assert( remappedVertexIndex != -1 );
				char szId[ 5 ];
				strncpy( szId, (char*)&lid, 4 );
				szId[ 4 ] = '\0';

				*_iffHandle << ID(szId);
				*_iffHandle << unsigned long( remappedVertexIndex );
			}
		}

		// Close the HNDL chunk, if we ever started one
		if (chunkEntered)
			_iffHandle->exitChunk();
	}
}


IffConversion::~IffConversion()
{
}


bool
CHECKBOX( bool& bVar, const char* szRegistryEntry )
{
	char str[ 512 ];

	if ( GetLocalMachineStringRegistryEntry( szWorldFoundryGDK "\\max2iff", szRegistryEntry, str, sizeof( str ) ) )
		return atoi( str );
	else
		return bVar;
}

//==============================================================================

IffConversion::IffConversion( INode* pNode, _IffWriter* iff, const max2iffOptions* options )
{
	assert( pNode );
	ObjectState os = pNode->EvalWorldState( TimeValue( 0 ) );
//	AssertMessageBox( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID, "Object <" << _pNode->GetName() << "> cannot be converted into a GEOMOBJECT");
	if (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
		return;


	_pNode = pNode;
	assert( iff );
	_iff = iff;
	assert( options );
	_options = *options;

	Interface* ip = GetCOREInterface();
	assert( ip );

	HWND _hwndMax = ip->GetMAXHWnd();
	assert( _hwndMax );

	int retVal = 0;

	_iffGeometry = _options.bOutputGeometry ? _iff : &theNullIffWriter;
	assert( _iffGeometry );

	_iffAnimation = _options.bOutputAnimation ? _iff : &theNullIffWriter;
	assert( _iffAnimation );

	_iffHandle = _options.bOutputHandles ? _iff : &theNullIffWriter;
	assert( _iffHandle );

	_iffMaterial = _options.bOutputMaterials ? _iff : &theNullIffWriter;
	assert( _iffMaterial );

	_iffEvent = _options.bOutputEvents ? _iff : &theNullIffWriter;
	assert( _iffEvent );

	// Walk this mesh's Face list and build a list of unique points
	BOOL bNeedDel;
	NullView nullView;
	bool deleteMeshObject = false;	// Did we make this mesh up, or did MAX do it?

	// If this is a single object, just output the mesh.  If it's a group object, create one
	// great big mesh from all of the child meshes first.
//	if (pNode->IsGroupHead() == false)
		mesh = ((GeomObject*)os.obj)->GetRenderMesh( TimeValue(0), _pNode, nullView, bNeedDel );
//	else
//	{
//		mesh = CreateGroupMesh();
//		deleteMeshObject = true;
//	}

	assert( mesh );

	for (int faceIdx=0; faceIdx < mesh->numFaces; ++faceIdx)
	{
		int numFaces = mesh->numFaces;

		Face thisFace = mesh->faces[faceIdx];
		TVFace thisTVFace;

		VertColor vc[ 3 ];
		vc[ 0 ] = vc[ 1 ] = vc[ 2 ] = VertColor( 0.0, 0.0, 0.0 );
#if MAX_RELEASE >= 2000
		if ( mesh->vertCol )
		{
			#pragma message ("KTS " __FILE__ ": add code here to set the PRELIT flag in the material this face references")
#if 1
			// kts get this right
			assert(ValidPtr(mesh));
			assert(ValidPtr(mesh->vcFace));
			TVFace& vcFace = mesh->vcFace[ faceIdx ];
			assert(vcFace.getTVert(0) < mesh->numCVerts);
			assert(vcFace.getTVert(1) < mesh->numCVerts);
			assert(vcFace.getTVert(2) < mesh->numCVerts);
			vc[0] = mesh->vertCol[ vcFace.getTVert(0) ];
			vc[1] = mesh->vertCol[ vcFace.getTVert(1) ];
			vc[2] = mesh->vertCol[ vcFace.getTVert(2) ];
#else
			vc[ 0 ] = mesh->vertCol[ faceIdx*3 + 0 ];
			vc[ 1 ] = mesh->vertCol[ faceIdx*3 + 1 ];
			vc[ 2 ] = mesh->vertCol[ faceIdx*3 + 2 ];
#endif
		}
#endif

		if (mesh->getNumTVerts() > 0)
			thisTVFace = mesh->tvFace[faceIdx];
		else
			thisTVFace.t[0] = thisTVFace.t[1] = thisTVFace.t[2] = 0;

		RemappedFace newFace;	// the indices in this face refer to OUR vertex list, not MAX's
		newFace.matIdx = thisFace.getMatID();

		for (int faceVertexNumber=0; faceVertexNumber < 3; ++faceVertexNumber)
		{
			XYZUV tempXYZUV;
			tempXYZUV.vertexIdx = thisFace.getVert(faceVertexNumber);
			if (mesh->getNumTVerts() > 0)
				tempXYZUV.uvIdx = thisTVFace.getTVert(faceVertexNumber);
			else
				tempXYZUV.uvIdx = 0;

			tempXYZUV._vc = vc[ faceVertexNumber ];

			// add it to the list if it doesn't exist yet
			bool foundIt = false;
			for (unsigned int searchIdx=0; (searchIdx < _pointList.size()) && !foundIt; ++searchIdx)
			{
				if (_pointList[searchIdx] == tempXYZUV)
				{
					newFace.v[faceVertexNumber] = searchIdx;
					foundIt = true;
				}
			}

			if (!foundIt)
			{
				newFace.v[faceVertexNumber] = _pointList.size();
				_pointList.push_back(tempXYZUV);
			}
		}

		_faceList.push_back(newFace);
	}

	int pointListSize = _pointList.size();

	if ( _options.bOutputGeometry )
	{
		_iff->enterChunk( ID( "MODL" ) );

		{	// Write the header
		_iff->enterChunk( ID( "SRC" ) );
			*_iff << PrintF( "3D Studio MAX  Lock #%08x", HardwareLockID() )();
		_iff->exitChunk();

		_iff->enterChunk( ID( "TRGT" ) );
			*_iff << "World Foundy Game Engine (www.worldfoundry.org)";
		_iff->exitChunk();
		}

		writeMesh();
		writeHandles();

		_iff->exitChunk();
	}

	if ( _options.bOutputAnimation )
	{
		_iff->enterChunk( ID( "ANIM" ) );

  			{	// Write the header
//			_iff->enterChunk( ID( 'NAME' ) );	// 4 character id for this animation
//				_iff->out_string( "????" );
//			_iff->exitChunk();

			_iff->enterChunk( ID( "SRC" ) );
				*_iff << PrintF( "3D Studio MAX  Lock#: %08x", HardwareLockID() )();
				char* szUser = getenv( "USERNAME" );
				if ( !szUser )
					szUser = "[Unknown]";
				*_iff << PrintF( "User: %s", szUser )();
				char* szMachine = getenv( "USERDOMAIN" );
				if ( !szMachine )
					szMachine = "[Unknown]";
				*_iff << PrintF( "Machine: %s", szMachine )();
			_iff->exitChunk();

			_iff->enterChunk( ID( "TRGT" ) );
				*_iff << "World Foundy Game Engine (www.worldfoundry.org)";
			_iff->exitChunk();
			}

		writeAnimation();

		_iff->exitChunk();
	}

//	// Delete the "made up" mesh, if necessary
//	if (deleteMeshObject)
//		delete mesh;
}

//============================================================================

Mesh*
IffConversion::CreateGroupMesh()
{

#if 0
	// Create empty mesh
	Mesh* combinedMesh = new Mesh;
	int numVerts=0, numFaces=0, numTVerts=0, numCVerts=0;
	Mesh* tempMesh;
	BOOL bNeedDel;
	NullView nullView;

	int nChildren = _pNode->NumberOfChildren();
	assert( nChildren > 0 );

	Mesh* childArray = new Mesh*[nChildren];
	for (int childIdx=0; childIdx < nChildren; childIdx++)
	{
		INode* thisChild = _pNode->GetChildNode(childIdx);
		ObjectState os = thisChild->EvalWorldState( TimeValue(0) );
		tempMesh = ((GeomObject*)os.obj)->GetRenderMesh( TimeValue(0), thisChild, nullView, bNeedDel );
		numVerts += tempMesh->getNumVerts();
		numFaces += tempMesh->getNumFaces();
		numTVerts += tempMesh->getNumTVerts();
		numCVerts += tempMesh->getNumVertCol();
		childArray[childIdx] = tempMesh;
	}

	// Allocate storage for all of this stuff in the combined mesh
	AssertMessageBox( combinedMesh->setNumVerts(numVerts), "Unable to allocate vertex space");
	AssertMessageBox( combinedMesh->setNumFaces(numFaces), "Unable to allocate face space");
	AssertMessageBox( combinedMesh->setNumTVerts(numTVerts), "Unable to allocate texture vertex space");
	AssertMessageBox( combinedMesh->setNumVertCol(numCVerts), "Unable to allocate color vertex space");

	// Copy all vertices
	int groupVertexIdx = 0;
	for (int childIdx=0; childIdx < nChildren; childIdx++)
	{
		for (int childVertexIdx=0; childVertexIdx < childArray[childIdx]->getNumVerts(); childVertexIdx++)
			combinedMesh->setVert( groupVertexIdx++, childArray[childIdx]->getVert(childVertexIdx) );
	}

	// Copy all faces (and fix the vertex indices)
	int groupFaceIdx = 0;
	int vertexOffset=0;

	for (int childIdx=0; childIdx < nChildren; childIdx++)
	{
		// Fix the vertex indices if this isn't the first child
		if (childIdx > 0)
			vertexOffset += childArray[childIdx-1]->getNumVerts();

		for (int childFaceIdx=0; childFaceIdx < childArray[childIdx]->getNumFaces(); childFaceIdx++)
		{
			Face tempFace = *childArray[childIdx]->faces[childFaceIdx];

			tempFace.v[0] += vertexOffset;
			tempFace.v[1] += vertexOffset;
			tempFace.v[2] += vertexOffset;

#pragma message("WARNING: Need to do some fixup on the per-face material indices!!!")

			memcpy(combinedMesh->faces[groupFaceIdx++], &tempFace, sizeof(Face));
		}
	}


#endif	// 0
	return NULL;
}

//============================================================================
