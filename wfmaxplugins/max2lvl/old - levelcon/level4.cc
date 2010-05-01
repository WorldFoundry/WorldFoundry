//============================================================================
// level4.cc
//============================================================================
//
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

#include <iostream.h>
#include <fstream.h>
#include "levelcon.hp"
#include "level.hp"
#include "asset.hp"
#include <stl/algo.h>
#include <pclib/hdump.hp>
#include "file3ds.hp"
#include <direct.h>
#include <stl/bstring.h>
#include <process.h>
#include "symbol.hp"
#include "template.hp"

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//extern const int OADBUFFER_SIZE;		// Now in level.hp
extern char oadBuffer[];
extern int parseStringIntoParts(  const char* _string, char* _xDataParameters[], int nMaxParameters );
extern int FindAndLoadMeshXDataChunk(QFile3ds* levelFile3ds,const char* meshName,const char* chunkName, char buffer[],int bufferSize,chunktag3ds chunkType);

//============================================================================

void
QLevel::DoCrossReferences(const string& inputFileName, const string& lcFileName, const string& outputFileName)				// handle references, and cameras etc.
{
	cprogress << "DoCrossReferences" << endl;

	CreateScarecrowFiles(lcFileName,inputFileName);

	CompileScripts(inputFileName,lcFileName);
	SortCameraBoxes();					// sort actboxca objects by nesting order
	SortIntoRooms();

	CreateMergedObjects(outputFileName);		// since this creates new objects, must run before create common data

	CreateAssetLST(outputFileName.c_str());
	PatchAssetIDsIntoOADs();

	// The following two functions MUST run after PatchAssetIDsIntoOADs().
	// Neither Kevin nor Phil can figure out why...
	AddXDataToCommonData();
	CreateCommonData();

	ResolvePathOffsets();

}

//============================================================================

const int MAX_LINELEN = 255;

