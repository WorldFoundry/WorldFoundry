//============================================================================
// oaddump.cc:
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

//#define __BOOL_DEFINED

#include <cstdio>
#include <cstdlib>
#include <pigsys/assert.hp>
#include <pigsys/pigsys.hp>
#include <iostream>
#include <fstream>
#include <strstream>
using namespace std;

#include <cpplib/stdstrm.hp>
#include <string>

#include "oad.hp"


#if 0
//============================================================================
// kts private call, do all of the crap to locate and load a named xdata chunk
// returns size of chunk loaded

int
LoadNamedXDataChunk(QFile3ds* levelFile3ds,char* objectName, chunk3ds* xDataChunk,char* destBuffer,size_t bufferSize,const char* name,chunktag3ds chunkType)
{
	chunk3ds* tempDataChunk;
	chunk3ds* stringDataChunk;

	tempDataChunk = levelFile3ds->FindNamedXDataChunk(xDataChunk,name);
	if(!tempDataChunk)
	 {
		cerror << "OadDump Error: Unable to locate XData chunk <" << name << "> from object <" << objectName  << ">" << endl;
		return(0);
		exit(1);
	 }

	assert(tempDataChunk);
	stringDataChunk = ChunkFindChild(tempDataChunk,chunkType);		// actual data is stored in the first string chunk
	if(!stringDataChunk)
	 {
		cerror << "OadDump Error: XData chunk <" << name << "> Doesn't contain desired subchunk" << endl;
		return(0);
		exit(1);
	 }

	assert(stringDataChunk);
	cdebug << "LoadNamedXDataChunk: data size = " << levelFile3ds->GetChunkDataSize(stringDataChunk) << endl;
	assert(levelFile3ds->GetChunkDataSize(stringDataChunk) < bufferSize);
	levelFile3ds->GetChunkData(stringDataChunk,destBuffer);
	return(levelFile3ds->GetChunkDataSize(stringDataChunk));
}

//=============================================================================

//const int OADBUFFER_SIZE = 30000;
const int OADBUFFER_SIZE = 60000;
char oadBuffer[OADBUFFER_SIZE];

//=============================================================================

#define OAD_CHUNK_NAME "Cave Logic Studios Object Attribute Editor v0.2"

void
Get3DStudioMesh(QFile3ds* levelFile3ds,mesh3ds* mesh)
{
	int typeIndex = 0;
	int pathIndex = -1;
	char* oadData = NULL;
	size_t oadDataSize = 0;
	const int STRINGBUFFER_SIZE = 256;
	char stringBuffer[STRINGBUFFER_SIZE];

	chunk3ds* meshChunk = levelFile3ds->FindMeshChunk(mesh->name);
	if(meshChunk)											// if .oad data present, create object
	 {
		// find xdata chunk
		chunk3ds* xDataChunk = levelFile3ds->FindXDataChunk(meshChunk);
		if(!xDataChunk)
		 {
			cerror << "OadDump Error: mesh <" << mesh->name << "> is missing class name" << endl;
			exit(1);
		 }
		assert(xDataChunk);							// ready to look for our data in the xdata chunk

		cout << "----------------------------------------------------------------------------" << endl;
		cout << "Object named <" << mesh->name << ">" << endl;

		// find class name so we can reference the correct .oad
		LoadNamedXDataChunk(levelFile3ds,mesh->name, xDataChunk,stringBuffer,STRINGBUFFER_SIZE,"Cave Logic Studios Class Object Editor",XDATA_STRING);
		cdebug << "QLevel::Get3DStudioMesh: Class name <" << stringBuffer << ">" << endl;

		char fileName[_MAX_FNAME];
		_splitpath(stringBuffer,NULL,NULL,fileName,NULL);
		string className(fileName);

		cout << "Class name <" << className << ">" << endl;
		// find mesh name data so we can get the model name, if LEVELCONFLAG_NOMESH isn't set
		LoadNamedXDataChunk(levelFile3ds,mesh->name,xDataChunk,stringBuffer,STRINGBUFFER_SIZE,"Cave Logic Studios Mesh Name Assigner",XDATA_STRING);
		cout << "Mesh name <" << stringBuffer << ">" << endl;

		// now find attribute data so that object will have it
		// kts added to allow objects with all defaults
		if(levelFile3ds->FindNamedXDataChunk(xDataChunk,OAD_CHUNK_NAME))
		 {
			oadDataSize = LoadNamedXDataChunk(levelFile3ds,mesh->name,xDataChunk,oadBuffer,OADBUFFER_SIZE,OAD_CHUNK_NAME,XDATA_VOID);
			assert(oadDataSize);
			oadData = oadBuffer;
		 }
		else
		 {
			// kts added to allow objects with all defaults
			oadData = NULL;
			oadDataSize = 0;
			cwarn << "OadDump Warning: OAD Data not found for object <" << className << ">, using defaults from .oad file" << endl;
		 }

		// load OAD from object
		cdebug << "QLevel::Get3DStudioMesh:Parsing OAD chunk from object" << endl;
		QObjectAttributeData objOAD;
		strstream objOADStream(oadBuffer,oadDataSize,ios::in|ios::binary);
		objOAD.LoadEntries(objOADStream,cerror);
		cout << "OAD from object <" << mesh->name << "> " << objOAD << endl;
	 }
	else
		cwarn << "Mesh object <" << mesh->name << "> found with no .oad data" << endl;
}

