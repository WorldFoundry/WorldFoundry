//==============================================================================
// attribedit.cc: main for attribute editor
//==============================================================================

#include <hal/hal.h>
#include <math/vector3.hp>
#include <memory/realmalloc.hp>
#include <iffwrite/iffwrite.hp>
#include <ini/profile.hp>
                  
#if defined(USEXML)
#include <xmlfile.h>
#include <xmlinput.h>
#include <xmlstream.h>
#include <xmloutput.h>
#endif
                    
#include "parsetreenode.hp"
#include "oadesciffbinparser.hp"
#include "gladexmlgenerator.hp"
#include "attribedit.hp"
#include "gui.hp"
#include "aegui.hp"
#include "iffhelpers.hp"

Memory* scratchMemory;
                        
//==============================================================================

void
AttributeEditor::Construct()
{
    parser = NULL;

    _wfConfigPath = getenv("HOME");
    _wfConfigPath += "/.worldfoundry/";
    _iniFilename = _wfConfigPath + "attribedit.ini";
    
    LoadLCFile();                               // read all class types
}


AttributeEditor::AttributeEditor()
{
    Construct();
    ClassName("disabled");
}

AttributeEditor::AttributeEditor(const std::string& overrideFile)
{
    Construct();
    LoadOverrideFile(overrideFile);
}

//==============================================================================
                         
void AttributeEditor::ClassName(const std::string& newName)
{
  Validate();
  _className = newName;
  LoadOADFile(_className);
  Validate();
}

//==============================================================================

// kts this sucks, returning a string instance seems to crash

void                                                         
AttributeEditor::GetAttribEditProfileString(const std::string& section, const std::string& key, std::string& result) const
{
    char charresult[1024];
    result[0] = 0;

    std::string filename = WFConfigPath();
    filename += "/attribedit.ini";

    int retVal = GetPrivateProfileString(section.c_str(), key.c_str(), "default", charresult, 1024, filename.c_str() ); 
    AssertMsg(retVal != -1,"Unable to load string <" << key << "> from section <" << section << "> in ini file <" << filename << ">");
    result = &charresult[0];
}

//==============================================================================



void
AttributeEditor::LoadLCFile()
{
    std::string oadDir;
    GetAttribEditProfileString("dirs","oaddir",oadDir);
    //cout << "oadDir is set to " << oadDir << std::endl;

    std::string lcFilename(oadDir);
    lcFilename += "/objects.lc";

    std::ifstream lcFile(lcFilename.c_str());

    AssertMsg(lcFile.good(),"failed to open lc file named <" << lcFilename << ">");

    bool started=false;

    while(!lcFile.eof())
    {
        
        std::string line;
        getline(lcFile,line);

        //cout << " line [" << line << "]" << std::endl;

        if(line[0] == '}')
            break;              // all done
        // strip out leading whitespace
        while(line.size() && (line[0] == ' ' || line[0] == '\t'))
            line.erase(0,1);

        if(started)
        {
            if(line.size())
            {
                _oadClassList.push_back(line);
                //cout << "adding line [" << line << "]" << std::endl;
            }
        }
        if(line[0] == '{')
            started=true;

    }
}

//==============================================================================

void
AttributeEditor::LoadOADFile(const std::string& className)
{

   std::string oadDir;
    GetAttribEditProfileString("dirs", "oaddir", oadDir);
    oadDir  += "/";

    std::string filename(oadDir);
    filename += className;
    filename += ".iff";

  //  ifstream oadFile(filename.c_str());
  //  assert(oadFile.good());

    if(parser)
        delete parser;
    parser = new OADescIffBinParser;
    parser->parseBinaryOADescFile(filename);
}

//==============================================================================