void
QLevel::CreateScarecrowFiles(string lcFileName, string inputFileName)
{
	// read the template from the .lc file directory first

	char lcdrive[_MAX_DRIVE];
	char lcpath[_MAX_PATH];
	char lcresult[_MAX_PATH];

	_splitpath(lcFileName.c_str(),lcdrive,lcpath,NULL,NULL);
	_makepath(lcresult,lcdrive,lcpath,"scare","wrs");			// wrl source

	char indrive[_MAX_DRIVE];
	char inpath[_MAX_PATH];
	char inresult[_MAX_PATH];

	_splitpath(inputFileName.c_str(),indrive,inpath,NULL,NULL);

	ifstream templateFile(lcresult);
	if(templateFile.rdstate() != ios::goodbit)
	{
		cerror << "LevelCon Error: template file <" << lcresult << "> not found." << endl;
		exit(1);
	}

	char tempBuffer[MAX_LINELEN];
	string templateString;
	while(templateFile.good())
	{
		templateFile.getline(tempBuffer, MAX_LINELEN, '\n');
		templateString += tempBuffer;
		templateString += "\n";
	}

	DBSTREAM3(cdebug << "scarecrow file = " << templateString << endl; )

	// now loop through all objects, creating output files for each which is a scarecrow
	QObject *theObj;							// current object
	QObjectAttributeData *theOAD;					// OAD for current object
	QObjectAttributeDataEntry* td;				// OAD entry for current OAD
	for (int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	{
		DBSTREAM3( cdebug << "QLevel::CreateScarecrowFiles: examining object #" << objectIndex << endl; )
		theObj = objects[objectIndex];
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();

		QObjectAttributeDataEntry const * modelTypeEntry;
		modelTypeEntry = theOAD->GetEntryByName("Model Type");

		if(modelTypeEntry && modelTypeEntry->GetDef() == MODEL_TYPE_SCARECROW)
		{
			QObjectAttributeDataEntry const * meshNameEntry;
			meshNameEntry = theOAD->GetEntryByName("Mesh Name");
			if(!meshNameEntry)
			{
				cerror << "QObjectAttributeData::CreateScarecrowFiles:Cannot find .oad field named <" << "Mesh Name" << ">" << endl;
				exit(1);
			}
			DBSTREAM3( cdebug << "scarecrow name = " << meshNameEntry->GetString() << endl; )

			// make the name
			string outputName = indrive;
			outputName += inpath;
			outputName += objects[objectIndex]->GetName();
			outputName += ".wrl";
			DBSTREAM3( cdebug << "output file name = " << outputName << endl; )
			// now create a .wrl file for this scarecrow
			ofstream out(outputName.c_str());
			if(!out.good())
			{
				cerror << "LevelCon Error: unable to create output .wrl file <" << outputName << ">." << endl;
				exit(1);
			}
			vector<Symbol> tblSymbols;

			char tempBuffer[12];
			Symbol name( "FILENAME", meshNameEntry->GetString());
			tblSymbols.push_back(name);
			const QColBox& objColBox = theObj->GetColBox();
//			itoa(320,tempBuffer,10);
//			itoa(objColBox.GetMax().x()-objColBox.GetMin().x(),tempBuffer,10);
			sprintf(tempBuffer,"%f",BrScalarToFloat(objColBox.GetMax().y()-objColBox.GetMin().y()));
			Symbol sWidth( "WIDTH", string( tempBuffer ));
			tblSymbols.push_back( sWidth );
//			itoa(240,tempBuffer,10);
//			itoa(objColBox.GetMax().z()-objColBox.GetMin().z(),tempBuffer,10);
			sprintf(tempBuffer,"%f",BrScalarToFloat(objColBox.GetMax().z()-objColBox.GetMin().z()));
			Symbol sHeight( "HEIGHT", string( tempBuffer));
			tblSymbols.push_back( sHeight );

			string result = _template( templateString, tblSymbols );
			out << result;

			// ok, now change the oad to contain a model type of mesh (from scarecrow)
			QObjectAttributeDataEntry* writableModelTypeEntry = (QObjectAttributeDataEntry*)modelTypeEntry;
			writableModelTypeEntry->SetDef(MODEL_TYPE_MESH);

			QObjectAttributeDataEntry* writableMeshNameEntry = (QObjectAttributeDataEntry*)meshNameEntry;
			string assetName = indrive;
			assetName += inpath;
			assetName += objects[objectIndex]->GetName();
			assetName += ".3ds";

			writableMeshNameEntry->SetString(assetName);
		}
	}
}

//============================================================================

const int MAX_XDATA_PARAMETERS= 50;
const int32 XDataSourceField = 0;
const int32 XDataProgField = 1;
const int32 XDataDestField = 2;

void
QLevel::CompileScripts(const string& inputFileName, const string& lcFileName)
{
	DBSTREAM1 ( cprogress << "Compile Scripts" << endl; )
	char* _xDataParameters[MAX_XDATA_PARAMETERS];

	char cwd[_MAX_PATH];
	getcwd(cwd,_MAX_PATH);
	DBSTREAM2(cdebug << "CompileScripts: cwd = " << cwd << endl; )

	char sourceDir[_MAX_PATH];
	assert(inputFileName.length() < _MAX_PATH);
	strcpy(sourceDir,inputFileName.c_str());
	char* end = strrchr(sourceDir,'\\');
	if(end)
	 {
		*end = 0;
		DBSTREAM2(cdebug << "CompileScripts: changing to directory " << sourceDir << endl; )
		chdir(sourceDir);
	 }

	{	// write out the object id list
		ofstream objIDOut("objects.id");
		if(objIDOut.rdstate() != ios::goodbit)
		{
			cerror << "Levelcon Error: cannot create file objects.id" << endl;
			exit(1);
		}
		PrintNameList(objIDOut);
	}

	char* pEnvVelocityDir = getenv( "VELOCITY_DIR" );
	if(!pEnvVelocityDir)
	 {
		cerror << "VELOCITY_DIR environment variable not set!" << endl;
		exit(1);
	 }

	QObjectAttributeData *theOAD;					// OAD for current object
	QObjectAttributeDataEntry* td;				// OAD entry for current OAD
	for (int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	 {
		DBSTREAM3( cdebug << "QLevel::CompileScripts: examining object #" << objectIndex << endl; )
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();
		for(int entryIndex=0;entryIndex < theOAD->entries.size(); entryIndex++)		// walk entries in this OAD and add to common block as needed
		 {
			td = &theOAD->entries[entryIndex];

			if(td->GetType() == BUTTON_XDATA && (td->GetXDataConversionAction() == XDATA_SCRIPT))						// if def > 0 then write out this xdata, if present
			{
				DBSTREAM3( cdebug << "QLevel::CompileScripts: found xdata reference named <" << td->GetName() << "> with string <" << td->GetString() << ">" << endl; )

				parseStringIntoParts(td->GetString().c_str(),_xDataParameters,MAX_XDATA_PARAMETERS);

				if( td->GetString().find("Cave Logic Studios AI") != NPOS)
//				if( td->GetString() == "Cave Logic Studios AI [Script]|")
//				if(*_xDataParameters[1])
				{

					DBSTREAM3( cdebug << "QLevel::CompileScripts: could have script" << endl; )
					AssertMsg(_xDataParameters[XDataSourceField],"CompileScriptsData: string is <" << td->GetString() << ">");
					assert(*_xDataParameters[XDataSourceField]);
					DBSTREAM3( cdebug << "CompileScriptsData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

					// load data into oadbuffer
					int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataSourceField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);

					if(xdataSize == -1 && td->GetXDataRequired())
			 	 	{
						cerror << "LevelCon Error: Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">" << endl;
						exit(1);
		 	 	 	}
//					assert(xdataSize != -1);
					if(xdataSize != -1)
					{
					// found script, lets compile it
						DBSTREAM3( cdebug << "QLevel::CompileScripts: found script" << endl; )
						{
							ofstream out("xdata.ai",ios::out|ios::binary);
							if(out.rdstate() != ios::goodbit)
		 					{
								cerror << "Levelcon Error: cannot create file xdata.ai" << endl;
								exit(1);
		 					}
							DBSTREAM3( cdebug << "CompileScripts: xdata size is <" << xdataSize << ">" << endl; )

    						out.write(oadBuffer, xdataSize);
						}

						int error;
						char* szCommand = getenv( "COMSPEC" );
						assert( szCommand );

//						string platformString = "-dPLATFORM_";
//						platformString += targetNames[levelconTargetSystem];
//						string _preprocessorProgram = pEnvVelocityDir;
//						_preprocessorProgram += "\\bin\\";
//						_preprocessorProgram += "prep";
//						DBSTREAM2(cdebug << "CompileScripts: called program " << _preprocessorProgram << endl; )
//						error = spawnl( P_WAIT, _preprocessorProgram.c_str(), platformString,"xdata.ai", "xdata.pp", NULL );
//						if(error)
//						 {
//							cerror << "Levelcon Error: script preprocessor error \nobject <" << objects[objectIndex]->GetName() << ">\noad field named <" << td->GetName() << ">" << endl;
//							exit(1);
//						 }
//						AssertMsg(!error,"error =" << error);

						string _converterProgram(pEnvVelocityDir);
						_converterProgram += "\\bin\\";
//						_converterProgram += _xDataParameters[1];			// kts for now hard code it
						_converterProgram += "aicomp2";
						DBSTREAM2(cdebug << "CompileScripts: called program " << _converterProgram << endl; )
//						error = spawnl( P_WAIT, szCommand, szCommand, "/c",
//							_converterProgram, "-q", "xdata.ai", NULL );

//						error = spawnl( P_WAIT, _converterProgram.c_str(), "-q","xdata.pp", NULL );
						error = spawnl( P_WAIT, _converterProgram.c_str(), "-q","xdata.ai", NULL );
						if(error)
						 {
							cerror << "Levelcon Error: bad ai script in \nobject <" << objects[objectIndex]->GetName() << ">\noad field named <" << td->GetName() << ">\n on line " << error << endl;
							exit(1);
						 }
						AssertMsg(!error,"error =" << error);

						ifstream in("xdata.aib",ios::in|ios::binary);
						if(in.rdstate() != ios::goodbit)
		 				{
							cerror << "Levelcon Error: cannot open file xdata.aib" << endl;
							exit(1);
		 				}

						in.seekg( 0, ios::end );
						int inLength = in.tellg();
						in.seekg( 0, ios::beg );
						in.read((char*)&oadBuffer, inLength);

						DBSTREAM3( cdebug << "CompileScripts: output size is <" << inLength << "> for object named " << objects[objectIndex]->GetName() << endl; )
						td->SetCompiledScript(oadBuffer,inLength);
					}
				}

				// free all strings allocated by parseStringIntoParts
				for ( int paramIndex=0; paramIndex < MAX_XDATA_PARAMETERS; ++paramIndex )
				 {
					if ( _xDataParameters[ paramIndex ] )
						free( _xDataParameters[ paramIndex ] ), _xDataParameters[ paramIndex ] = NULL;
				 }

			}
		 }
	 }

#if 0
	int error = 0;
	char _converterProgram[] = "aicomp";
//	char _converterProgram[_MAX_PATH];
//	sprintf( _converterProgram, "%s\\bin\\%s", theDialogBox->szVelocityDir, _xDataParameters[1] );
	char* szCommand = getenv( "COMSPEC" );
	assert( szCommand );
	error = spawnl( P_WAIT, szCommand, szCommand, "/c",
		_converterProgram, "-q", "xdata.ai", NULL );
	AssertMsg(!error,"error =" << error);
//#else
	char sourceDir[_MAX_PATH];
	assert(strlen(inputFileName) < _MAX_PATH);
	strcpy(sourceDir,inputFileName);
	char* end = strrchr(sourceDir,'\\');
	if(end)
	{
		*end = 0;
		ExecProgram("c:\\command.com","-c dir",sourceDir);
	}
	else
	{
		ExecProgram("c:\\command.com","-c dir",NULL);
	}
#endif
	chdir(cwd);
}

//============================================================================

void
QLevel::PrintObjectList(ostream& output) const
{
	for(int index=0;index<objects.size();index++)
		output << "  " << objects[index]->GetName() << endl;
}

//============================================================================

void
QLevel::LoadAssetFile(const char* lcFileName)
{
// now load all of the .oad files for all of the objects
	DBSTREAM3( cprogress << "Loading assets.txt file" << endl; )

	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(lcFileName,drive,path,NULL,NULL);
	_makepath(result,drive,path,NULL,NULL);

	string line(result);
	line += "assets.txt";

	ifstream input(line.c_str(),ios::in);
	if(input.rdstate() != ios::goodbit)
	{
		cerror << "LevelCon Error: problem reading file <assets.txt>, Probably not found" << endl;
		exit(1);
	}
	assetExts.Load(input);
}

//============================================================================
// note: LoadLCFile must have been called first
void
QLevel::LoadOADFiles(const char* lcFileName)
{
// now load all of the .oad files for all of the objects
	DBSTREAM1( cprogress << "Loading .OAD files" << endl; )

	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(lcFileName,drive,path,NULL,NULL);
	_makepath(result,drive,path,NULL,NULL);

	objectOADs.push_back(QObjectAttributeData());						// create 0 entry, which is never used
	string line;

	for(int i=1;i<objectTypes.size();i++)		// note: this starts at one since 0 is the null object
	 {
		line = result;
		line += objectTypes[i];
		line += ".oad";
		DBSTREAM3( cdebug << "QLevel::LoadOADFiles: Line = <" << line << ">" << endl; )

		ifstream input(line.c_str(),ios::in|ios::binary);
		if(input.rdstate() != ios::goodbit)
		 {
			cerror << "LevelCon Error: problem reading .OAD file <" << line << ">, Probably not found" << endl;
			exit(1);

		 }
		QObjectAttributeData oad;
		if(oad.Load(input,cerror))
		{
			objectOADs.push_back(oad);
		}
		else
			exit(1);						// since oad.Load will print whatever the error is
	 }
}

//============================================================================
#define MAX_DOT_LC_LINELEN 160

void
QLevel::LoadLCFile(const char* name)
{
	ifstream file(name);
	if(file.rdstate() != ios::goodbit)
	{
		cerror << "LevelCon Error: LC file <" << name << "> not found." << endl;
		exit(1);
	}
// ok, verify it is an .lc file

	char tempBuffer[MAX_DOT_LC_LINELEN];

	file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
	string lineBuffer(tempBuffer);
	if(lineBuffer.find("Objects.lc") == NPOS)
	{
		cerror << "LevelCon Error: <" << name << "> is not a .LC file" << endl;
		exit(1);
	}

// look for opening {
	while(lineBuffer.find("{") == NPOS)
	{
		file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
		lineBuffer = tempBuffer;

		if(file.rdstate() != ios::goodbit)
		{
			cerror << "LevelCon Error: problem parsing .LC file" << endl;
			exit(1);
		}
	}
	file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
	lineBuffer = tempBuffer;

// ok, now create in memory enumeration of all of the game objects
	while(lineBuffer.find("}") == NPOS)
	{
		if(lineBuffer.length())
		{
			int charIdx;
			strcpy(tempBuffer, lineBuffer.c_str());
			for (charIdx=0; (tempBuffer[charIdx] == '\t' || tempBuffer[charIdx] == ' '); charIdx++) ;
			objectTypes.push_back(string(strlwr(&tempBuffer[charIdx])));
			DBSTREAM3( cdebug << "QLevel::LoadLCFile: Inserting object type <" << strlwr(&tempBuffer[charIdx]) << ">" << endl; )
		}
		file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
		lineBuffer = tempBuffer;

		if(file.rdstate() != ios::goodbit)
		{
			cerror << "LevelCon Error: problem parsing .LC file" << endl;
			exit(1);
		}
	}

// now print in memory enumeration
	DBSTREAM3
	(
		cstats << "Game Object Enumeration" << endl;
		for(int i=0;i<objectTypes.size();i++)
		{
			cstats << "Object Index <" << i  << ">, Name <" << objectTypes[i] << ">" << endl;
		}
	)

	assert(objectTypes[0] == string("nullobject"));
}

//============================================================================

int16
QLevel::GetObjectIndex(const QObject* object) const		// return index of object in array, used to create other object lists, which refer to this one
{
	for(int index=0;index < objects.size();index++)
	{
		if(objects[index] == object)
			return(index);
	}
	assert(0);
	return(-1);
}

//============================================================================
// go through all objects, writing out all related xdata chunks (AI and object lists)

//#define OBJECTLISTTAG "Cave Logic Studios Object List"
const int32 OBJECTLIST_MAXENTRIES = 50;
const int32 CONTEXTUALANIMATIONLIST_MAXENTRIES = 50;

void
QLevel::AddXDataToCommonData(void)
{
	char* _xDataParameters[MAX_XDATA_PARAMETERS];

	DBSTREAM1(cprogress << "AddXDataToCommonData" << endl; )

	QObjectAttributeData *theOAD;					// OAD for current object
	QObjectAttributeDataEntry* td;				// OAD entry for current OAD
	for (int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	 {
		DBSTREAM3( cdebug << "QLevel::AddXDataToCommonData: examining object #" << objectIndex << " named " << objects[objectIndex]->GetName() << endl; )
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();
		for(int entryIndex=0;entryIndex < theOAD->entries.size(); entryIndex++)		// walk entries in this OAD and add to common block as needed
		 {
			td = &theOAD->entries[entryIndex];

			if(td->GetType() == BUTTON_XDATA)						// if def > 0 then write out this xdata, if present
			{
				DBSTREAM3( cdebug << "QLevel::AddXDataToCommonData: found xdata reference named <" << td->GetName() << "> with string <" << td->GetString() << ">" << endl; )

				parseStringIntoParts(td->GetString().c_str(),_xDataParameters,MAX_XDATA_PARAMETERS);

				switch(td->GetXDataConversionAction())
				{
					case XDATA_CONTEXTUALANIMATIONLIST:
					{
						DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: Choose XDATA_CONTEXTUALANIMATIONLIST" << endl; )
						assert(_xDataParameters[XDataSourceField]);
						assert(*_xDataParameters[XDataSourceField]);
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

						// load data into oadbuffer
						int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataSourceField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);

						int32 contextAnimationArray[CONTEXTUALANIMATIONLIST_MAXENTRIES][2];
						int32 contextAnimationArrayEntries;
						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							cerror << "LevelCon Error: Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">" << endl;
							exit(1);
		 	 	 	 	}

						if(xdataSize != -1)
					 	{
							int32 roomNum = 0;
							// figure out which room object this is
							QObject thisObj = objects[objectIndex];
//							while ( strcmp( rooms[roomNum].GetName().c_str(), thisObj.GetName()) != 0 )
							while ( roomNum < rooms.size() && rooms[roomNum].InsideCheck(&thisObj) == 0 )
					  			roomNum++;

							assert(roomNum < rooms.size());

							DBSTREAM3( cdebug << "AddXDataToCommonData: found contextual animation list" << endl; )
							contextAnimationArrayEntries = ParseTextFileIntoContextualAnimationList(oadBuffer,xdataSize,contextAnimationArray,CONTEXTUALANIMATIONLIST_MAXENTRIES, roomNum);
							contextAnimationArray[contextAnimationArrayEntries][0] = 0L;
							int offset = AddCommonBlockData(contextAnimationArray,
															(contextAnimationArrayEntries * sizeof(int32) * 2) + sizeof(int32));
							td->SetDef(offset);					// store offset until written
							td->SetLen(contextAnimationArrayEntries * sizeof(int32) * 2 + sizeof(int32));
							DBSTREAM3( cdebug << "Added " << contextAnimationArrayEntries * 8 + 4 << " bytes of contextual animation list to commonblock at offset " << offset << endl;)
						}
						else
							td->SetDef(-1);		// if no xdata
						break;
					}
					case XDATA_OBJECTLIST:
				 	{
						DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: Choose XDATA_OBJECTLIST" << endl; )
						assert(_xDataParameters[XDataSourceField]);
						assert(*_xDataParameters[XDataSourceField]);
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

						// load data into oadbuffer
						int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataSourceField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);

						int32 objectReferenceArray[OBJECTLIST_MAXENTRIES];
						int32 objectArraySize = 0;
						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							cerror << "LevelCon Error: Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">" << endl;
							exit(1);
		 	 	 	 	}

						if(xdataSize != -1)
					 	{
							DBSTREAM3( cdebug << "AddXDataToCommonData: found object list" << endl; )
							objectArraySize = ParseTextFileIntoObjectList(oadBuffer,xdataSize, objectReferenceArray,OBJECTLIST_MAXENTRIES);
					 	}
						objectReferenceArray[objectArraySize] = 0xffffffff;
						objectArraySize += 1;
						int offset = AddCommonBlockData(objectReferenceArray,objectArraySize*sizeof(int32));
						td->SetDef(offset);					// store offset until written
						break;
				 	}
					case XDATA_SCRIPT:
						DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: Choose XDATA_SCRIPT" << endl; )
						if(td->GetCompiledScript())
						{
							DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: found Compiled Script, length = " << td->GetCompiledScriptLength() << endl; )

							// load data into oadbuffer
							int xdataSize = td->GetCompiledScriptLength();
							assert(xdataSize);
							assert ((xdataSize % 4) == 0);	// check longword alignment

							DBSTREAM3( cdebug << "AddXDataToCommonData: found data:" << endl; )
							int offset = AddCommonBlockData(td->GetCompiledScript(),xdataSize);
							DBSTREAM3( cdebug << "wbn) xdata offset = " << offset << endl; )
							td->SetDef(offset);					// store offset until written
						}
						else
							td->SetDef(-1);
						break;
					case XDATA_COPY:
				 	{
						DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: Choose XDATA_COPY" << endl; )
						AssertMsg(_xDataParameters[XDataDestField],"AddXDataToCommonData: string is <" << td->GetString() << ">");
						assert(*_xDataParameters[XDataDestField]);
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataDestField] << ">" << endl; )

						// load data into oadbuffer
						int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataDestField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);

						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							cerror << "LevelCon Error: Object XData chunk named <" << _xDataParameters[XDataDestField] << "> not found in object <" << objects[objectIndex]->GetName() << ">" << endl;
							exit(1);
		 	 	 	 	}
//						assert(xdataSize != -1);

						if(xdataSize > 0)
						{
							DBSTREAM3( cdebug << "AddXDataToCommonData: found data:" << endl; )
							int offset = AddCommonBlockData(oadBuffer,xdataSize);
							DBSTREAM3( cdebug << "wbn) xdata offset = " << offset << endl; )
							td->SetDef(offset);					// store offset until written
						}
						else
						{
							DBSTREAM3( cdebug << "AddXDataToCommonData: data not found, setting to -1" << endl; )
							td->SetDef(-1);
						}
						break;
				 	}
					case XDATA_IGNORE :
						break;
					default:
						assert(0);
				}

				// free all strings allocated by parseStringIntoParts
				for ( int paramIndex=0; paramIndex < MAX_XDATA_PARAMETERS; ++paramIndex )
				 {
					if ( _xDataParameters[ paramIndex ] )
						free( _xDataParameters[ paramIndex ] ), _xDataParameters[ paramIndex ] = NULL;
				 }

			}
		 }
	 }
}

