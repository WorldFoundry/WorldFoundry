#include <math/vector3.hp>
#include <memory/memory.hp>

#include "oadesciffbinparser.hp"
#include "oad.hp"
  
// Norman, move this into a header file (I reccomend having a common
// header called global.hp that everything includes).
               
extern Memory* scratchMemory;
  
  
OADescIffBinParser::OADescIffBinParser()
{
    _parseTree = NULL;
}    

//==============================================================================

OADescIffBinParser::~OADescIffBinParser()
{
    if(_parseTree)
        delete _parseTree;
}    

//==============================================================================

void OADescIffBinParser::_parseStart(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  _parseRecursionLevel = 0;
  _structRecursionLevel = 0;
  assert(chunkIter->GetChunkID().ID() == IFFTAG('T','Y','P','E'));
  _parseTree = _parseNonterminalType(chunkIter);

  Validate();
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalType(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new NonterminalTypeNode;
  _parseRecursionLevel++;
  while(chunkIter->BytesLeft() > 0)
  {
    IFFChunkIter *childChunkIter = chunkIter->GetChunkIter(*scratchMemory);
    switch(childChunkIter->GetChunkID().ID())
    {
      case IFFTAG('N','A','M','E'):
      {
        thisNode->children().push_back(_parseTerminalName(childChunkIter));
        break;
      }
      case IFFTAG('S','T','R','U'):
      {
        thisNode->children().push_back(_parseNonterminalStruct(childChunkIter));
        break;
      }
       case IFFTAG('H','I','N','T'):
       {
         thisNode->children().push_back(_parseTerminalHint(childChunkIter));
         break;
       }
      default:
      {       
        std::cerr << childChunkIter->GetChunkID() << std::endl;
        assert(!"unknown token encountered while parsing TYPE");
      }
    }
    MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
  }
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalStruct(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new NonterminalStructNode;
  _parseRecursionLevel++;
//    printf("%-*.*s%d %s",
//           _structRecursionLevel*4, _structRecursionLevel*4,"",
//           _structRecursionLevel,
//           "Struct\n");
  _structRecursionLevel++;

  while(chunkIter->BytesLeft() > 0)
  {
    IFFChunkIter *childChunkIter = chunkIter->GetChunkIter(*scratchMemory);

    switch(childChunkIter->GetChunkID().ID())
    {
      case IFFTAG('S','T','R','U'):
      {
        thisNode->children().push_back(_parseNonterminalStruct(childChunkIter));
        break;
      }
      case IFFTAG('F','L','A','G'):
      {
        thisNode->children().push_back(_parseTerminalFlag(childChunkIter));
        break;
      }
      case IFFTAG('N','A','M','E'):
      {
        thisNode->children().push_back(_parseTerminalName(childChunkIter));
        break;
      }
      case IFFTAG('O','P','E','N'):
      {
        thisNode->children().push_back(_parseTerminalOpen(childChunkIter));
        break;
      }
      case IFFTAG('I','3','2','\0'):
      {
          TypeInt32* entry = new TypeInt32(*childChunkIter);
          thisNode->children().push_back(entry);
        //thisNode->children().push_back(_parseNonterminalI32Field(childChunkIter));
        break;
      }
      case IFFTAG('F','3','2','\0'):
      {
          TypeFixed32* entry = new TypeFixed32(*childChunkIter);
          thisNode->children().push_back(entry);
        //thisNode->children().push_back(_parseNonterminalF32Field(childChunkIter));
        break;
      }
      case IFFTAG('S','T','R','\0'):
      {
          TypeString* entry = new TypeString(*childChunkIter);
          thisNode->children().push_back(entry);
        //thisNode->children().push_back(_parseNonterminalStringField(childChunkIter));
        break;
      }
        case IFFTAG('H','I','N','T'):
        {
          thisNode->children().push_back(_parseTerminalHint(childChunkIter));
          break;
        }

      default:
      {       
        std::cerr << childChunkIter->GetChunkID() << std::endl;
        assert(!"unknown token encountered while parsing STRU");
      }
    }
    MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
  }
  _structRecursionLevel--;
//    printf("%-*.*s%d %s",
//           _structRecursionLevel*4, _structRecursionLevel*4,"",
//           _structRecursionLevel,
//           structIsEmptyWrapper ? "EndEMPTY\n" : "End\n");
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalName(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  _parseRecursionLevel++;
  char fieldValue[256];
  assert(chunkIter->BytesLeft() < 256);
  chunkIter->ReadBytes(fieldValue, chunkIter->BytesLeft());

  ParseTreeNode *thisNode = new TerminalNameNode(fieldValue);
//    printf("%-*.*sNAME: %s\n",
//           _structRecursionLevel*4, _structRecursionLevel*4, "",
//           fieldValue);
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalAnyField(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new NonterminalAnyFieldNode;
  _parseRecursionLevel++;
  while(chunkIter->BytesLeft() > 0)
  {
    IFFChunkIter *childChunkIter = chunkIter->GetChunkIter(*scratchMemory);

    switch(childChunkIter->GetChunkID().ID())
    {
      case IFFTAG('N','A','M','E'):
      {
        thisNode->children().push_back(_parseTerminalFieldName(childChunkIter));
        break;
      }
      case IFFTAG('D','S','N','M'):
      {
        thisNode->children().push_back(_parseTerminalDisplayName(childChunkIter));
        break;
      }
      case IFFTAG('R','A','N','G'):
      {
        thisNode->children().push_back(_parseTerminalRange(childChunkIter));
        break;
      }
      case IFFTAG('D','A','T','A'):
      {
        thisNode->children().push_back(_parseTerminalData(childChunkIter));
        break;
      }
      case IFFTAG('D','I','S','P'):
      {
        thisNode->children().push_back(_parseTerminalDisplayType(childChunkIter));
        break;
      }
      case IFFTAG('E','N','V','L'):
      {
        thisNode->children().push_back(_parseTerminalChoiceList(childChunkIter));
        break;
      }
      case IFFTAG('H','E','L','P'):
      {
        thisNode->children().push_back(_parseTerminalHelp(childChunkIter));
        break;
      }
      case IFFTAG('H','I','N','T'):
      {
        thisNode->children().push_back(_parseTerminalHint(childChunkIter));
        break;
      }
      case IFFTAG('E','N','B','L'):
      {
        thisNode->children().push_back(_parseTerminalEnabled(childChunkIter));
        break;
      }
      case IFFTAG('B','O','O','L'):
      {
        thisNode->children().push_back(_parseNonterminalBoolField(childChunkIter));
        break;
      }
      case IFFTAG('D','E','F','\0'):
      {
        thisNode->children().push_back(_parseTerminalDef(childChunkIter)); // ?? has something to do with object reference
        break;
      }
      default:
      {       
        assert(!"unknown token encountered while parsing AnyField");
      }
    }
    MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
  }
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalF32Field(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new NonterminalF32FieldNode;
  _parseRecursionLevel++;
  thisNode->children().push_back(_parseNonterminalAnyField(chunkIter));
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalI32Field(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new NonterminalI32FieldNode;
  _parseRecursionLevel++;
  thisNode->children().push_back(_parseNonterminalAnyField(chunkIter));
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalStringField(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode* thisNode = new NonterminalStringFieldNode;
  _parseRecursionLevel++;
  thisNode->children().push_back(_parseNonterminalAnyField(chunkIter));
  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalFlag(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();
  _parseRecursionLevel++;
  
  char fieldValue[256];
  assert(chunkIter->BytesLeft() < 256);
  chunkIter->ReadBytes(fieldValue, chunkIter->BytesLeft());
//    printf("%-*.*sFLAG: %s\n",
//           _structRecursionLevel*4, _structRecursionLevel*4, "",
//           fieldValue);
  ParseTreeNode *thisNode = new TerminalFlagNode(fieldValue);

  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalOpen(IFFChunkIter *chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode *thisNode = new TerminalOpenNode;
  _parseRecursionLevel++;

  unsigned char fieldValue;
  assert(chunkIter->BytesLeft() == sizeof(fieldValue));
  chunkIter->ReadBytes(&fieldValue, chunkIter->BytesLeft());
//    printf("%-*.*sOPEN: %d\n",
//           _structRecursionLevel*4, _structRecursionLevel*4, "",
//           fieldValue);

  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalFieldName(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  _parseRecursionLevel++;

  ParseTreeNode* thisNode = new TerminalFieldNameNode;
  char fieldValue[256];
  assert(chunkIter->BytesLeft() < sizeof(fieldValue));
  chunkIter->ReadBytes(fieldValue, chunkIter->BytesLeft());
//    printf("%-*.*sFIELDNAME: %s\n",
//           _structRecursionLevel*4, _structRecursionLevel*4, "",
//           fieldValue);

  _parseRecursionLevel--;

  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalDisplayName(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();

  ParseTreeNode* thisNode = new TerminalDisplayNameNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalRange(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode* thisNode = new TerminalRangeNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalData(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode* thisNode = new TerminalDataNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalDisplayType(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode* thisNode = new TerminalDisplayTypeNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalHelp(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode *thisNode = new TerminalHelpNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalHint(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();

  _parseRecursionLevel++;
  char fieldValue[256];
  assert(chunkIter->BytesLeft() < sizeof(fieldValue));
  chunkIter->ReadBytes(fieldValue, chunkIter->BytesLeft());

  ParseTreeNode *thisNode = new TerminalHintNode(fieldValue);
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalEnabled(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode* thisNode = new TerminalEnabledNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalChoiceList(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode* thisNode = new TerminalChoiceListNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseNonterminalBoolField(IFFChunkIter* chunkIter)
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode *thisNode = new NonterminalBoolFieldNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

ParseTreeNode* OADescIffBinParser::_parseTerminalDef(IFFChunkIter* chunkIter)
// ?? has something to do with object reference
{
  chunkIter->Validate();
  Validate();
  ParseTreeNode *thisNode = new TerminalDefNode;
  _parseRecursionLevel++;
  _parseRecursionLevel--;
  Validate();
  return thisNode;
}

void OADescIffBinParser::parseBinaryOADescFile(const std::string& filename)
{
  Validate();

  // I use the term OADesc instead of OAD to mean Object Attribute Description
  // I use the term OADat instead of OAD to mean Object Attribute Data
  // I use the term binIff to differentiate ASCII from binary IFF files

  binistream binIffOADescStream(filename.c_str());
  IFFChunkIter OADescIter(binIffOADescStream);
  _parseStart(&OADescIter);
  
  Validate();
}

void OADescIffBinParser::_recursivePrintParseTree(ParseTreeNode *root)
{
  root->Validate();
  Validate();

  //printf("%-*.*sBEGIN\n",_parseRecursionLevel*4,_parseRecursionLevel*4,"");

  _parseRecursionLevel++;

  //printf("%-*.*s",_parseRecursionLevel*4,_parseRecursionLevel*4,"");

  if(dynamic_cast<NonterminalTypeNode *>(root)) {printf("NonterminalTypeNode\n");}
  else if(dynamic_cast<NonterminalStructNode *>(root)) {printf("NonterminalStructNode\n");}
  else if(dynamic_cast<NonterminalAnyFieldNode *>(root)) {printf("NonterminalAnyFieldNode\n");}
  else if(dynamic_cast<NonterminalF32FieldNode *>(root)) {printf("NonterminalF32FieldNode\n");}
  else if(dynamic_cast<NonterminalI32FieldNode *>(root)) {printf("NonterminalI32FieldNode\n");}
  else if(dynamic_cast<NonterminalStringFieldNode *>(root)) {printf("NonterminalStringFieldNode\n");}
  else if(dynamic_cast<TerminalNameNode *>(root)) {printf("TerminalNameNode\n");}
  else if(dynamic_cast<TerminalFlagNode *>(root)) {printf("TerminalFlagNode\n");}
  else if(dynamic_cast<TerminalOpenNode *>(root)) {printf("TerminalOpenNode\n");}
  else if(dynamic_cast<TerminalFieldNameNode *>(root)) {printf("TerminalFieldNameNode\n");}
  else if(dynamic_cast<TerminalDisplayNameNode *>(root)) {printf("TerminalDisplayNameNode\n");}
  else if(dynamic_cast<TerminalRangeNode *>(root)) {printf("TerminalRangeNode\n");}
  else if(dynamic_cast<TerminalDataNode *>(root)) {printf("TerminalDataNode\n");}
  else if(dynamic_cast<TerminalDisplayTypeNode *>(root)) {printf("TerminalDisplayTypeNode\n");}
  else if(dynamic_cast<TerminalHelpNode *>(root)) {printf("TerminalHelpNode\n");}
  else if(dynamic_cast<TerminalHintNode *>(root)) {printf("TerminalHintNode\n");}
  else if(dynamic_cast<TerminalEnabledNode *>(root)) {printf("TerminalEnabledNode\n");}
  else if(dynamic_cast<TerminalChoiceListNode *>(root)) {printf("TerminalChoiceListNode\n");}
  else if(dynamic_cast<NonterminalBoolFieldNode *>(root)) {printf("NonterminalBoolFieldNode\n");}
  else if(dynamic_cast<TerminalDefNode *>(root)) {printf("TerminalDefNode\n");}

  int i;
  for(i=0; i<root->children().size(); i++)
  {
    _recursivePrintParseTree(root->children()[i]);
  }

  _parseRecursionLevel--;
  printf("%-*.*sEND\n",
	 _parseRecursionLevel*4,_parseRecursionLevel*4,"");

  Validate();
}

void OADescIffBinParser::printParseTree(void)
{
  Validate();
  _parseRecursionLevel = 0;
  _recursivePrintParseTree(_parseTree);
  Validate();
}

//==============================================================================

TypeBase* 
OADescIffBinParser::_LookupNodeByName(std::string name, ParseTreeNode* parentNode)
{
    TypeBase* node = dynamic_cast<TypeBase*>(parentNode);
    if(node)
    {
        if(node->Name() == name)
            return node;
    }

    for(int i=0; i<parentNode->children().size(); i++)
    {
      TypeBase* result = _LookupNodeByName(name,parentNode->children()[i]);
      if(result)
          return result;
    }
    return NULL;
}

TypeBase* 
OADescIffBinParser::_LookupNodeByNoSpacesName(std::string name, ParseTreeNode* parentNode)
{
    TypeBase* node = dynamic_cast<TypeBase*>(parentNode);
    if(node)
    {
        if(node->NameNoSpaces() == name)
            return node;
    }

    for(int i=0; i<parentNode->children().size(); i++)
    {
      TypeBase* result = _LookupNodeByNoSpacesName(name,parentNode->children()[i]);
      if(result)
          return result;
    }
    return NULL;
}

//==============================================================================

TypeBase* 
OADescIffBinParser::LookupNodeByName(std::string name)
{
    return _LookupNodeByName(name, getParseTree());
}

TypeBase* 
OADescIffBinParser::LookupNodeByNoSpacesName(std::string name)
{
    return _LookupNodeByNoSpacesName(name, getParseTree());
}

//==============================================================================