binistream*
OpenIFFFile(const std::string& filename, const std::string& commandName)
{   
    std::string finalFileName = filename;
    char byte;
    {
        binistream iffFile(filename.c_str());
        if(!iffFile.good())
            return NULL;

        iffFile >> byte;
    }

    if(byte == '{' || byte == 0xa)
    {
        // text file, needs to be compiled

        std::string tempFileName = tmpnam(NULL);

        std::string command;
        command += commandName;
        command += " -o=";
        command += tempFileName;
        command += " ";
        command += filename;
//        command += " >errorfile";

        //cout << "executing " << command << std::endl;
        int result = system(command.c_str());
        assert(result == 0);

//
//         if( errorfile has errors)
//         {
//             return NULL;
//         }
        finalFileName = tempFileName;
    }
    else
    {
        // binary file, just create and return
    }

    binistream* finalFile = new binistream(finalFileName.c_str());

    assert(finalFile);
    assert(finalFile->good());
    return finalFile;
}

//==============================================================================

#if defined(USEXML)


static void sObjectElementHandler(XML::Element& elem, void *userData)
{
    assert(ValidPtr(userData));
    AttributeEditor* ae = static_cast<AttributeEditor*>(userData);
    ae->XMLObjectElementHandler(elem);
}


void AttributeEditor::XMLObjectElementHandler(XML::Element &elem)
{
    cout << "Element name: " << elem.GetName() << std::endl;
     if (strcmp(elem.GetName(), "ClassName") == 0)
     {
         XML::Char tmp[100];
         size_t len = elem.ReadData(tmp, sizeof(tmp));
         tmp[len] = 0;
         assert(len);
         ClassName(tmp);          // this will cause the entire oad file to get loaded
     }
     else
     {
         assert(parser);
         TypeBase* entry = parser->LookupNodeByNoSpacesName(elem.GetName());
         AssertMsg(entry,"failed to look up entry named " << elem.GetName() << " loading override file");
         entry->ReadOverride(elem);
     }
}


static void sObjectHandler(XML::Element& elem, void *userData)
{
    assert(ValidPtr(userData));
    AttributeEditor* ae = static_cast<AttributeEditor*>(userData);
    ae->XMLObjectHandler(elem);
}


void AttributeEditor::XMLObjectHandler(XML::Element &elem)
{
    cout << "object name: " << elem.GetName() << std::endl;
	// set up initial handler for object
	XML::Handler handlers[] = {
		XML::Handler(NULL, ::sObjectElementHandler),
		XML::Handler::END
	};

	try {
		elem.Process(handlers, this);
	}
	catch (const XML::ParseException &e)
	{
		fprintf(stderr, "ERROR: %s (line %d, column %d)\n", e.What(), e.GetLine(), e.GetColumn());
	}
}

#endif

//==============================================================================