//============================================================================

int32
QLevel::ParseTextFileIntoObjectList(char* textBuffer, int32 sourceSize, long* objectReferenceArray,int32 maxEntries)
{
#if 0
	cout << "ParseTextFileIntoObjectList: sourceSize = " << sourceSize << endl;
	HDump(textBuffer,sourceSize,0);
#else
	int referenceCount = 0;
	char* stringPtr = textBuffer;
	char* endStringPtr = stringPtr;
	while(endStringPtr = strchr(stringPtr,'\r') )
	 {
		*endStringPtr = 0;
//		assert(strlen(stringPtr));

		if(*stringPtr)
		 {
			int32 objIndex = FindObjectIndex(stringPtr);
			DBSTREAM3( cdebug << "ParseTextFileIntoObjectList:: looking up object <" << stringPtr << ">, objIndex = <" << objIndex << ">" << endl; )
			if(objIndex < 0)
		 	{
				cerror << "LevelCon Error: object <" << stringPtr << "> refered to in object list not found" << endl;
				DBSTREAM3( cdebug << "Object Name List:" << endl; )
				DBSTREAM3( PrintObjectList(cdebug); )
		 	}
			assert(referenceCount < maxEntries);
			objectReferenceArray[referenceCount] = objIndex;
			referenceCount++;
		 }
		assert(endStringPtr[1] == 0xa);
		stringPtr = endStringPtr + 2;
	 }
	return(referenceCount);
#endif
	return(0);
}

