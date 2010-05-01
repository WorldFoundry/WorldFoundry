//============================================================================
// level4.cc
//============================================================================
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

#include "levelcon.hp"
#ifdef __WIN__
#include <direct.h>
#include <process.h>
#endif
#ifdef __LINUX__
#include <unistd.h>
#endif
#include "level.hp"
#include "asset.hp"
#include "hdump.hp"
#include "symbol.hp"
#include "template.hp"
#include "registry/registry.hp"

#if 0
class NullView: public View
{
public:
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};
#endif

//============================================================================

char oadBuffer[ OADBUFFER_SIZE ];
extern int parseStringIntoParts(  const char* _string, char* _xDataParameters[], int nMaxParameters );

//============================================================================

///extern Interface* gMaxInterface;

//============================================================================

void
QLevel::DoCrossReferences(const string& inputFileName, const string& lcFileName, const string& outputFileName)				// handle references, and cameras etc.
{
	DBSTREAM3( cprogress << "DoCrossReferences" << endl; )

//	CreateScarecrowFiles(inputFileName,lcFileName,outputFileName);

	CompileScripts(inputFileName,lcFileName);
	SortCameraBoxes();					// sort actboxca objects by nesting order
	SortIntoRooms();

//	CreateMergedObjects(outputFileName);		// since this creates new objects, must run before create common data

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

#if 0
void
QLevel::CreateScarecrowFiles(string inputFileName, string lcFileName, string outputFileName)
{
	// read the template from the .lc file directory first

	char lcdrive[_MAX_DRIVE];
	char lcpath[_MAX_PATH];
	char lcresult[_MAX_PATH];

	_splitpath(lcFileName.c_str(),lcdrive,lcpath,NULL,NULL);
	_makepath(lcresult,lcdrive,lcpath,"scare","wrs");			// wrl source

	char indrive[_MAX_DRIVE];
	char inpath[_MAX_PATH];

	_splitpath(inputFileName.c_str(),indrive,inpath,NULL,NULL);

	char outname[_MAX_FNAME];
	_splitpath(outputFileName.c_str(),NULL,NULL,outname,NULL);

	ifstream templateFile(lcresult);
	AssertMessageBox(templateFile.good(), "Template file <" << lcresult << "> not found.");

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
	for (unsigned int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	{
		theObj = objects[objectIndex];
		DBSTREAM3( cdebug << "QLevel::CreateScarecrowFiles: examining object #" << objectIndex << ":" << *theObj <<  endl; )
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();

		QObjectAttributeDataEntry const * modelTypeEntry;
		modelTypeEntry = theOAD->GetEntryByName("Model Type");

		if(modelTypeEntry && modelTypeEntry->GetDef() == MODEL_TYPE_SCARECROW)
		{
			QObjectAttributeDataEntry const * meshNameEntry;
			meshNameEntry = theOAD->GetEntryByName("Mesh Name");
			AssertMessageBox(meshNameEntry, "QObjectAttributeData::CreateScarecrowFiles():" << endl << "Cannot find .oad field named <Mesh Name>");
			DBSTREAM3( cdebug << "scarecrow name = " << meshNameEntry->GetString() << endl; )

			// make the name
			string outputName = indrive;
			outputName += inpath;
			outputName += objects[objectIndex]->GetName();
			outputName += ".wrl";
			DBSTREAM3( cdebug << "output file name = " << outputName << endl; )
			// now create a .wrl file for this scarecrow
			ofstream out(outputName.c_str());
			AssertMessageBox(out.good(), "Unable to create output .wrl file <" << outputName << ">.");
			vector<Symbol> tblSymbols;

			char tempBuffer[12];
			Symbol name( "FILENAME", meshNameEntry->GetString());
			tblSymbols.push_back(name);
			const QColBox& objColBox = theObj->GetColBox();
//			itoa(320,tempBuffer,10);
//			itoa(objColBox.GetMax().x()-objColBox.GetMin().x(),tempBuffer,10);
			sprintf(tempBuffer,"%f",WF_SCALAR_TO_FLOAT(objColBox.GetMax().y - objColBox.GetMin().y));
			string foo(tempBuffer);
			Symbol sWidth( "WIDTH", foo );
			tblSymbols.push_back( sWidth );
//			itoa(240,tempBuffer,10);
//			itoa(objColBox.GetMax().z()-objColBox.GetMin().z(),tempBuffer,10);
			sprintf(tempBuffer,"%f",WF_SCALAR_TO_FLOAT(objColBox.GetMax().z - objColBox.GetMin().z));
			foo = string(tempBuffer);
			Symbol sHeight( "HEIGHT", foo);
			tblSymbols.push_back( sHeight );

			string result = _template( templateString, tblSymbols );
			out << result;

			// ok, now change the oad to contain a model type of mesh (from scarecrow)
			QObjectAttributeDataEntry* writableModelTypeEntry = (QObjectAttributeDataEntry*)modelTypeEntry;
			writableModelTypeEntry->SetDef(MODEL_TYPE_MESH);

			QObjectAttributeDataEntry* writableMeshNameEntry = (QObjectAttributeDataEntry*)meshNameEntry;
			string assetName;
			// = indrive;
//			assetName += inpath;
			assetName += outname;
			assetName += '/';
			assetName += objects[objectIndex]->GetName();
			assetName += ".3ds";

			writableMeshNameEntry->SetString(assetName);
		}
	}
}
#endif

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
		AssertMessageBox(objIDOut.good(), "Cannot create file:" << endl << sourceDir << "\\objects.id");
		PrintNameList(objIDOut);
	}

	char worldFoundryDir[_MAX_PATH];
#if defined(__WIN__)
	if(!GetLocalMachineStringRegistryEntry("Software\\World Foundry\\GDK","WORLD_FOUNDRY_DIR",worldFoundryDir,_MAX_PATH))
		AssertMessageBox(0, "WORLD_FOUNDRY_DIR Registry Entry is missing");
#else
    char* wfDirString = getenv("WF_DIR");
    if(wfDirString)
    {
        strncpy(worldFoundryDir,wfDirString,_MAX_PATH);                                                                    
    }
    else
        AssertMsg(0,"WF_DIR environment variable not found\n");
#endif

	QObjectAttributeData* theOAD;					// OAD for current object
	QObjectAttributeDataEntry* td;				// OAD entry for current OAD
	for (unsigned int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	 {
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();
		DBSTREAM3( cdebug << "QLevel::CompileScripts: examining object #" << objectIndex << ":" << *objects[objectIndex] << endl; )
		for(unsigned int entryIndex=0;entryIndex < theOAD->entries.size(); entryIndex++)		// walk entries in this OAD and add to common block as needed
		 {
			td = &theOAD->entries[entryIndex];

			if(td->GetType() == BUTTON_XDATA && (td->GetXDataConversionAction() == XDATA_SCRIPT))						// if def > 0 then write out this xdata, if present
			{
				DBSTREAM3( cdebug << "QLevel::CompileScripts: found xdata reference named <" << td->GetName() << "> with string <" << td->GetString() << ">" << endl; )

				if (td->GetString().c_str())
					parseStringIntoParts(td->GetString().c_str(),_xDataParameters,MAX_XDATA_PARAMETERS);

				if( td->GetString().find("Cave Logic Studios AI") != string::npos)
				{
					DBSTREAM3( cdebug << "QLevel::CompileScripts: could have script" << endl; )
					AssertMessageBox(_xDataParameters[XDataSourceField],"CompileScriptsData: string is <" << td->GetString() << ">");
					assert(*_xDataParameters[XDataSourceField]);
					DBSTREAM3( cdebug << "CompileScriptsData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

					// load data into oadbuffer
					// Scripts are noticed much earlier (during OAD::Apply), and a pointer to the data
					// is stored in _scriptText.
					if(!td->GetScriptText() && td->GetXDataRequired())
			 	 	{
						AssertMessageBox(0, "Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">");
		 	 	 	}
					if(td->GetScriptText())
					{	// found script--compile it
						DBSTREAM3( cdebug << "QLevel::CompileScripts: found script" << endl; )
						{
							ofstream out("xdata.ai",ios::out|ios::binary);
							AssertMessageBox(out.good(), "Cannot create file xdata.ai");
							DBSTREAM3( cdebug << "CompileScripts: xdata size is <" << strlen(td->GetScriptText()) << ">" << endl; )
							assert(strlen(td->GetScriptText()));
    						out << td->GetScriptText();
						}

						int error;
						//char* szCommand = getenv( "COMSPEC" );
						//assert( szCommand );

						string _converterProgram(worldFoundryDir);
#if defined(__WIN__)
						_converterProgram += "\\bin\\";
						_converterProgram += "aicomp.exe";
						DBSTREAM2(cdebug << "CompileScripts: called program " << _converterProgram << endl; )
						error = spawnl( P_WAIT, _converterProgram.c_str(), "-q","xdata.ai", NULL );
						if(error)
							AssertMessageBox(0, "Bad script in object <" << objects[objectIndex]->GetName() << ">\noad field named <" << td->GetName() << ">\n on line " << error);
#else
						_converterProgram += "/bin/";
						_converterProgram += "aicomp";
						DBSTREAM2(cdebug << "CompileScripts: called program " << _converterProgram << endl; )
                        string command;
                        command = _converterProgram;
                        command += " -q xdata.ai";
                        error = system(command.c_str());
						if(error)
							AssertMessageBox(0, "Bad script in object <" << objects[objectIndex]->GetName() << ">\noad field named <" << td->GetName() << ">\n on line " << error);
#endif
						ifstream in("xdata.aib",ios::in|ios::binary);
						AssertMessageBox(in.good(), "Cannot open file xdata.aib");

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
            else
                if(td->GetType() == BUTTON_XDATA && (td->GetXDataConversionAction() == XDATA_COPY))						
                {
					if(!td->GetScriptText() && td->GetXDataRequired())
                        td->SetCompiledScript(td->GetScriptText(),strlen(td->GetScriptText()));
                }

		 }
	 }

	chdir(cwd);
}

//============================================================================

void
QLevel::PrintObjectList(ostream& output) const
{
	for(unsigned int index=0;index<objects.size();index++)
		output << "  " << objects[index]->GetName() << endl;
}

//============================================================================

void
QLevel::LoadAssetFile(const char* lcFileName)
{
// now load all of the .oad files for all of the objects
	DBSTREAM3( cprogress << "Loading assets.txt file" << endl; )

	char drive[_MAX_PATH];
	char path[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(lcFileName,drive,path,NULL,NULL);
	_makepath(result,drive,path,NULL,NULL);

	string line(result);
	line += "assets.txt";

	ifstream input(line.c_str(),ios::in);
	AssertMessageBox(input.good(), "Problem reading file <assets.txt>, Probably not found");
	assetExts.Load(input);
}

//============================================================================
// note: LoadLCFile must have been called first
void
QLevel::LoadOADFiles(const char* lcFileName)
{
// now load all of the .oad files for all of the objects
	DBSTREAM1( cprogress << "Loading .OAD files" << endl; )

	char drive[_MAX_PATH];
	char path[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(lcFileName,drive,path,NULL,NULL);
	_makepath(result,drive,path,NULL,NULL);

	objectOADs.push_back(QObjectAttributeData());						// create 0 entry, which is never used
	string line;

	for(unsigned int i=1;i<objectTypes.size();i++)		// note: this starts at one since 0 is the null object
	{
		line = result;
		line += objectTypes[i];

#if 0
		line += ".oad";
		DBSTREAM3( cdebug << "QLevel::LoadOADFiles: Line = <" << line << ">" << endl; )
		ifstream input(line.c_str(),ios::in|ios::binary);
		AssertMessageBox(input.good(), "Problem reading .OAD file <" << line << ">, Probably not found");
		QObjectAttributeData oad;
		if(oad.Load(input,cerror))
#else
		line += ".iff";
		DBSTREAM3( cdebug << "QLevel::LoadOADFiles: Line = <" << line << ">" << endl; )
		binistream input(line.c_str());
		AssertMessageBox(input.good(), "Problem reading .OAD file <" << line << ">, Probably not found");
		QObjectAttributeData oad;
		if(oad.LoadIFF(input,cerror))
#endif
		{
//             cout << "Oad dump:" << endl;
//             cout << oad << endl;
            oad.Validate();
			objectOADs.push_back(oad);
		}
		else
			AssertMessageBox(0, "Problem reading .OAD file <" << line << ">.");
	}
}

//============================================================================
#define MAX_DOT_LC_LINELEN 160

void
QLevel::LoadLCFile(const char* name)
{
	ifstream file(name);

	AssertMessageBox(file.good(), "Unable to open .LC file:" << endl << name);
// ok, verify it is an .lc file

	char tempBuffer[MAX_DOT_LC_LINELEN];

	file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
	string lineBuffer(tempBuffer);
	AssertMessageBox(lineBuffer.find("Objects.lc") != string::npos, name << endl << "is not a valid .LC file!");

// look for opening {
	while(lineBuffer.find("{") == string::npos)
	{
		file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
		lineBuffer = tempBuffer;

		AssertMessageBox(file.good(), "Problem parsing .LC file");
	}
	file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
	lineBuffer = tempBuffer;

// ok, now create in memory enumeration of all of the game objects
	while(lineBuffer.find("}") == string::npos)
	{
		if(lineBuffer.length())
		{
			int charIdx;
			strcpy(tempBuffer, lineBuffer.c_str());
			for (charIdx=0; (tempBuffer[charIdx] == '\t' || tempBuffer[charIdx] == ' '); charIdx++) 
                ;

			strlwr(&tempBuffer[charIdx]);
			objectTypes.push_back(string(&tempBuffer[charIdx]));

			DBSTREAM3( cdebug << "QLevel::LoadLCFile: Inserting object type <" << &tempBuffer[charIdx] << ">" << endl; )
		}
		file.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
		lineBuffer = tempBuffer;

		AssertMessageBox(file.good(), "Problem parsing .LC file");
	}

// now print in memory enumeration
	DBSTREAM3
	(
		cstats << "Game Object Enumeration" << endl;
		for(unsigned int i=0;i<objectTypes.size();i++)
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
	for(unsigned int index=0;index < objects.size();index++)
	{
		if(objects[index] == object)
			return(index);
	}
	assert(0);
	return(-1);
}

//============================================================================
// go through all objects, writing out all related xdata chunks (AI and object lists)

const int32 OBJECTLIST_MAXENTRIES = 50;
const int32 CONTEXTUALANIMATIONLIST_MAXENTRIES = 50;

void
QLevel::AddXDataToCommonData(void)
{
	char* _xDataParameters[MAX_XDATA_PARAMETERS];

	DBSTREAM1(cprogress << "AddXDataToCommonData" << endl; )

	QObjectAttributeData *theOAD;					// OAD for current object
	QObjectAttributeDataEntry* td;				// OAD entry for current OAD
	for (unsigned int objectIndex=0; objectIndex < objects.size(); objectIndex++)
	{
		DBSTREAM3( cdebug << "QLevel::AddXDataToCommonData: examining object #" << objectIndex << " named " << objects[objectIndex]->GetName() << endl; )
		theOAD = (QObjectAttributeData *)&objects[objectIndex]->GetOAD();
		for(unsigned int entryIndex=0;entryIndex < theOAD->entries.size(); entryIndex++)		// walk entries in this OAD and add to common block as needed
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
						DBSTREAM3( cdebug << "QLevel::AddXDataToXCommonData: Chose XDATA_CONTEXTUALANIMATIONLIST" << endl; )
						assert(_xDataParameters[XDataSourceField]);
						assert(*_xDataParameters[XDataSourceField]);
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

						// load data into oadbuffer
						//int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataSourceField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);
						int xdataSize = -1; // Waiting for AppData

						int32 contextAnimationArray[CONTEXTUALANIMATIONLIST_MAXENTRIES][2];
						int32 contextAnimationArrayEntries;
						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							cdebug << "LevelCon Error: Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">" << endl;
							AssertMessageBox(0, "Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">");
		 	 	 	 	}

						if(xdataSize != -1)
					 	{
							unsigned long roomNum = 0;
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
						AssertMsg(*_xDataParameters[XDataSourceField],"string = <" << td->GetString() << ">");
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataSourceField] << ">" << endl; )

						// load data into oadbuffer
						//int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataSourceField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);
						int xdataSize = -1;	// Waiting for AppData

						int32 objectReferenceArray[OBJECTLIST_MAXENTRIES];
						int32 objectArraySize = 0;
						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							AssertMessageBox(0, "Object XData chunk named <" << _xDataParameters[XDataSourceField] << "> not found in object <" << objects[objectIndex]->GetName() << ">");
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
						AssertMessageBox(_xDataParameters[XDataDestField],"AddXDataToCommonData: string is <" << td->GetString() << ">");
						assert(*_xDataParameters[XDataDestField]);
						DBSTREAM3( cdebug << "AddXDataToCommonData: xdata chunk name is <" << _xDataParameters[XDataDestField] << ">" << endl; )

						// load data into oadbuffer
						//int xdataSize = FindAndLoadMeshXDataChunk(levelFile3ds,objects[objectIndex]->GetName(),_xDataParameters[XDataDestField],oadBuffer,OADBUFFER_SIZE,XDATA_VOID);
						int xdataSize = -1;	// Waiting for AppData

						const char* script = td->GetScriptText();
						char* scriptBuffer = NULL;
						if(script)
						{
							xdataSize = ((strlen(script)+1) + 3) & ~3;  // add 1 so that 0 termination always gets copied		
						 	scriptBuffer = new char[xdataSize];
							assert(scriptBuffer);
							strncpy(scriptBuffer,script,xdataSize);
						}

#pragma message("Search for 'Waiting for AppData'")

						if(xdataSize == -1 && td->GetXDataRequired())
			 	 	 	{
							AssertMessageBox(0, "Object XData chunk named <" << _xDataParameters[XDataDestField] << "> not found in object <" << objects[objectIndex]->GetName() << ">");
		 	 	 	 	}

						if(xdataSize > 0)
						{
							DBSTREAM3( cdebug << "AddXDataToCommonData: found data:" << endl; )
							int offset = AddCommonBlockData(scriptBuffer,xdataSize);
							DBSTREAM3( cdebug << "wbn) xdata offset = " << offset << endl; )
							td->SetDef(offset);					// store offset until written
						}
						else
						{
							DBSTREAM3( cdebug << "AddXDataToCommonData: data not found, setting to -1" << endl; )
							td->SetDef(-1);
						}
						delete scriptBuffer;
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
	while((endStringPtr = strchr(stringPtr,'\r')) != 0 )
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

	assert((unsigned long)sourceSize > strlen("a.bcd"));

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

void
QLevel::SaveLVLFile(const char* name)
{
	DBSTREAM1(cprogress << "SaveLVLFile" << endl; )
//	FILE* fp;
	unsigned int index;
	int32 offset;
	_LevelOnDisk diskLevel;

	// copy level into disk level structure
	diskLevel.versionNum = LEVEL_VERSION;

	assert(objects.size() < INT16_MAX);
	diskLevel.objectCount = int16(objects.size());
	assert(paths.size() < INT16_MAX);
	diskLevel.pathCount = paths.size();
	assert(channels.size() < INT16_MAX);
	diskLevel.channelCount = int16(channels.size());
	assert(rooms.size() < MAX_ROOMS);
	DBSTREAM3( cstats << "saving " << rooms.size() << " rooms" << endl; )
	diskLevel.roomCount = rooms.size();

	diskLevel.lightCount = 0;
	diskLevel.lightsOffset = 0;

	// calculate the offsets in the file of each major section
	offset = sizeof(_LevelOnDisk);				// offset in file to objects
	DBSTREAM3( cstats << "SizeOf (levelondisk ) = " << sizeof(_LevelOnDisk) << endl; )
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

	DBSTREAM1(cprogress << "SaveLVLFile: do paths offsets" << endl; )
	offset += (sizeof(int32)*paths.size());		// skip to end of offsets list
	for(index=0;index < paths.size();index++)
		offset += paths[index].SizeOfOnDisk();
	assert(ALIGNED(offset));
	diskLevel.channelsOffset = offset;

	DBSTREAM1( cprogress << "SaveLVLFile: do channel offsets" << endl; )
	offset += (sizeof(int32)*channels.size());
	for(index=0; index < channels.size(); index++)
		offset += channels[index].SizeOfOnDisk();
	assert(ALIGNED(offset));
	diskLevel.roomsOffset = offset;

	DBSTREAM1(cprogress << "SaveLVLFile: do room offsets" << endl; )
	offset += (sizeof(int32)*rooms.size());		// skip to end of offsets list
	for(index=0;index < rooms.size();index++)
		offset += rooms[index].SizeOfOnDisk();

	assert(ALIGNED(offset));
	diskLevel.commonDataOffset = offset;			// start of common area

	diskLevel.commonDataLength = commonAreaSize;

	// actually write the data out
	DBSTREAM1(cprogress << "SaveLVLFile: open file" << endl; )
	ofstream lvl(name, ios::out | ios::binary);
	if(!lvl.good())
	{
		cout << "error opening output file <" << name << ">" << endl;
		exit(5);
	}
//	fwrite(&diskLevel,sizeof(_LevelOnDisk),1,fp);
	lvl.write( (const char*)&diskLevel, sizeof(_LevelOnDisk) );
	DBSTREAM1(cprogress << "SaveLVLFile: wrote file header" << endl; )

	DBSTREAM3( cprogress << "Writing object offset array" << endl; )
	assert(lvl.tellp() == streampos(diskLevel.objectsOffset));
	int32 tempOffset;
	tempOffset = diskLevel.objectsOffset + (sizeof(int32)*objects.size());
	for(index=0;index<objects.size();index++)
	{
		DBSTREAM3( cdebug << "QLevel::Save: Writing object offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
//		fwrite(&tempOffset,sizeof(int32),1,fp);
		lvl.write( (const char*)&tempOffset, sizeof(int32) );
		tempOffset += objects[index]->SizeOfOnDisk();
	}

	DBSTREAM3( cprogress << "Writing " << objects.size() << " objects" << endl; )
	for(index=0;index<objects.size();index++)
	{
		DBSTREAM3( cdebug << "QLevel::Save: Writing object #" << index << ": " << *objects[index] << endl; )
		objects[index]->Save(lvl,*this);
	}

	DBSTREAM3( cprogress << "Writing path offset array" << endl; )
	assert(lvl.tellp() == streampos(diskLevel.pathsOffset));
	tempOffset = diskLevel.pathsOffset + (sizeof(int32)*paths.size());
	for(index=0;index<paths.size();index++)
	{
		DBSTREAM3( cdebug << "QLevel::Save: Writing path offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
//		fwrite(&tempOffset,sizeof(int32),1,fp);
		lvl.write( (const char*)&tempOffset, sizeof(int32) );
		tempOffset += paths[index].SizeOfOnDisk();
	}

	DBSTREAM3( cprogress << "Writing " << paths.size() << " paths" << endl; )
	for(index=0;index<paths.size();index++)
		paths[index].Save(lvl);

	DBSTREAM3( cprogress << "Writing channel offset array" << endl; )
	assert(lvl.tellp() == streampos(diskLevel.channelsOffset));
	tempOffset = diskLevel.channelsOffset + (sizeof(int32)*channels.size());
	for(index=0;index<channels.size();index++)
	{
		DBSTREAM3( cdebug << "QLevel::Save: Writing channel offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
//		fwrite(&tempOffset,sizeof(int32),1,fp);
		lvl.write( (const char*)&tempOffset, sizeof(int32) );
		tempOffset += channels[index].SizeOfOnDisk();
	}

	DBSTREAM3( cprogress << "Writing " << channels.size() << " channel(s)" << endl; )
	for(index=0;index<channels.size();index++)
		channels[index].Save(lvl);


	DBSTREAM3( cprogress << "Writing room offset array" << endl; )
	assert(lvl.tellp() == streampos(diskLevel.roomsOffset));
	tempOffset = diskLevel.roomsOffset + (sizeof(int32)*rooms.size());
	for(index=0;index<rooms.size();index++)
	{
		DBSTREAM3( cdebug << "QLevel::Save: Writing room offset #" << index << " offset of <" << tempOffset << "> to disk" << endl; )
		assert(ALIGNED(tempOffset));
//		fwrite(&tempOffset,sizeof(int32),1,fp);
		lvl.write( (const char*)&tempOffset, sizeof(int32) );
		tempOffset += rooms[index].SizeOfOnDisk();
	}

	DBSTREAM3( cprogress << "Writing " << rooms.size() << " rooms" << endl; )
	for(index=0;index<rooms.size();index++)
		rooms[index].Save(lvl,*this);

	DBSTREAM3( cprogress << "Writing " << diskLevel.commonDataLength << " bytes of common block data" << endl; )
//	fwrite (commonArea, diskLevel.commonDataLength, 1, fp);
	lvl.write( (const char*)commonArea, diskLevel.commonDataLength );
	DBSTREAM3( cprogress << "Finished writing level" << endl; )

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
	bool didSwap = false;

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
		didSwap = false;
		check1Index = 0;
		for (unsigned int objIndex=0; objIndex < objects.size(); objIndex++)
		{
			int oadIndex = objects[objIndex]->GetTypeIndex();
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

						didSwap = true;
					}
					check1Index = check2Index;			// kts added 12-15-95 04:25pm
				}
			}
		}
		loopCount++;
		DBSTREAM3( cdebug << "QLevel::SortCameraBoxes: loopcount = <" << loopCount << ">" << endl; )
		assert(loopCount < 1000);
	}
	while ( didSwap );
}

//============================================================================
// This method add a piece of data to the commonblock, attempting to notice redundancies

int32
QLevel::AddCommonBlockData(const void* data, int32 size)
{
	assert(data);
	assert ((size % 4) == 0);	// check longword alignment
	assert( size > 0);

	DBSTREAM3( cdebug << "QLevel::AddCommonData called with data size of " << size << "\nData:\n"; )
	DBSTREAM3( HDump(data,size,1,"\t",cdebug); )

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
	return blockStart;
}

//============================================================================

int32
QLevel::GetClassIndex( const string& name ) const
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

int
QLevel::FindObjectIndex(const string& name) const
{
	for(unsigned int index=0;index<objects.size();index++)
	{
		QObject& foo = objects[index];
		if(name == foo.GetName())
			return(index);
	}
	return 0;
}

//============================================================================
