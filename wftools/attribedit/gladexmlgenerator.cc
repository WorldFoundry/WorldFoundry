//==============================================================================
// gladexmlgenerator.cc:
//==============================================================================

#include "gladexmlgenerator.hp"
#include "oad.hp"

void GladeXMLGenerator::generate(ParseTreeNode* root)
{
  root->Validate();
  Validate();
  while(_removeUselessNodes(root,NULL)) {};
  _parseRecursionLevel = 0;
  _recursiveGenerate(root);
  Validate();
}

void GladeXMLGenerator::_recursiveGenerate(ParseTreeNode* root)
{
  root->Validate();
  Validate();

//    printf("%-*.*sBEGIN\n",
//       _parseRecursionLevel*4,_parseRecursionLevel*4,"");

  _parseRecursionLevel++;

  if(_parseRecursionLevel==2)
    {printf("TAB----------------------\n");}

  printf("%-*.*s",
         _parseRecursionLevel*4,_parseRecursionLevel*4,"");

  if(dynamic_cast<NonterminalTypeNode *>(root))
  {
    //printf("NonterminalTypeNode\n");
  }
  else if(dynamic_cast<NonterminalStructNode *>(root))
  {
    // level 1 is the top-level TYPE
    // level 2 structures are tabsheets
    // level 3 and beyond are substructures in a tabsheet => group boxes
        printf("GROUP_BOX\n");
  
  }
  else if(dynamic_cast<NonterminalAnyFieldNode *>(root)) {printf("NonterminalAnyFieldNode\n");}
  else if(dynamic_cast<NonterminalF32FieldNode *>(root)) {printf("NonterminalF32FieldNode\n");}
  else if(dynamic_cast<NonterminalI32FieldNode *>(root)) {printf("NonterminalI32FieldNode\n");}
  else if(dynamic_cast<NonterminalStringFieldNode *>(root)) {printf("NonterminalStringFieldNode\n");}
  else if(TerminalNameNode* node = dynamic_cast<TerminalNameNode *>(root)) {printf("TerminalNameNode: %s\n",node->value());}
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
  else if(dynamic_cast<TypeInt32 *>(root)) { std::cout << *(dynamic_cast<TypeInt32 *>(root));}
  else if(dynamic_cast<TypeString *>(root)) {std::cout << *(dynamic_cast<TypeString *>(root));}
  else if(dynamic_cast<TypeFixed32 *>(root)) {std::cout << *(dynamic_cast<TypeFixed32 *>(root));}
  else {printf("UNKNOWN NODE!!\n");}

  int i;
  for(i=0; i<root->children().size(); i++)
  {
//      printf("%-*.*schild %d\n",
//         _parseRecursionLevel*4,_parseRecursionLevel*4,"",
//             i);

    _recursiveGenerate(root->children()[i]);
  }

  _parseRecursionLevel--;
//    printf("%-*.*sEND\n",
//       _parseRecursionLevel*4,_parseRecursionLevel*4,"");

  Validate();
}

int GladeXMLGenerator::_nodeIsUseful(ParseTreeNode *root) const
{
  root->Validate();
  Validate();

  int isUseful = 1;

  if(dynamic_cast<NonterminalStructNode*>(root)) {

    // a structure is only useful if it contains a non-substructure as a child,
    // or if it contains more than one substructure. If it just contains
    // ONE substructure, then it is a useless (common-block) structure.

    int structContainsUsefulInformation = 0;
    int substructCount = 0;

    int i;
    for(i=0; i<root->children().size() && !structContainsUsefulInformation; i++)
    {
      if((dynamic_cast<NonterminalStructNode *>(root->children()[i]) == 0)
         && (dynamic_cast<TerminalFlagNode *>(root->children()[i]) == 0))
      {
        // node is neither a structure nor a flag (ie FLAG COMMON)
        // node; thus it contains some useful information (such as a field)
        structContainsUsefulInformation = 1;
      }
      else if(dynamic_cast<NonterminalStructNode *>(root->children()[i]))
      {
        substructCount++;
      }
    }

    isUseful = structContainsUsefulInformation || (substructCount>1);
  }
  else if(dynamic_cast<TerminalFlagNode*>(root))
  {
    // we define flag nodes to be always useless

    isUseful=0;
    assert(root->children().size() == 0);
    // flag nodes should have no children; otherwise, where should we
    // put the children?
  }

  Validate();
  return isUseful;
}

int GladeXMLGenerator::_removeUselessNodes(ParseTreeNode* root,
                                           ParseTreeNode* parent)
{
  root->Validate();
  parent->Validate();
  Validate();

  if(!_nodeIsUseful(root))
  {
    if(parent)
    {
      int i;
      for(i=0; i<parent->children().size(); i++)
      {
        if(parent->children()[i] == root)
        {
//            printf("useless struct child %d being removed\n",i);
//            printf("before erase %p\n",parent->children()[i]);

          // delete this useless struct from parent's list
          parent->children().erase(parent->children().begin()+i,
                                   parent->children().begin()+i+1);
//            printf("DONE useless struct child %d being removed\n",i);
//            printf("after erase %p\n",parent->children()[i]);

          // add this useless struct's children directly to parent's list
          // at the location where the useless struct was deleted

          int iChildNode;
          for(iChildNode=0; iChildNode<root->children().size(); iChildNode++)
          {
            printf("shoveling child %d back into parent\n",iChildNode);
            parent->children().insert(parent->children().begin()+i,
                                      root->children()[iChildNode]);
          }
          Validate();
          return 1;
        }
      }
    }

    printf("can't remove toplevel useless struct\n");
  }

  int i;
  for(i=0; i<root->children().size(); i++)
  {
    int somethingRemoved;
    somethingRemoved = _removeUselessNodes(root->children()[i], root);
    if(somethingRemoved) 
    {
      Validate();
      return 1;
    }
  }

  Validate();
  return 0;
}

