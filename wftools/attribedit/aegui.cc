//==============================================================================
//aegui.cc
//==============================================================================

#include <iostream>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/togglebutton.h>
#include <gtk--/radiobutton.h>
#include <gtk--/spinbutton.h>
#include <gtk--/checkbutton.h>
#include <gtk--/separator.h>
#include <gtk--/scale.h>
#include <gtk--/adjustment.h>
#include <gtk--/range.h>
#include <gtk--/menu.h>
#include <gtk--/optionmenu.h>
#include <gtk--/main.h>
#include <gtk--/scrollbar.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/fileselection.h>
#include <gtk--/frame.h>
#include <gtk--/notebook.h>
#include <gtk--/tooltips.h>
#include <gtk--/packer.h>
#include <vector>
#include <string>
#include <fstream>

#include "gui.hp"
#include "aegui.hp"
#include "attribedit.hp"
#include "guihelpers.hp"
#include "aegui.hp"
#include "oad.hp"
#include "parsetreenode.hp"

//#include <oas/oad.h>
#include <ini/profile.hp>

using std::cout;

using SigC::bind;
using SigC::slot;

//==============================================================================

inline gint 
AttributeEditorGUI::delete_event_impl(GdkEventAny*) 
{
    Gtk::Main::quit(); return 0; 
}

//==============================================================================

void
AttributeEditorGUI::ConstructStaticInterface(const std::string& title)
{
    //cout << "AttributeEditorGUI::Construct Interface " << std::endl;

    set_title(title);
    //cout << "AttributeEditorGUI::AttributeEditorGUI: top of loop " << std::endl;

    set_border_width(10);

    set_default_size(200,480);
    add(masterBox);
    masterBox.pack_start(toolBox,false);
    Gtk::Widget* newWidget;
    newWidget = new Gtk::HSeparator();
    masterBox.pack_start(*newWidget,false);
    newWidget->show();

// add the dynamic widgets
//     Gtk::ScrolledWindow* sWind = new Gtk::ScrolledWindow();
//     assert(sWind);
//     sWind->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
//
//     masterBox.pack_start(*sWind);
//     sWind->add_with_viewport(attribContainerBox);
//     attribContainerBox.set_spacing(8);

    masterBox.pack_start(attribContainerBox);
    masterBox.set_spacing(5);

// add the static widgets at the top

    {
        using namespace Gtk::Menu_Helpers;

        menuClassName=manage(new Gtk::Menu);
        MenuList& menuClassNameList=menuClassName->items();

        std::vector<std::string>::const_iterator iter(editor.ClassList().begin());
        iter++;             // kts skip NULLobject
        int classIndex = -1;
        for(int index=0;iter != editor.ClassList().end();iter++,index++)
        {
            menuClassNameList.push_back(
                                       MenuElem((*iter).c_str(),bind(
                                                                    slot(this,&AttributeEditorGUI::SelectNewClass),*iter)));

            if(*iter == editor.ClassName())
                classIndex = index;

        }
        assert(classIndex != -1);
       // menuClassName->set_active(classIndex);

        optionMenuClassName = new LabeledOptionMenu("Object Class:",menuClassName);
        assert(optionMenuClassName);
        toolBox.pack_start(*optionMenuClassName);
        optionMenuClassName->om->set_history(classIndex);
    }

     Gtk::Button* newButton;
//     newButton = new Gtk::Button("dialog");
//     //newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("Revert",GUIElementCommandButton::STATIC_BUTTON_TEST,newButton)));
//     newButton->clicked.connect(SigC::slot(&pop_dialog));
//     toolBox.pack_start(*newButton);
//     newButton->show();

//     newButton = new Gtk::Button("test");
//     newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("test",GUIElementCommandButton::STATIC_BUTTON_TEST,newButton)));
//     toolBox.pack_start(*newButton);
//     newButton->show();

    newButton = new Gtk::Button("Copy");
    newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("copy",GUIElementCommandButton::STATIC_BUTTON_COPY,newButton)));
    toolBox.pack_start(*newButton);
    newButton->show();
    newButton = new Gtk::Button("Paste");
    newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("paste",GUIElementCommandButton::STATIC_BUTTON_PASTE,newButton)));
    toolBox.pack_start(*newButton);
    newButton->show();

    newButton = new Gtk::Button("Cancel");
    newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("Cancel",GUIElementCommandButton::STATIC_BUTTON_CANCEL,newButton)));
    toolBox.pack_start(*newButton);
    newButton->show();

    newButton = new Gtk::Button("Ok");
    newButton->clicked.connect(bind<GUIElementCommandButton*>(slot(this, &AttributeEditorGUI::StaticButtonCallback), new GUIElementCommandButton("Ok",GUIElementCommandButton::STATIC_BUTTON_OK,newButton)));
    toolBox.pack_start(*newButton);
    newButton->show();

    toolBox.show_all();

}