//============================================================================

int32
QLevel::ParseTextFileIntoContextualAnimationList(char* textBuffer,
												 int32 sourceSize,
												 long contextAnimArray[][2],
												 int32 maxEntries,
												 int32 roomNum)
{
	//cout << "ParseTextFileIntoContextualAnimationList: sourceSize = " << sourceSize << endl;
	//HDump(textBuffer,sourceSize,0);

	assert(sourceSize > strlen("a.bcd"));

	char* inputPtr = textBuffer;
	char filename[_MAX_PATH];
	int32 outputSize = 0;
	int arrayIndex = 0;

	while (sscanf(inputPtr, "%s", filename) == 1)
	{
		asset tempAsset = AssignUniqueAssetID(filename, roomNum);
		contextAnimArray[arrayIndex][0] = tempAsset.ID().ID();
		inputPtr += strlen(filename) + 1;	// Skip space between filenames
		if (sscanf(inputPtr, "%s", filename) == 1)
		{
			tempAsset = AssignUniqueAssetID(filename, roomNum);
			contextAnimArray[arrayIndex][1] = tempAsset.ID().ID();
			inputPtr += strlen(filename) + 2;	// Skip \r\n between filename pairs
			outputSize++;
		}
		arrayIndex++;
	}

	return outputSize;      // return number of long pairs written to the array
}

