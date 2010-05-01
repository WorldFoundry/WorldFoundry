//============================================================================
// level.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <math.h>

#include "levelcon.hp"
#include "pigtool.h"
#include "model.hp"
#include "object.hp"
#include "level.hp"
#include "oad.hp"
#include "colbox.hp"
#include "room.hp"
#include "3d.hp"
#include "asset.hp"
#include "file3ds.hp"

#include <stl/algo.h>

#pragma pack(1);
extern "C" {
#include <source\oas\oad.h>			// from the velocity project
};
#pragma pack();

#pragma pack(1);
extern "C" {
#include "3dsftk.h"
};
#pragma pack();

#include <source\levelcon.h>		// included from velocity\source
#include <brender.h>

#include <pclib/stdstrm.hp>
#include <pclib/hdump.hp>
#include <stl/bstring.h>

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

QLevel::QLevel(const char* levelFileName, const char* lcFileName)
{
	DBSTREAM1 ( cprogress << "QLevel::QLevel:" << endl; )
	// kts added 10/1/96 20:28 to make object index 0 invalid
	assert(objects.size() == 0);
	br_euler newRotation;
	newRotation.order = BR_EULER_XYZ_S;
	newRotation.a = 0;
	newRotation.b = 0;
	newRotation.c = 0;
	QColBox collision(QPoint(0,0,0),QPoint(1,1,1));
	QObjectAttributeData newobjOAD;						// make an empty oad
	int32 oadFlags = 0;
	objects.push_back
	(
		new QObject
		(
			"NULL_Object",
			1,-1,						// object type is not relevent for first object
			QPoint(0,0,0),
			QPoint(0,0,0),
			newRotation,
			collision,
			oadFlags,
			-1,
			newobjOAD
		)
	);
	assert(objects.size() == 1);
	// end kts addition

	commonAreaSize = 0;
	DBSTREAM3( cprogress << "Loading .LC file <" << lcFileName << ">" << endl; )
	LoadLCFile(lcFileName);
	LoadAssetFile(lcFileName);						// used to find assets.txt files
	LoadOADFiles(lcFileName);						// used to find the .oad files
	Load3DStudio(levelFileName);
	DBSTREAM3( cprogress << "Updating references, etc." << endl; )
}

//============================================================================

QLevel::~QLevel()
{
	objects.erase(objects.begin(), objects.end());
	delete levelFile3ds;
}

//============================================================================
// kts private call, do all of the crap to locate and load a named xdata chunk from a mesh
// returns size of chunk loaded, if -1, there was an error

int
LoadNamedXDataChunk(QFile3ds* levelFile3ds,const char* objectName, chunk3ds* xDataChunk,char* destBuffer,size_t bufferSize,const char* name,chunktag3ds chunkType, ostream& error)
{
	chunk3ds* tempDataChunk;
	chunk3ds* stringDataChunk;

	DBSTREAM3( cdebug << "LoadNamedXDataChunk: looking for chunk named <" << name << "> in chunk <" << hex << xDataChunk << ">, object <" << objectName << ">" << endl; )

	tempDataChunk = levelFile3ds->FindNamedXDataChunk(xDataChunk,name);
	if(!tempDataChunk)
	 {
//		error << "Levelcon Error: Unable to locate XData chunk <" << name << "> from object <" << objectName  << ">" << endl;
//		levelFile3ds->DumpChunk3DS(xDataChunk, 4, FALSE, objectName);
		return(-1);
	 }

	assert(tempDataChunk);
	stringDataChunk = ChunkFindChild(tempDataChunk,chunkType);		// actual data is stored in the first string chunk
	if(!stringDataChunk)
	 {
		cerror << "Levelcon Error: XData chunk <" << name << "> Doesn't contain desired subchunk <" << chunkType  << ">" << endl;
		exit(1);
	 }

	assert(stringDataChunk);
	int stringChunkLength = levelFile3ds->GetChunkDataSize(stringDataChunk);
	if(stringChunkLength > bufferSize)
	 {
	 	cerror << "Levelcon error: XData chunk <" << name << "> is too large: " << stringChunkLength << " bytes, Maximum is <" << bufferSize << ">" << endl;
		exit(1);
	 }
	assert(stringChunkLength <= bufferSize);
	levelFile3ds->GetChunkData(stringDataChunk,destBuffer);
	return(levelFile3ds->GetChunkDataSize(stringDataChunk)-chunkHeaderSize);
}