//==============================================================================


AttributeEditorGUI::AttributeEditorGUI(AttributeEditor& newEditor, const std::string& title, bool writeAll) :
masterBox(false, 0), // creates a box to pack widgets into
toolBox(false, 0), 
menuClassName(NULL),
editor(newEditor),
_writeAll(writeAll)
{
    //cout << "AttributeEditorGUI::AttributeEditorGUI " << std::endl;
    ConstructStaticInterface(title);
    attribBox = new Gtk::VBox(false,0);
    assert(attribBox);
    attribContainerBox.pack_start(*attribBox);
    ConstructOADInterface(editor.Parser().getParseTree(),*attribBox);
    attribContainerBox.show();
    masterBox.show();                  
    show();
}

//==============================================================================

void AttributeEditorGUI::StaticButtonCallback(GUIElementCommandButton* scd)
{
    switch(scd->buttonIndex)
    {
        case GUIElementCommandButton::STATIC_BUTTON_OK:
            // Write out the override file
            editor.SaveOverrideFile(editor.OutputFileName(),false,_writeAll);
            Gtk::Main::quit();
            break;
        case GUIElementCommandButton::STATIC_BUTTON_CANCEL:
            Gtk::Main::quit();
            break;
        case GUIElementCommandButton::STATIC_BUTTON_COPY:
            {
                std::string filename = editor.WFConfigPath();
                filename += "/attribedit.ini";

                filename = editor.WFConfigPath(); 
                filename += "/clipboard" OVERRIDE_FILE_EXTENSION;
#if defined(USEXML)
                editor.SaveOverrideFile(filename);
#else
                editor.SaveOverrideFile(filename,true,_writeAll);
#endif
            }
            break;
        case GUIElementCommandButton::STATIC_BUTTON_PASTE:
            {
                attribContainerBox.hide();
                attribContainerBox.remove(*attribBox);
                DestroyOADInterface();

                std::string filename = editor.WFConfigPath();
                filename += "/clipboard" OVERRIDE_FILE_EXTENSION;
                editor.LoadOverrideFile(filename);

                attribBox = new Gtk::VBox(false,0);
                assert(attribBox);
                attribContainerBox.pack_start(*attribBox);

                ConstructOADInterface(editor.Parser().getParseTree(),*attribBox);
                attribContainerBox.show_all();
            }
            break;
        case GUIElementCommandButton::STATIC_BUTTON_TEST:
            printf("button: test\n");
            menuClassName->set_active(5);

       //WritePrivateProfileString( "dirs", "oaddir", "/usr/local/src/WorldFoundry/wfsource/levels/oad", filename.c_str() );
         // kts test spot, put whatever gui action is being experimented with here
            break;
        default:
            assert(0);
            break;
    }

//    cout << "AEGUI:: static callback: " << scd->buttonIndex << ", " << scd->name<< endl;
}


//==============================================================================

static AttributeEditorGUI* aegui;