void 
AttributeEditor::LoadOverrideFile(const std::string& filename)
{

#if defined(USEXML)
	XML::FileInputStream file(filename.c_str());
	XML::Input input(file);

	// set up initial handler for object
	XML::Handler handlers[] = {
		XML::Handler("object", ::sObjectHandler),
		XML::Handler::END
	};

	try {
		input.Process(handlers, this);
	}
	catch (const XML::ParseException &e)
	{
		fprintf(stderr, "ERROR: %s (line %d, column %d)\n", e.What(), e.GetLine(), e.GetColumn());
	}

#else
    std::string command;
    GetAttribEditProfileString("programs", "iffcomp", command);

     binistream* pOverrideFile = OpenIFFFile(filename,command);
     if(!pOverrideFile)
     {
         std::cerr << "error opening override file <" << filename << ">" << std::endl;
         exit(2);
     }

//    binistream overrideFile(filename.c_str());
    binistream& overrideFile(*pOverrideFile);
    assert(overrideFile.good());

    //cout << "opening " << filename << " as override file" << std::endl;
    IFFChunkIter chunkIter(overrideFile);
    assert(chunkIter.GetChunkID().ID() == IFFTAG('O','B','J',0));
    std::string name = "";
    while(chunkIter.BytesLeft())
    {
        IFFChunkIter *childChunkIter = chunkIter.GetChunkIter(*scratchMemory);
        
        //cout << "Processing chunk " << childChunkIter->GetChunkID() << std::endl;
        switch(childChunkIter->GetChunkID().ID())
        {
            case IFFTAG('S','T','R',0):
            case IFFTAG('F','X','3','2'):
            case IFFTAG('I','3','2',0):
                {
                    while(childChunkIter->BytesLeft() > 0)
                    {
                        IFFChunkIter* typeChunkIter = childChunkIter->GetChunkIter(*scratchMemory);
                        //cout << "Processing type chunk " << typeChunkIter->GetChunkID() << std::endl;
                        switch(typeChunkIter->GetChunkID().ID())
                        {
                            case IFFTAG('N','A','M','E'):
                                {
                                    name = IFFReadString(*typeChunkIter);
                                    //cout << "name = " << name << std::endl;
                                }
                                break;
                            case IFFTAG('D','A','T','A'):
                                {
                                    if(childChunkIter->GetChunkID().ID() == IFFTAG('S','T','R',0))
                                    {
                                        assert(name.length());          // name chunk must come before str chunk

                                        if(name == "Class Name")
                                         {
                                            _className = IFFReadString(*typeChunkIter);
                                            ClassName(_className);          // this will cause the entire oad file to get loaded
                                        }
                                        else
                                        {
                                            //cout << "looking up node by name " << name << std::endl;
                                            assert(parser);
                                            TypeBase* entry = parser->LookupNodeByName(name);
                                            assert(entry);
                                            entry->ReadOverride(*typeChunkIter);
                                        }
                                    }
                                    //std::string data = IFFReadString(*typeChunkIter);
                                    //cout << "data = " << data << std::endl;
                                    break;
                                }
                            case IFFTAG('S','T','R',0):
                                {
                                    assert(name.length());          // name chunk must come before str chunk
//                                     std::string data = IFFReadString(*typeChunkIter);
//                                     cout << "string = " << data << std::endl;
                                    // now look up correct entry and apply override
                                    assert(parser);
                                    TypeBase* entry = parser->LookupNodeByName(name);
                                    assert(entry);
                                    entry->ReadOverride(*typeChunkIter);
                                    break;
                                }
                            default:
                                std::cerr << typeChunkIter->GetChunkID() << std::endl;
                                assert(!"unknown token encountered while parsing override file");
                                break;
                        }
                        MEMORY_DELETE(*scratchMemory,typeChunkIter,IFFChunkIter);
                    }
                    break;
                }
            default:
                {       
                    std::cout << childChunkIter->GetChunkID() << std::endl;
                    assert(!"unknown token encountered while parsing override file");
                }
        }
        MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
    }
    //delete pOverrideFile;
#endif
}

//==============================================================================

void 
AttributeEditor::SaveOverrideFile(const std::string& filename, bool binary, bool writeAll) const
{

#if defined(USEXML)
	// now open up the output file
    assert(binary == false);                    // there is no binary mode in XML
	XML::FileOutputStream file(filename.c_str());
    //AssertMsg(file.good(),"Failed to open override file named <" << filename << "> for writing");

    XML::Output out(file);
    out.BeginDocument();

	out.BeginElement("object", XML::Output::indent);
    	out.BeginElement("ClassName", XML::Output::terse);
            out << ClassName().c_str();
    	out.EndElement(XML::Output::terse);
        
        assert(parser);
        SaveOverrideFileRecurse(out,*parser->getParseTree(),writeAll);

	out.EndElement(XML::Output::indent);
    out.EndDocument();

#else
    std::ofstream out( filename.c_str(), binary ? std::ios::binary|std::ios::out : std::ios::out );
    AssertMsg(out.good(),"Failed to open override file named <" << filename << "> for writing");
    OADOutput* iffOut;
    if ( binary )
        iffOut = new IffWriterBinary( out );
    else
        iffOut = new IffWriterText( out );
    assert( iffOut );

	iffOut->enterChunk( ID( "OBJ" ) );
    
    	iffOut->enterChunk( ID( "STR" ) );
    		iffOut->enterChunk( ID( "NAME" ) );
    		    *iffOut << "Class Name";
    		iffOut->exitChunk();
    		iffOut->enterChunk( ID( "DATA" ) );
    			*iffOut << ClassName().c_str();
    		iffOut->exitChunk();
    	iffOut->exitChunk();
    
        assert(parser);
        SaveOverrideFileRecurse(*iffOut,*parser->getParseTree(),writeAll);
    iffOut->exitChunk();
    delete iffOut;
#endif
}