//============================================================================

//const int OADBUFFER_SIZE = 60000;			// Now in level.hp
char oadBuffer[OADBUFFER_SIZE];

//-----------------------------------------------------------------------------
// returns size of xdata chunk found, -1 for not found (I think)

int
FindAndLoadMeshXDataChunk(QFile3ds* levelFile3ds,const char* meshName,const char* chunkName, char buffer[],int bufferSize,chunktag3ds chunkType)
{
	DBSTREAM3( cdebug << "FindAndLoadMeshXDataChunk: meshname = <" << meshName << ">, chunk name = <" << chunkName << ">" << endl; )
	chunk3ds* meshChunk = levelFile3ds->FindMeshChunk(meshName);			// get chunk in 3ds file for this mesh
	assert(meshChunk);
	DBSTREAM3( cdebug << "  meshChunk = <" << meshChunk << ">" << endl; )
	int xdataSize = -1;
	if(meshChunk)
	 {
		// find xdata chunk
		DBSTREAM3( cdebug << "FindAndLoadMeshXDataChunk: looking for xdata chunk for object named <" << meshName << ">" << endl; )
		chunk3ds* xDataChunk = levelFile3ds->FindXDataChunk(meshChunk);		// now look up the xdata section for this mesh
//		assert(xDataChunk);
		if ( xDataChunk )
		 {	// now 	look up particular xdata chunk in the xdata section
			DBSTREAM3( cdebug << "FindAndLoadMeshXDataChunk: looking for xdata chunk <" << chunkName << ">" << endl; )
			xdataSize = LoadNamedXDataChunk(levelFile3ds, meshName, xDataChunk, buffer, bufferSize, chunkName, chunkType,cerror);
			DBSTREAM3( if(xdataSize != -1) )
		 		DBSTREAM3( cdebug << "FindAndLoadMeshXDataChunk: found, size = " << xdataSize << endl; )
		 }
	 }
	return(xdataSize);
}

//-----------------------------------------------------------------------------
// load a single 3DStudio mesh with its .oad data, and add it to the various lists

#define OAD_CHUNK_NAME "Cave Logic Studios Object Attribute Editor v0.3"
const char CLASS_CHUNK_NAME[] = "Cave Logic Studios Class Object Editor";