double
fnSymbolLookup( const char* yytext )
{
    std::string name(yytext);

    std::vector<GUIElementOAD*>::const_iterator iter;
    const std::vector<GUIElementOAD*>& elements = aegui->Elements();
    for(iter = elements.begin();iter!=elements.end();iter++)
    {
        const TypeBase& tb = (*iter)->OAD(); 

        if(tb.NameNoSpaces() == name)
        {
            double value = 0.0;

            if(dynamic_cast<const TypeInt32*>(&tb)) 
            {
                const TypeInt32* ti = dynamic_cast<const TypeInt32*>(&tb);
                return ti->CurrentValue();
            }
            else if(dynamic_cast<const TypeFixed32*>(&tb)) 
            {
                const TypeFixed32* tf = dynamic_cast<const TypeFixed32*>(&tb);
                return tf->CurrentValue();
            }
            else if(dynamic_cast<const TypeString*>(&tb)) 
            {
                const TypeString* ts = dynamic_cast<const TypeString*>(&tb);
                int len = ts->CurrentValue().size();
                return len;
            }

            else
                assert(0);



            return value;
        }

    }

//	uiDialog* field = theAttributes.findGadget( yytext );
//	return field ? field->eval() : 0.0;
    return 0.0;
}


void 
AttributeEditorGUI::UpdateButtonEnables()
{
    aegui = this;
    std::vector<GUIElementOAD*>::iterator iter;
    for(iter = elements.begin();iter!=elements.end();iter++)
    {
        (*iter)->UpdateEnabled(fnSymbolLookup); 
    }
}

//==============================================================================

void 
AttributeEditorGUI::SelectNewClass(std::string name)
{
//    cout << "new class chosen: " << name << endl;

    attribContainerBox.hide();
    attribContainerBox.remove(*attribBox);
    DestroyOADInterface();
    editor.ClassName(name);

    attribBox = new Gtk::VBox(false,0);
    assert(attribBox);
    attribContainerBox.pack_start(*attribBox);

    ConstructOADInterface(editor.Parser().getParseTree(),*attribBox);
    attribContainerBox.show();
}

//==============================================================================

void
AttributeEditorGUI::DestroyOADInterface()
{
    delete attribBox;
    elements.clear();
}

//==============================================================================

void AttributeEditorGUI::ConstructOADInterface(ParseTreeNode* root, Gtk::Box& box)
{
  //cout << "generate!!!" << endl;
    root->Validate();
  //Validate();
  //while(_removeUselessNodes(root,NULL)) {};

    std::vector<std::string>::const_iterator iter(editor.ClassList().begin());
    iter++;             // kts skip NULLobject
    int classIndex = -1;

    assert(menuClassName);

    for(int index=0;iter != editor.ClassList().end();iter++,index++)
    {
        if(*iter == editor.ClassName())
            classIndex = index;
    }
    assert(classIndex != -1);
    optionMenuClassName->om->set_history(classIndex);  

  //cout << "looking for class named " << editor.ClassName() << ", classIndex = " << classIndex << endl;

    assert(classIndex != -1);
    menuClassName->set_active(classIndex);


    Gtk::Notebook* noteBook = manage(new Gtk::Notebook());
    assert(noteBook);

    box.pack_start(*noteBook);
    noteBook->set_tab_pos(GTK_POS_TOP);
    noteBook->set_page(0);

    _recursiveGenerate(root,*noteBook, &box,0,0);

    box.show_all();
    UpdateButtonEnables();
  //Validate();
}