void
AttributeEditor::SaveOverrideFileRecurse(OADOutput& iffOut, const ParseTreeNode& root, bool writeAll) const
{
    root.Validate();

    const TypeBase* tb;
    if((tb = dynamic_cast<const TypeBase *>(&root))) 
    { 
        if(tb->Overridden() || writeAll)
            tb->WriteOverride(iffOut);
    }

    for(unsigned int i=0; i<root.children().size(); i++)
    {
        SaveOverrideFileRecurse(iffOut, *root.children()[i],writeAll);
    }
}

//==============================================================================

AttributeEditor* pGlobalEditor=NULL;
AttributeEditorGUI* pGlobalEditorGUI=NULL;

int
main(int argc, char *argv[]) {
  scratchMemory = new RealMalloc MEMORY_NAMED( ("attribedit") );
  Gtk::Main kit(argc, argv);


  bool writeAll = false;
  std::string overrideName;
  std::string objectName;
  std::string outputFileName;

  int currentArgIndex = 1;
  while(currentArgIndex < argc)
  {
      if(argv[currentArgIndex][0] == '-')
      {
#define OUTPUTNAME "--outputname="
          if(!strncmp(argv[currentArgIndex],OUTPUTNAME,strlen(OUTPUTNAME)))
          {
              outputFileName = &argv[currentArgIndex][strlen(OUTPUTNAME)];
          }
#define OVERRIDENAME "--overridename="
          else if(!strncmp(argv[currentArgIndex],OVERRIDENAME,strlen(OVERRIDENAME)))
          {
              overrideName = &argv[currentArgIndex][strlen(OVERRIDENAME)];
          }
#define OBJECTNAME "--objectname="
          else if(!strncmp(argv[currentArgIndex],OBJECTNAME,strlen(OBJECTNAME)))
          {
              objectName = &argv[currentArgIndex][strlen(OBJECTNAME)];
          }

#define WRITEALL "--writeall"
          else if(!strncmp(argv[currentArgIndex],WRITEALL,strlen(WRITEALL)))
          {
              writeAll = true;
          }

          else if(!strcmp(argv[currentArgIndex],"--help"))
          {
              std::cerr << "attribedit {--outputname=NAME | --overridename=NAME | --objectName=NAME | --writeAll | --help}" << std::endl;
              exit(1);
          }
          else
          {
            std::cerr << "Error: argument " << argv[currentArgIndex] << " not recognized" << std::endl;
          }
      }
      else
      {
        overrideName = argv[currentArgIndex];
      }

    currentArgIndex++;
  }

  AttributeEditor* editor;

  if(overrideName.length())
      editor = new AttributeEditor(overrideName);
  else
      editor = new AttributeEditor;
  assert(editor);
  pGlobalEditor = editor;

  //GladeXMLGenerator generator;
  //assert(parser);
  //generator.generate(editor.Parser()->getParseTree());


  if(outputFileName.size() == 0)
  {
     if(overrideName.size() == 0)
     {
        std::cerr << "You must specify either an input filename to edit, or use the --outputname= flag to start a new editing session and generate a new output file." << std::endl;
        exit(1);
     }
     outputFileName = overrideName;
  }
  
  editor->OutputFileName(outputFileName);

  std::string title = "WF Attribute Editor";
  if(objectName.length())
  {
      title += " - ";
      title += objectName;
  }

  AttributeEditorGUI aegui(*editor,title,writeAll);
  pGlobalEditorGUI = &aegui;
  kit.run();

  delete editor;
  return(0);
}

//==============================================================================

void
GlobalUpdateButtonEnables()
{
    if(pGlobalEditorGUI)
        pGlobalEditorGUI->UpdateButtonEnables();
}

//==============================================================================