void
QLevel::Get3DStudioMesh(QFile3ds* levelFile3ds,mesh3ds* mesh)
{
	int typeIndex = 0;
	int pathIndex = -1;
	char* oadData = NULL;
	size_t oadDataSize = 0;
	const int STRINGBUFFER_SIZE = 256;
	char stringBuffer[STRINGBUFFER_SIZE];

	chunk3ds* meshChunk = levelFile3ds->FindMeshChunk(mesh->name);			// find mesh chunk
	assert(meshChunk);
	int xDataSize = -1;
	if(meshChunk)
	 {
		// if class name not found, make it a debug object
		if(FindAndLoadMeshXDataChunk(levelFile3ds,mesh->name,CLASS_CHUNK_NAME,stringBuffer,STRINGBUFFER_SIZE,XDATA_STRING) == -1)
			strcpy( stringBuffer, "platform.oad" );
//			strcpy( stringBuffer, "debugobj.oad" );

		chunk3ds* xDataChunk = levelFile3ds->FindXDataChunk(meshChunk);		// find xdata section of mesh chunk

#if 0
		// TODO: Add -final switch
		if ( bFinal && strcmp( stringBuffer, "debugobj.oad" ) == 0 )
		 {
			cerror << "No DebugObj's allowed in final game output" << endl;
			exit( 1 );
		 }
#endif

		char fileName[_MAX_FNAME];
		_splitpath(stringBuffer,NULL,NULL,fileName,NULL);
		string className(fileName);

		typeIndex = GetClassIndex(className);
		if(typeIndex == -1)
		 {
			cerror << "LevelCon Error: Object Type of <" << className << "> in 3D Studio file not found in .LC file" << endl;
			exit(1);
		 }

		DBSTREAM3( cdebug << "Object Type is " << typeIndex << ", which is " << objectTypes[typeIndex] << endl; )

#pragma message ( __FILE__ " kts remove after a while" )
		if( xDataChunk && levelFile3ds->FindNamedXDataChunk(xDataChunk,"Cave Logic Studios Object Attribute Editor v0.2"))
		 {
			cerror << "Levelcon Error: level contains old OAD format data, please run Bill's converter on it" << endl;
			exit(1);
		 }

		// now find attribute data so that object will have it
		// kts added to allow objects with all defaults
		if( xDataChunk && levelFile3ds->FindNamedXDataChunk(xDataChunk,OAD_CHUNK_NAME))
		 {
			oadDataSize = LoadNamedXDataChunk(levelFile3ds,mesh->name,xDataChunk,oadBuffer,OADBUFFER_SIZE,OAD_CHUNK_NAME,XDATA_VOID,cerror);
			assert(oadDataSize != -1);
			// zero byte data size allowed
//#pragma message ( "kts turn this back on when Bill fixes the new .prj oad format" )
//			assert(oadDataSize);
			if(oadDataSize)
				oadData = oadBuffer;
			else
				oadData = NULL;
		 }
		else
		 {
			// kts added to allow objects with all defaults
			oadData = NULL;
			oadDataSize = 0;
			DBSTREAM3( cstats << "LevelCon Warning: OAD Data not found for object <" << className << ">, using defaults from .oad file" << endl; )
		 }

// kts new new oad format
		// load OAD from object
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh:Parsing new OAD chunk from object <" << mesh->name << ">" << endl; )
//		strstream objOADStream(oadBuffer,oadDataSize,ios::in|ios::binary);
//		objOAD.LoadEntries(objOADStream,cerror);
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: OAD from object" << endl; )
//		cdebug << objOAD << endl;
		// now sync oad up with .oad file
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Applying OAD:: " << endl; )
//		cdebug << objectOADs[typeIndex] << endl;
		QObjectAttributeData newobjOAD = objectOADs[typeIndex];
		newobjOAD.Apply(oadBuffer,oadDataSize,mesh->name);
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: OAD after apply: " << endl; )
//		cdebug << newobjOAD << endl;

		// deal with object paths
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: looking for keyframe data" << endl; )
		kfmesh3ds *kfmesh = NULL;
		InitObjectMotion3ds(&kfmesh, 30, 0, 0, 0, 0);
		PRINT_ERRORS_EXIT(stderr);
		assert(kfmesh);
		kfmesh->npkeys = 0;
		GetObjectMotionByName3ds(levelFile3ds->GetDatabase(), mesh->name, &kfmesh);
		PRINT_ERRORS_EXIT(stderr);
		if(kfmesh->npkeys > 1)
		 {
			DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Objects path found with <" << kfmesh->npkeys << "> keys" << endl; )
			assert(kfmesh->npkeys > 1);
			QPath path(BrFloatToScalar(kfmesh->pos[0].x),
					   BrFloatToScalar(kfmesh->pos[0].y),
					   BrFloatToScalar(kfmesh->pos[0].z));

			for (int keyIndex = 0; keyIndex < kfmesh->npkeys; keyIndex++)
			 {
				QPoint point(BrFloatToScalar(kfmesh->pos[keyIndex].x),
							 BrFloatToScalar(kfmesh->pos[keyIndex].y),
							 BrFloatToScalar(kfmesh->pos[keyIndex].z)
							);

				path.Add(point,kfmesh->pkeys[keyIndex].time);
				DBSTREAM3( cstats << "Object Path data: " << point << "Time: <" << kfmesh->pkeys[keyIndex].time << ">" << endl; )
			 }
			path.SizeOfOnDisk();

			vector<QPath>::iterator where = find(paths.begin(),paths.end(),path);
			if(where == paths.end())
			 {
				paths.push_back(path);					// create new path
				pathIndex = paths.size() - 1;
			 }
			else
			 {
				pathIndex = where  - paths.begin();		// use existing path
			 }
		 }
		assert(kfmesh->npkeys > 0);

		// find mesh name data so we can get the model name, if LEVELCONFLAG_NOMESH isn't set
		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: look for mesh name: " << endl; )
		bool isDebugObject = false;
		int modelIndex = MODEL_NULL;
		if(!objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_NOMESH))
		 {
			DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: nomesh flag not found, so looking for mesh name: " << endl; )
                        if(
                                xDataChunk &&
                                (LoadNamedXDataChunk(levelFile3ds,mesh->name,xDataChunk,stringBuffer,STRINGBUFFER_SIZE,"Cave Logic Studios Mesh Name Assigner",XDATA_STRING,cnull) != -1) &&
                                strlen(stringBuffer)
                        )
			 {
				modelIndex = FindModel(stringBuffer);

				// kts kludge to set the model name in the OAD data since it is special, and stored in a seperate xdata chunk
					// kts cast away constness in this one case
				cdebug << "checking for mesh name in object " << mesh->name << endl;
				QObjectAttributeDataEntry* oadMeshName = (QObjectAttributeDataEntry*)newobjOAD.GetEntryByName("Mesh Name");
				if(oadMeshName)
				 {
					cdebug << "    found, set to " << stringBuffer << endl;
					assert(strlen(stringBuffer) < 512);
					cdebug << " before dump:\n" << *oadMeshName << endl;
					oadMeshName->SetString(stringBuffer);
//					strcpy(oadMeshName->GetString(),stringBuffer);
					cdebug << " set dump:\n" << *oadMeshName << endl;
					cdebug << *newobjOAD.GetEntryByName("Mesh Name") << endl;
				 }


				if(modelIndex == -1)
			 	{
			 		cerror << "Levelcon Error: problem Finding Model in object <" << mesh->name << ">" << endl;
			 		exit(1);
			 	}
				assert(modelIndex >= 0);
			 }
			else
			 {
				// kts changed 5/10/96 3:13PM
				modelIndex = -1;
				DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Not Adding Object Mesh of type <" << typeIndex << "> since it does'nt have a mesh name" << endl; )
				isDebugObject = true;
//			 	cerror << "Levelcon Error: mesh name not found in object <" << mesh->name << ">, check mesh name field in common page" << endl;
//			 	exit(1);
			 }
		 }
		else
		 {
			modelIndex = -1;
			DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Not Adding Object Mesh of type <" << typeIndex << "> since its OAD contains a LEVELCONFLAG_NOMESH field" << endl; )
			isDebugObject = true;
		 }

		// create collision box
		QColBox collision;
		collision.Bound(*mesh,*kfmesh);

		DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: colbox before collision for object " << mesh->name << ":" << endl; )
		DBSTREAM3( cdebug << collision << endl; )

// kts removed 5/28/96 4:08PM since 3d Studio pre-rotates all of the vertices of an object
//		if ((kfmesh->rot[0].angle != 0) && (isDebugObject == false))
//		{
//			cdebug << "QLevel::Get3DStudioMesh: colbox before collision for object " << mesh->name << ":" << endl;
//			cdebug << collision << endl;
//			QPoint rot_axis(BrFloatToScalar(kfmesh->rot[0].x),
//							BrFloatToScalar(kfmesh->rot[0].y),
//							BrFloatToScalar(kfmesh->rot[0].z));
//			cdebug << "  rotation axis = " << rot_axis <<  ", angle = " << kfmesh->rot[0].angle << endl;

//			collision.Rotate(kfmesh->rot[0].angle, rot_axis);
//			cdebug << "QLevel::Get3DStudioMesh: colbox after collision:" << endl;
//			cdebug << collision << endl;
//		}

		// kts insure no rotations exist
//		assert(kfmesh->nrkeys > 0);

//		if(kfmesh->rot[0].angle != 0)
//			cstats << "LevelCon Warning: Object <" << className << "> has rotation" << endl;

		DBSTREAM3(
		if(
			kfmesh->scale[0].x != 1.0 ||
			kfmesh->scale[0].y != 1.0 ||
			kfmesh->scale[0].z != 1.0
		  )
			cstats << "LevelCon Warning: Object <" << className << "> has scaling" << endl; )

		// kts temp until velocity can handle thin floors
		if(collision.GetMin().x() == collision.GetMax().x())
		 {
			QPoint temp = collision.GetMin();
			temp.x(temp.x()-BrFloatToScalar(0.25));
			collision.SetMin(temp);
			DBSTREAM3( cdebug << "Min & Max x match at <" << BrScalarToFloat(collision.GetMax().x()) << ">, moved min to <" << BrScalarToFloat(collision.GetMin().x()) << ">" << endl; )
		 }
		if(collision.GetMin().y() == collision.GetMax().y())
		 {
			QPoint temp = collision.GetMin();
			temp.y(temp.y()-BrFloatToScalar(0.25));
			collision.SetMin(temp);
			DBSTREAM3( cdebug << "Min & Max y match at <" << BrScalarToFloat(collision.GetMax().y()) << ">, moved min to <" << BrScalarToFloat(collision.GetMin().y()) << ">" << endl; )
		 }
		if(collision.GetMin().z() == collision.GetMax().z())
		 {
			QPoint temp = collision.GetMin();
			temp.z(temp.z()-BrFloatToScalar(0.25));
			collision.SetMin(temp);
			DBSTREAM3( cdebug << "Min & Max z match at <" << BrScalarToFloat(collision.GetMax().z()) << ">, moved min to <" << BrScalarToFloat(collision.GetMin().z()) << ">" << endl; )
		 }

		DBSTREAM3( cdebug << "Coarse Bounding box:" << endl << collision << endl; )

		// create oadflags
		int32 oadFlags = newobjOAD.GetOADFlags();
		DBSTREAM3( cdebug << "QLevel::Get3dStudioMesh: oadFlags = <" << hex << oadFlags << ">" << dec << endl; )

		// check for presence of a LEVELCONFLAG_NOINSTANCES entry in the oad
		if(!objectOADs[typeIndex].ContainsButtonType(LEVELCONFLAG_NOINSTANCES))
		 {
			DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Calling QOBject constructor: oadData <" << !!oadData << ">, oadDataSize <" << oadDataSize << ">" << endl; )

			br_euler newRotation;
			newRotation.order = BR_EULER_XYZ_S;
//			newRotation.order = BR_EULER_XYX_S;

//#define kevins_old_way

#ifdef PHILS_OLD_WAY
//			float s = sin(kfmesh->rot[0].angle/2);
			float s = sin(-kfmesh->rot[0].angle/2);

			br_quat rot_quat = BR_QUAT(kfmesh->rot[0].x * s,
									   kfmesh->rot[0].y * s,
									   kfmesh->rot[0].z * s,
//									   cos(kfmesh->rot[0].angle/2));
									   cos(-kfmesh->rot[0].angle/2));			// kts added - 6/5/96 5:53PM

			BrQuatToEuler(&newRotation, &rot_quat);
#elif defined(kevins_old_way)
			br_matrix34 tempMatrix;
			br_angle tempAngle;
			br_vector3 tempVector;

			tempAngle = BR_ANGLE_RAD(-kfmesh->rot[0].angle);			// kts negate since 3ds uses a clockwise rotation
			tempVector.v[0] = BrFloatToScalar(kfmesh->rot[0].x);
			tempVector.v[1] = BrFloatToScalar(kfmesh->rot[0].y);
			tempVector.v[2] = BrFloatToScalar(kfmesh->rot[0].z);
			BrVector3Normalise(&tempVector,&tempVector);

			DBSTREAM3( cdebug << "tempAngle: " << BrScalarToFloat(BrAngleToRadian(tempAngle)) << endl; )
			DBSTREAM3( cdebug << "tempVector.x: " << BrScalarToFloat(tempVector.v[0]) << endl; )
			DBSTREAM3( cdebug << "tempVector.y: " << BrScalarToFloat(tempVector.v[1]) << endl; )
			DBSTREAM3( cdebug << "tempVector.z: " << BrScalarToFloat(tempVector.v[2]) << endl; )

			BrMatrix34Rotate(&tempMatrix,tempAngle,&tempVector);

			// kts dump matrix
			DBSTREAM3( cdebug << "tempMatrix(float): " << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(tempMatrix.m[0][0]) << "," << BrScalarToFloat(tempMatrix.m[0][1]) << "," << BrScalarToFloat(tempMatrix.m[0][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(tempMatrix.m[1][0]) << "," << BrScalarToFloat(tempMatrix.m[1][1]) << "," << BrScalarToFloat(tempMatrix.m[1][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(tempMatrix.m[2][0]) << "," << BrScalarToFloat(tempMatrix.m[2][1]) << "," << BrScalarToFloat(tempMatrix.m[2][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(tempMatrix.m[3][0]) << "," << BrScalarToFloat(tempMatrix.m[3][1]) << "," << BrScalarToFloat(tempMatrix.m[3][2]) << endl; )

			DBSTREAM3( cdebug << "tempMatrix(fixed): " << endl; )
			DBSTREAM3( cdebug << tempMatrix.m[0][0] << "," << tempMatrix.m[0][1] << "," << tempMatrix.m[0][2] << endl; )
			DBSTREAM3( cdebug << tempMatrix.m[1][0] << "," << tempMatrix.m[1][1] << "," << tempMatrix.m[1][2] << endl; )
			DBSTREAM3( cdebug << tempMatrix.m[2][0] << "," << tempMatrix.m[2][1] << "," << tempMatrix.m[2][2] << endl; )
			DBSTREAM3( cdebug << tempMatrix.m[3][0] << "," << tempMatrix.m[3][1] << "," << tempMatrix.m[3][2] << endl; )

			BrMatrix34ToEuler(&newRotation,&tempMatrix);
//			assert(newRotation.order == BR_EULER_XYX_S);
			assert(newRotation.order == BR_EULER_XYZ_S);

			DBSTREAM3( cdebug << "converting euler back to matrix for sanity check:" << endl; )
			br_matrix34 temp2Matrix;
			BrEulerToMatrix34(&temp2Matrix,&newRotation);

			// kts dump matrix
			DBSTREAM3( cdebug << "temp2Matrix(float): " << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(temp2Matrix.m[0][0]) << "," << BrScalarToFloat(temp2Matrix.m[0][1]) << "," << BrScalarToFloat(temp2Matrix.m[0][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(temp2Matrix.m[1][0]) << "," << BrScalarToFloat(temp2Matrix.m[1][1]) << "," << BrScalarToFloat(temp2Matrix.m[1][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(temp2Matrix.m[2][0]) << "," << BrScalarToFloat(temp2Matrix.m[2][1]) << "," << BrScalarToFloat(temp2Matrix.m[2][2]) << endl; )
			DBSTREAM3( cdebug << BrScalarToFloat(temp2Matrix.m[3][0]) << "," << BrScalarToFloat(temp2Matrix.m[3][1]) << "," << BrScalarToFloat(temp2Matrix.m[3][2]) << endl; )

			DBSTREAM3( cdebug << "temp2Matrix(fixed): " << endl; )
			DBSTREAM3( cdebug << temp2Matrix.m[0][0] << "," << temp2Matrix.m[0][1] << "," << temp2Matrix.m[0][2] << endl; )
			DBSTREAM3( cdebug << temp2Matrix.m[1][0] << "," << temp2Matrix.m[1][1] << "," << temp2Matrix.m[1][2] << endl; )
			DBSTREAM3( cdebug << temp2Matrix.m[2][0] << "," << temp2Matrix.m[2][1] << "," << temp2Matrix.m[2][2] << endl; )
			DBSTREAM3( cdebug << temp2Matrix.m[3][0] << "," << temp2Matrix.m[3][1] << "," << temp2Matrix.m[3][2] << endl; )
#else
			float	pi = acos(-1);
			float	x, y, z, r;
											// arbitrary vector and angle
//			r = (4.0 * pi) / 3.0;					// -120 degrees
//			x = y = z = sqrt(1.0 / 3.0);			// nake normalized 1,1,1 vector

			br_matrix34 tempMatrix;
			br_angle tempAngle;
			br_vector3 tempVector;

//			r = -kfmesh->rot[0].angle;			// kts negate since 3ds uses a clockwise rotation

			DBSTREAM3( cdebug << "Original Vector: x = " << kfmesh->rot[0].x )
				DBSTREAM3( << ", y = " << kfmesh->rot[0].y )
				DBSTREAM3( << ", z = " << kfmesh->rot[0].z << endl; )

//			DBSTREAM3( cdebug << "Pi = " << pi << endl; )
			DBSTREAM3( cdebug << "Original Angle: " << kfmesh->rot[0].angle << endl; )
//			DBSTREAM3( cdebug << "-Original Angle: " << -kfmesh->rot[0].angle << endl; )
			DBSTREAM3( cdebug << "2pi - Original Angle: " << (2*pi) - kfmesh->rot[0].angle << endl; )

			assert(kfmesh->rot[0].angle < (2*pi));
			assert(kfmesh->rot[0].angle > -(2*pi));
			r = (2*pi) - kfmesh->rot[0].angle;			// kts negate since 3ds uses a clockwise rotation
			x = kfmesh->rot[0].x;
			y = kfmesh->rot[0].y;
			z = kfmesh->rot[0].z;
			BrVector3Normalise(&tempVector,&tempVector);

			float newX,newY,newZ;
			RotationConvert(x,y,z,r,&newX, &newY, &newZ);
			DBSTREAM3( cdebug << "newX = " << newX << endl; )
			DBSTREAM3( cdebug << "newY = " << newY << endl; )
			DBSTREAM3( cdebug << "newZ = " << newZ << endl; )

			br_scalar sx, sy, sz;
			sx = BrFloatToScalar(newX);
			sy = BrFloatToScalar(newY);
			sz = BrFloatToScalar(newZ);
			DBSTREAM3( cdebug << "sx = " << sx << endl; )
			DBSTREAM3( cdebug << "sy = " << sy << endl; )
			DBSTREAM3( cdebug << "sz = " << sz << endl; )

			newRotation.a = BrRadianToAngle(sx);
			newRotation.b = BrRadianToAngle(sy);
			newRotation.c = BrRadianToAngle(sz);

			DBSTREAM3( cdebug << "br_angle a = " << newRotation.a << endl; )
			DBSTREAM3( cdebug << "br_angle b = " << newRotation.b << endl; )
			DBSTREAM3( cdebug << "br_angle c = " << newRotation.c << endl; )

#endif
			DBSTREAM3( cdebug << "Object has rotations:" << endl; )
			DBSTREAM3( cdebug << "     X: " << BrScalarToFloat(BrAngleToRadian(newRotation.a)) << endl; )
			DBSTREAM3( cdebug << "     Y: " << BrScalarToFloat(BrAngleToRadian(newRotation.b)) << endl; )
			DBSTREAM3( cdebug << "     Z: " << BrScalarToFloat(BrAngleToRadian(newRotation.c)) << endl << endl; )

			objects.push_back
			(
				new QObject
				(
					mesh->name,
					typeIndex,modelIndex,
					QPoint(
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->pos[0].x)),
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->pos[0].y)),
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->pos[0].z))
						),
					QPoint(
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->scale[0].x)),
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->scale[0].y)),
						BrFloatToScalar(FLOAT2VELOCITY(kfmesh->scale[0].z))
						),
					newRotation,
					collision,
					oadFlags,
					pathIndex,
					newobjOAD
				)
			);

			cdebug << "Push back succeeded." << endl;
		}
		DBSTREAM3( else )
			DBSTREAM3( cdebug << "QLevel::Get3DStudioMesh: Not Adding Object instance of type <" << typeIndex << "> since its OAD contains a LEVELCONFLAG_NOINSTANCES field" << endl; )
	 }
	else
		cwarn << "Mesh object <" << mesh->name << "> found with no .oad data" << endl;
}

//============================================================================