//============================================================================
// Level on disk format:
//		see levelcon.h in the velocity project, source directory
// kts clean this up, make a class which tracks all of the struct sizes

extern int32 numPF3s;

void
QLevel::SaveLVLFile(const char* name)
{
	DBSTREAM1(cprogress << "SaveLVLFile" << endl; )
	FILE* fp;
	int index;
	int32 offset;
	_LevelOnDisk diskLevel;

	numPF3s = pf3Index;		// Output to level file

	// copy level into disk level structure
	diskLevel.versionNum = LEVEL_VERSION;

	assert(numPF3s < INT16_MAX);				// make sure it will fit
	diskLevel.modelCount = int16(numPF3s);
	assert(objects.size() < INT16_MAX);			// make sure it will fit
	diskLevel.objectCount = int16(objects.size());
	assert(paths.size() < INT16_MAX);			// make sure it will fit
	diskLevel.pathCount = paths.size();
	assert(rooms.size() < MAX_ROOMS);			// make sure it will fit
	DBSTREAM3( cstats << "saving " << rooms.size() << " rooms" << endl; )
	diskLevel.roomCount = rooms.size();

	diskLevel.lightCount = 0;

	// calculate the offsets in the file of each major section
	offset = sizeof(_LevelOnDisk);				// offset in file to models
	DBSTREAM3( cstats << "SizeOf (levelondisk ) = " << sizeof(_LevelOnDisk) << endl; )
	assert(ALIGNED(offset));
	diskLevel.modelsOffset = offset;				// offset in file to models

	offset += (sizeof(_ModelOnDisk)*models.size());	// offset in file to objects
	DBSTREAM3( cstats << "SizeOf (modelOnDisk ) = " << sizeof(_ModelOnDisk) << endl; )
	assert(ALIGNED(offset));
	diskLevel.objectsOffset = offset;

	DBSTREAM1(cprogress << "SaveLVLFile: do objects offsets" << endl; )
	offset += (sizeof(int32)*objects.size());			// skip to end of offset list
	for(index=0;index<objects.size();index++)						// skip actual object offsets
		offset += objects[index]->SizeOfOnDisk();

	DBSTREAM3( cstats << "SizeOf (int32 ) = " << sizeof(int32) << endl; )
	DBSTREAM3( cstats << "SizeOf (objectOnDisk ) = " << sizeof(_ObjectOnDisk) << endl; )
	assert(ALIGNED(offset));
	diskLevel.pathsOffset = offset;

	DBSTREAM1(cprogress << "SaveLVLFile: do oaths offsets" << endl; )
	offset += (sizeof(int32)*paths.size());		// skip to end of offsets list
	for(index=0;index < paths.size();index++)
		offset += paths[index].SizeOfOnDisk();
	assert(ALIGNED(offset));
	diskLevel.roomsOffset = offset;

	DBSTREAM1(cprogress << "SaveLVLFile: do room offsets" << endl; )
	offset += (sizeof(int32)*rooms.size());		// skip to end of offsets list
	for(index=0;index < rooms.size();index++)
		offset += rooms[index].SizeOfOnDisk();

	assert(ALIGNED(offset));
	diskLevel.commonDataOffset = offset;			// start of common area

//	Moving this into DoCrossReferences()... -Phil
//	DBSTREAM3( cdebug << "QLevel::Save: creating COMMONBLOCK area" << endl; )
//	AddXDataToCommonData();
//	CreateCommonData();

	diskLevel.commonDataLength = commonAreaSize;

	// actually write the data out
	DBSTREAM1(cprogress << "SaveLVLFile: open file" << endl; )
	fp = fopen(name,"wb");
	if(!fp)
	 {
		cout << "error opening output file <" << name << ">" << endl;
		exit(5);
	 }
	assert(fp);
	fwrite(&diskLevel,sizeof(_LevelOnDisk),1,fp);
	DBSTREAM1(cprogress << "SaveLVLFile: wrote file header" << endl; )

	DBSTREAM3( cprogress << "Writting " << models.size() << " models" << endl; )
	assert(ftell(fp) == diskLevel.modelsOffset);
	for(index=0;index<models.size();index++)
		models[index].Save(fp);

	DBSTREAM3( cprogress << "Writting object offset array" << endl; )
	assert(ftell(fp) == diskLevel.objectsOffset);
	int32 tempOffset;
	tempOffset = diskLevel.objectsOffset + (sizeof(int32)*objects.size());
	for(index=0;index<objects.size();index++)
	 {
		DBSTREAM3( cdebug << "QLevel::Save: Writting object offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
		fwrite(&tempOffset,sizeof(int32),1,fp);
		tempOffset += objects[index]->SizeOfOnDisk();
	 }

	DBSTREAM3( cprogress << "Writting " << objects.size() << " objects" << endl; )
	for(index=0;index<objects.size();index++)
	 {
		DBSTREAM3( cdebug << "QLevel::Save: Writing object #" << index << ": " << *objects[index] << endl; )
		objects[index]->Save(fp,*this);
	 }

	DBSTREAM3( cprogress << "Writting path offset array" << endl; )
	assert(ftell(fp) == diskLevel.pathsOffset);
	tempOffset = diskLevel.pathsOffset + (sizeof(int32)*paths.size());
	for(index=0;index<paths.size();index++)
	 {
		DBSTREAM3( cdebug << "QLevel::Save: Writting path offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
		fwrite(&tempOffset,sizeof(int32),1,fp);
		tempOffset += paths[index].SizeOfOnDisk();
	 }

	DBSTREAM3( cprogress << "Writting " << paths.size() << " paths" << endl; )
	for(index=0;index<paths.size();index++)
		paths[index].Save(fp);

	DBSTREAM3( cprogress << "Writting room offset array" << endl; )
	assert(ftell(fp) == diskLevel.roomsOffset);
	tempOffset = diskLevel.roomsOffset + (sizeof(int32)*rooms.size());
	for(index=0;index<rooms.size();index++)
	 {
		DBSTREAM3( cdebug << "QLevel::Save: Writting room offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
		fwrite(&tempOffset,sizeof(int32),1,fp);
		tempOffset += rooms[index].SizeOfOnDisk();
	 }

	DBSTREAM3( cprogress << "Writting " << rooms.size() << " rooms" << endl; )
	for(index=0;index<rooms.size();index++)
	 {
		rooms[index].Save(fp,*this);
	 }

	DBSTREAM3( cprogress << "Writting " << diskLevel.commonDataLength << " bytes of common block data" << endl; )
	fwrite (commonArea, diskLevel.commonDataLength, 1, fp);
	DBSTREAM3( cprogress << "Finished writting level" << endl; )

	fclose(fp);
}

//============================================================================
// This method sorts camera activation box objects in order of nesting depth,
// so that "inside" boxes appear in the list last.  (This lets the game
// assume that the last box it finds(since they all run) is the one which has control.)

void								// kts changed 12-17-95 02:43pm
QLevel::SortCameraBoxes(void)
{
	DBSTREAM1( cprogress << "SortCameraBoxes:" << endl; )
	int check1Index=0, check2Index=0;	// object indices for comparison
	QObject *tempObjPtr;
	boolean didSwap = boolean::BOOLFALSE;


//	// kts test code
//	cdebug << "actboxca volume list" << endl;
//	for (int objIndex=0; objIndex < objects.length(); objIndex++)
//	 {
//		int oadIndex = objects[objIndex]->GetTypeIndex();
//		if (objectTypes[oadIndex] == "actboxca")
//			cdebug << "actboxca named <" << objects[objIndex]->GetName() << "> has a volume of <" << objects[objIndex]->GetColBox().GetVolume() << ">" << endl;
//	 }

	int loopCount = 0;
	do
	 {
		didSwap = boolean::BOOLFALSE;
		check1Index = 0;
		for (int objIndex=0; objIndex < objects.size(); objIndex++)
		{
			int oadIndex = objects[objIndex]->GetTypeIndex();
//			if (objectTypes[oadIndex] == "actboxca")
			if (objectTypes[oadIndex] == "actboxor")                // kts: now sorts Object Reference boxes
			{
				DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: Found an ActBoxCA at index <" << objIndex << "> named <" << objects[objIndex]->GetName() << ">" << endl; )
				if (check1Index == 0)
				 {
					check1Index = objIndex;
					DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: First object <" << objects[check1Index]->GetName() << "> stored" << endl; )
				 }
				else
				 {
					check2Index = objIndex;
					DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: Checking if object <" << objects[check1Index]->GetName() << "> is inside of object <" << objects[check2Index]->GetName() << ">" << endl; )
					// check for second box inside of first box
					QColBox camBox1 = objects[check1Index]->GetColBox();
					QColBox camBox2 = objects[check2Index]->GetColBox();

					if (camBox2.GetVolume() > camBox1.GetVolume())
					{
						// it is. swap objects, clear all indices
						DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: Swaping objects <" << check1Index << "> and <" << check2Index << ">" << endl; )
						DBSTREAM3( cdebug << "  Object 1: " << camBox1 << "  Object 2:" << camBox2 << endl; )
						PtrObj<QObject> tempPtrObj = objects[check2Index];
						objects[check2Index] = objects[check1Index];
						objects[check1Index] = tempPtrObj;

						didSwap = boolean::BOOLTRUE;
					}
					check1Index = check2Index;			// kts added 12-15-95 04:25pm
				 }
			}
		}
		loopCount++;
		DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: loopcount = <" << loopCount << ">" << endl; )
		assert(loopCount < 1000);
	 } while(didSwap);
}

//============================================================================
// This method add a piece of data to the commonblock, attempting to notice redundancies

int32
QLevel::AddCommonBlockData(const void* data, int32 size)
{
	assert(data);
	assert ((size % 4) == 0);	// check longword alignment
	assert( size > 0);

	int32 blockStart = commonAreaSize;							// remember where this block began

	// now grep the commonArea for a matching hunk
	int match = 1;
	for (int searchPtr=0; searchPtr <= (commonAreaSize - size) && match != 0; searchPtr+=4)
	{
		match = memcmp(&commonArea[searchPtr], data, size-1);
		if (match == 0)
		{
			blockStart = searchPtr;			// Found one, so plug in the pointer.
			DBSTREAM3( cdebug << "QLevel::AddCommonData: Making common ref at offset " << blockStart << endl; )
		}
	}

	if (match != 0)					// oh well, we will add to the end, so blockStart contains its offset
	{
		memcpy(&commonArea[commonAreaSize], data, size);
		DBSTREAM3( cdebug << "QLevel::AddCommonData: Inserting new block at offset " << blockStart << endl; )
		commonAreaSize += size;
                if ( !( commonAreaSize < MAXCOMMONSIZE ) )
                        cerr << "commonAreaSize=" << commonAreaSize << "  MAXCOMMONSIZE=" << MAXCOMMONSIZE << endl;
		assert (commonAreaSize < MAXCOMMONSIZE);
	}
	DBSTREAM3( cdebug << "QLevel::AddCommonData:  Done...Total common data size = " << commonAreaSize << endl << endl; )
	return (blockStart);
}

//============================================================================

void
QLevel::Load3DStudio(const char* name)
{
	DBSTREAM1( cprogress << "Loading 3D Studio project file <" << name << ">" << endl; )
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

		DBSTREAM3( cstats << "processing mesh " << mesh->name << endl << "At " << )
			DBSTREAM3( mesh->locmatrix[9] << "," << )
			DBSTREAM3( mesh->locmatrix[10] << "," << )
			DBSTREAM3( mesh->locmatrix[11] << endl; )

		Get3DStudioMesh(levelFile3ds,mesh);
	 }

	DBSTREAM3( cdebug << "model list: " << endl; )
	DBSTREAM3( copy (models.begin(), models.end(), ostream_iterator<QModel>(cdebug, "\n")); )
}

//============================================================================

int32
QLevel::GetClassIndex(const string& name) const
{
	int32 typeIndex = -1;

	vector<string>::const_iterator where;
	where = find(objectTypes.begin(),objectTypes.end(),name);

	if(where != objectTypes.end())
	 {
		typeIndex = where - objectTypes.begin();
		assert(typeIndex);							// zero is an invalid type index
	 }
	return(typeIndex);
}

//============================================================================
// create new model from name, or find existing model

int
QLevel::FindModel(const char* name)
{
	char modelName[QModel::NAME_LEN+1];
	char path[_MAX_PATH];
	char filename[_MAX_FNAME];
	char ext[_MAX_EXT];
	int modelIndex;

	_splitpath(name,NULL,path,filename,ext);
	if(
		!
		 (
			strcmp(ext,".3ds") == 0 ||
			strcmp(ext,".tga") == 0 ||
			strcmp(ext,".bmp") == 0
		 )
	  )
	 {
		cerror << "Levelcon Error: Model <" << name << "> is an invalid name" << endl;
		return(-1);
	 }
//	assert(!strcmp(ext,".3ds"));
	if(!strcmp(ext,".3ds"))
		strcpy(ext,".pf3");

	_makepath(modelName,NULL,path,filename,ext);

	assert(strlen(modelName) < QModel::NAME_LEN);
	QModel newModel(modelName);
	DBSTREAM3( cdebug << "Model will be loaded from file <" << modelName << ">" << endl; )

	vector<QModel>::iterator where = find(models.begin(),models.end(),newModel);

	if(where != models.end())
	 {
		modelIndex = where - models.begin();
	 }
	else
	 {
		models.push_back(newModel);
		modelIndex = models.size()-1;
	 }

	assert(modelIndex >= 0);
	return(modelIndex);
}

//============================================================================

int
QLevel::FindObjectIndex(const string& name) const
{
	for(int index=0;index<objects.size();index++)
	{
		QObject& foo = objects[index];
		if(name == foo.GetName())
			return(index);
	}
	return(0);					// kts changed from -1 10/30/96 8:10PM
}

//============================================================================