void AttributeEditorGUI::_recursiveGenerate(ParseTreeNode* root, Gtk::Notebook& noteBook, Gtk::Box* box,int groupRecursionLevel, int structRecursionLevel)
{
    GUIElementOAD* element=NULL;

    root->Validate();
  //Validate();

//    printf("%-*.*sBEGIN\n",
//       _groupRecursionLevel*4,_groupRecursionLevel*4,"");

//  if(groupRecursionLevel==2)
//    {printf("TAB----------------------\n");}

//  printf("%-*.*s",
//         groupRecursionLevel*4,groupRecursionLevel*4,"");

    Gtk::Widget* newWidget = NULL;

    if(dynamic_cast<NonterminalTypeNode *>(root))
    {
        for(unsigned int i=0; i<root->children().size(); i++)
        {
    //      printf("%-*.*schild %d\n",
    //         groupRecursionLevel*4,groupRecursionLevel*4,"",
    //             i);

            _recursiveGenerate(root->children()[i],noteBook, box,groupRecursionLevel+1,structRecursionLevel+1);
        }

    //printf("NonterminalTypeNode\n");
    }
    else if(dynamic_cast<NonterminalStructNode *>(root))
    {
      // assume the name chunk is the first one in the struct chunk
        assert(root->children().size() > 0);
        TerminalNameNode* nameNode = dynamic_cast<TerminalNameNode *>(root->children()[0]);
        assert(nameNode);
        // check to see if there is a flag chunk (assume it is 2nd after the name chunk)
        TerminalFlagNode* flagNode = NULL;
        if((root->children().size() > 1))
        {
            //cout << "checking for flag node" << endl;
            flagNode = dynamic_cast<TerminalFlagNode *>(root->children()[1]);
        }

//         cout << "struct chunk nested at " << groupRecursionLevel << ", struct name = " << nameNode->value() << endl;
//         if(flagNode)
//             cout << "  flag = " << flagNode->value() << endl;


        if(flagNode && !strcmp(flagNode->value(),"COMMONBLOCK"))
        {
            // just a common block, ignore it (don't increment recursion level, but parse children)
            for(unsigned int i=0; i<root->children().size(); i++)
            {
                _recursiveGenerate(root->children()[i],noteBook,box,groupRecursionLevel,structRecursionLevel+1);
            }
        }
        else
        {
            switch(groupRecursionLevel)
            {
                case 1:         // property sheets
                    {
                        //cout << "doing property sheets" << endl;
                        Gtk::VBox* noteBookPage = manage(new Gtk::VBox(false));
                        assert(noteBookPage);

                        Gtk::ScrolledWindow* sWind = new Gtk::ScrolledWindow();
                        assert(sWind);
                        sWind->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

                        sWind->add_with_viewport(*noteBookPage);
                    //noteBookPage->set_spacing(8);

                        noteBook.pages().push_back(
                                                  Gtk::Notebook_Helpers::TabElem(*sWind,nameNode->value()));

                        noteBookPage->show();
                        for(unsigned int i=0; i<root->children().size(); i++)
                        {
                        //cout << "doing child at index " << i << endl;
                      //_recursiveGenerate(root->children()[i],noteBook,noteBookPage,groupRecursionLevel+1);
                            _recursiveGenerate(root->children()[i],noteBook,noteBookPage,groupRecursionLevel+1,structRecursionLevel+1);
                        }
                    }
                    break;
                case 2:         // group boxes
                    {
                    // assume the name chunk is the first one in the struct chunk
                        Gtk::Frame* frame = manage(new Gtk::Frame(nameNode->value()));
                        Gtk::VBox* innerBox = manage(new Gtk::VBox());
                        frame->add(*innerBox);
                        frame->set_label(nameNode->value());
                    //frame->show_all();
                        box->pack_start(*frame,false);
                        for(unsigned int i=0; i<root->children().size(); i++)
                        {
                            _recursiveGenerate(root->children()[i],noteBook,innerBox,groupRecursionLevel+1,structRecursionLevel+1);
                        }
                    }
                    break;
                default:
                    assert(0);
                    break;        
            }

        }

    // level 1 is the top-level TYPE
    // level 2 structures are tabsheets
    // level 3 and beyond are substructures in a tabsheet => group boxes
        //printf("GROUP_BOX, recursion level = %d\n",groupRecursionLevel);
    }
    else if(dynamic_cast<NonterminalAnyFieldNode *>(root))
    {
        printf("NonterminalAnyFieldNode\n");
    }
    else if(dynamic_cast<NonterminalF32FieldNode *>(root))
    {
        printf("NonterminalF32FieldNode\n");
    }
    else if(dynamic_cast<NonterminalI32FieldNode *>(root))
    {
        printf("NonterminalI32FieldNode\n");
    }
    else if(dynamic_cast<NonterminalStringFieldNode *>(root))
    {
        printf("NonterminalStringFieldNode\n");
    }
    else if(TerminalNameNode* node = dynamic_cast<TerminalNameNode *>(root))
    {
      //printf("struct recursion level: %d, Class name is : %s\n",structRecursionLevel,node->value());
    }
    else if(dynamic_cast<TerminalFlagNode *>(root))
    {
//        printf("TerminalFlagNode\n");
    }
    else if(dynamic_cast<TerminalOpenNode *>(root))
    {
//        printf("TerminalOpenNode\n");
    }
    else if(dynamic_cast<TerminalFieldNameNode *>(root))
    {
        printf("TerminalFieldNameNode\n");
    }
    else if(dynamic_cast<TerminalDisplayNameNode *>(root))
    {
        printf("TerminalDisplayNameNode\n");
    }
    else if(dynamic_cast<TerminalRangeNode *>(root))
    {
        printf("TerminalRangeNode\n");
    }
    else if(dynamic_cast<TerminalDataNode *>(root))
    {
        printf("TerminalDataNode\n");
    }
    else if(dynamic_cast<TerminalDisplayTypeNode *>(root))
    {
        printf("TerminalDisplayTypeNode\n");
    }
    else if(dynamic_cast<TerminalHelpNode *>(root))
    {
        printf("TerminalHelpNode\n");
    }
    else if(dynamic_cast<TerminalHintNode *>(root))
    {
        //printf("TerminalHintNode\n");
    }
    else if(dynamic_cast<TerminalEnabledNode *>(root))
    {
        printf("TerminalEnabledNode\n");
    }
    else if(dynamic_cast<TerminalChoiceListNode *>(root))
    {
        printf("TerminalChoiceListNode\n");
    }
    else if(dynamic_cast<NonterminalBoolFieldNode *>(root))
    {
        printf("NonterminalBoolFieldNode\n");
    }
    else if(dynamic_cast<TerminalDefNode *>(root))
    {
        printf("TerminalDefNode\n");
    }
    else if(dynamic_cast<TypeInt32 *>(root))
    {
      //cout << *(dynamic_cast<TypeInt32 *>(root));
        TypeInt32& entry = *(dynamic_cast<TypeInt32 *>(root));

//        GUIElementInteger* newElement;
        element = new GUIElementInteger(entry,*box);
        assert(element);
    }
    else if(dynamic_cast<TypeString *>(root))
    {
        TypeString& entry = *(dynamic_cast<TypeString *>(root));
//        GUIElementString* newElement;
        element = new GUIElementString(entry,*box);
        assert(element);
    }
    else if(dynamic_cast<TypeFixed32 *>(root))
    {
        TypeFixed32& entry = *(dynamic_cast<TypeFixed32 *>(root));
//        GUIElementFixed* newElement;
        element = new GUIElementFixed(entry,*box);
        assert(element);
    }
    else
    {
        printf("UNKNOWN NODE!!\n");
        assert(0);
    }


#if 0
    for(int i=0; i<root->children().size(); i++)
    {
//      printf("%-*.*schild %d\n",
//         groupRecursionLevel*4,groupRecursionLevel*4,"",
//             i);

        _recursiveGenerate(root->children()[i],box,groupRecursionLevel+1,structRecursionLevel+1);
    }
#endif



    if(element)
    {
        elements.push_back(element);
    }


//    printf("%-*.*sEND\n",
//       groupRecursionLevel*4,groupRecursionLevel*4,"");

//  Validate();
}

//==============================================================================