//=============================================================================

void
DumpPrjFile(const char* name)
{
	QFile3ds* levelFile3ds;						// 3ds database
	levelFile3ds = new QFile3ds(name);
	assert(levelFile3ds);

	// loop through all meshes, extracting xyz information
	ulong numElem = GetMeshCount3ds(levelFile3ds->GetDatabase());
	PRINT_ERRORS_EXIT(stderr);
	for (ulong index = 0; index < numElem; index++)				// loop through all meshes in 3DStudio
	 {
	    mesh3ds *mesh = NULL;
		GetMeshByIndex3ds(levelFile3ds->GetDatabase(), index, &mesh);
		PRINT_ERRORS_EXIT(stderr);

		cstats << "processing mesh " << mesh->name << endl << "At " <<
			mesh->locmatrix[9] << "," <<
			mesh->locmatrix[10] << "," <<
			mesh->locmatrix[11] << endl;

		Get3DStudioMesh(levelFile3ds,mesh);
	 }
}
#endif

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(int argc, char** argv)
{
    int index;
	for ( index=1; argv[index] && (*argv[index] == '-'); ++index )
	 {
		switch(*(argv[index]+1))
		 {
#if 0
			case 'l':
			case 'L':
				DumpPrjFile(argv[index]+2);
				exit(0);
				break;
#endif
			case 'p':
			case 'P':
				RedirectStandardStream(argv[index]+2);
				break;

			default:
				cerror << "OADDump Error: Unrecognized command line switch \"" <<
					*(argv[index]+1) << "\"" << endl;
				break;
		 }
	 }

	if(argc-(index-1) < 2)
	  {
		char* cp;
//         cp = strrchr( argv[0], '\\');
//         cp++;
//		cout << "Usage : " << ((cp) ? cp : argv[0]) << " {switches} <.OAD file name> {<output name>}" << endl;
		cout << "Usage : " << argv[0] << " {switches} <.OAD file name> {<output name>}" << endl;
		cout << "Switches:" << endl;
		cout << "    -l<3D Studio .prj file>   Dumps all oad data from .prj file" << endl;
		cout << "    -p<stream initial><stream output>, where:" << endl;
		cout << "        <stream initial> can be any of:" << endl;
		cout << "            w=warnings (defaults to standard err)" << endl;
		cout << "            e=errors (defaults to standard err)" << endl;
		cout << "            f=fatal (defaults to standard err)" << endl;
		cout << "            s=statistics (defaults to null)" << endl;
		cout << "            p=progress (defaults to null)" << endl;
		cout << "            d=debugging  (defaults to null)" << endl;
		cout << "        <stream output> can be any of:" << endl;
		cout << "            n=null, no output is produced" << endl;
		cout << "            s=standard out" << endl;
		cout << "            e=standard err" << endl;
		cout << "            f<filename>=output to filename" << endl;
		exit(1);
	  }
	return(index);
}

//============================================================================

int
main(int argc,char *argv[])
{
	cout << "OADDump V0.18 Copyright 1995,1996,1997,98,9 World Foundry Group.";
	cout << "By Kevin T. Seghetti" << endl;
	int index = ParseCommandLine(argc,argv);

	QObjectAttributeData oad;
	cprogress << "Loading .OAD file <" << argv[index+0] << ">" << endl;

	ifstream file(argv[index+0],ios::in|ios::binary);
	if(file.rdstate() != ios::goodbit)
	 {
		cerror << "OADDump Error: input OAD file <" << argv[index+0] << "> not found." << endl;
		exit(1);
	 }

	oad.Load(file,cerror);				// heres where the work occurs


	ostream* out;
	if((argc - index) > 1)
	 {
		cprogress << "Writing output file <" << argv[index+1] << ">"  << endl;
		out = new ofstream(argv[index+1]);
		if(!out)
		 {
			cerr << "Unable to open output file " << argv[index+1] << endl;
			exit(1);
		 }
	 }
	else
		out = &cout;
	(*out) << oad;

    if(out != &cout)
	    delete out;						
	cprogress << "OadDump Done" << endl;
    return 0;
}

//============================================================================
