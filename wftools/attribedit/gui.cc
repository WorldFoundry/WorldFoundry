//==============================================================================
// gui.cc: 
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
#include <gtk--/scrollbar.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/frame.h>
#include <gtk--/notebook.h>
#include <gtk--/tooltips.h>
#include <gtk--/packer.h>
#include <gtk--/colorselection.h>
#include <string>
#include <fstream>
#include <sstream>

#include <eval/eval.h>

#include "gui.hp"
#include "aegui.hp"
#include "guihelpers.hp"
#include "oad.hp"
#include "parsetreenode.hp"
#include "attribedit.hp"
//#include <oas/oad.h>

using std::cout;

using SigC::bind;
using SigC::slot;

//==============================================================================

GUIElement::GUIElement(Gtk::Widget* )
{
}

//==============================================================================

GUIElementOAD::GUIElementOAD(TypeBase& newOadEntry, Gtk::Box& parent) :  GUIElement(), oadEntry(newOadEntry)
{
    enableButton = NULL;
    if(oadEntry.GetShowAs() == TypeBase::SHOW_AS_HIDDEN)
        return;           // if hidden, do nothing

   // all entrys have to be in a nested hbox so that the enable checkbox on the left can ghost them
    Gtk::HBox* enablehbox = manage( new Gtk::HBox () );
    assert(enablehbox);

    enableButton = manage(new Gtk::CheckButton());
    assert(enableButton);

    Gtk::Tooltips* tooltip = manage(new Gtk::Tooltips);
    assert(tooltip);

    std::string result = "Enable ";
    result += oadEntry.DisplayName();
    tooltip->set_tip (*enableButton, result, 0);

    enableButton->clicked.connect(slot(this, &GUIElementOAD::EnableCallback));

    parent.pack_start(*enablehbox,false);
    enablehbox->pack_start(*enableButton,false);

    elementBox = manage( new Gtk::HBox (false, 0) );
    assert(elementBox);

    enablehbox->pack_end(*elementBox);

//     if(newOadEntry.Overridden())
//         cout << "override! on entry named " << newOadEntry.DisplayName() << std::endl;
    enableButton->set_active(newOadEntry.Overridden());
    elementBox->set_sensitive(newOadEntry.Overridden());
}

//==============================================================================
    
bool 
GUIElementOAD::CheckEnabled(double (*fnSymbolLookup)( const char* yytext ) )                // re-evaluate enable expression and enable accordingly
{
    const std::string& expression = oadEntry.EnableExpression();
    float val;

    if(expression != "1")
        val = eval(expression.c_str(), fnSymbolLookup );
    else
        val = 1;
    return val;
}

//==============================================================================

void 
GUIElementOAD::UpdateEnabled(double (*fnSymbolLookup)( const char* yytext ) )                // re-evaluate enable expression and enable accordingly
{

    if(enableButton)                        // hiden items are NULL
    {

        assert(enableButton);
        assert(elementBox);
        if(CheckEnabled(fnSymbolLookup))
        {
            elementBox->set_sensitive(enableButton->get_active());
            enableButton->show_all();
        }
        else
        {
            elementBox->set_sensitive(0);
            enableButton->hide_all();
        }   
    }
}

//==============================================================================

void
GUIElementOAD::AddHelp(Gtk::Widget& widget, const std::string* additionalHelp)
{
    Gtk::Tooltips* tooltip = manage(new Gtk::Tooltips);
    assert(tooltip);

    std::string result = oadEntry.Help();
    if(additionalHelp)
    {
        result += " ( ";
        result += *additionalHelp;
        result += " )";
    }

    tooltip->set_tip (widget, result.length()?result:"No Help Available", 0);
}

//==============================================================================

void GUIElementOAD::EnableCallback()
{
    elementBox->set_sensitive(enableButton->get_active());
    oadEntry.Overridden(enableButton->get_active());

//    cout << "GUIElementOAD::EnableCallback - " << oadEntry.DisplayName() << " was set to " << oadEntry.Overridden() << std::endl;
}

//==============================================================================

GUIElementCommandButton::GUIElementCommandButton(const char* newName, int newButtonIndex, Gtk::Widget*) : GUIElement(), buttonIndex(newButtonIndex), name(newName)
{
}

//==============================================================================

GUIElementInteger::GUIElementInteger(TypeInt32& entry, Gtk::Box& box) :
GUIElementOAD(entry, box)
{

    if(entry.GetShowAs() == TypeBase::SHOW_AS_HIDDEN)
        return;           // if hidden, do nothing

   //elementBox->set_sensitive(false);
    intAdj = NULL;
    toggleButton = NULL;

    switch(entry.GetShowAs())
    {
        case TypeBase::SHOW_AS_N_A:
        // now what?
            assert(0);
        case TypeBase::SHOW_AS_NUMBER:
            {
                Gtk::SpinButton* newButton;

                intAdj = new Gtk::Adjustment( entry.CurrentValue(),
                                              entry.Min(),
                                              entry.Max(),
                                              1.0,
                                              (entry.Max()-entry.Min())/10,
                                              0 );

                newButton = new Gtk::SpinButton(*intAdj,0,0);
                assert(newButton);
                newButton->changed.connect(slot(this, &GUIElementInteger::Callback));

                newButton->set_numeric(true);
                //widget = newButton;

                Gtk::Label* label = manage( new Gtk::Label (entry.DisplayName(), 0, 0.5) );
                elementBox->pack_end (*newButton,false);
                elementBox->pack_end (*label,false);

                std::ostringstream temp;
                temp << entry.Min()<< "-" << entry.Max();
                AddHelp(*newButton,&temp.str());
                elementBox->show_all();

                break;
            }
            break;
        case TypeBase::SHOW_AS_SLIDER:  
            {
                intAdj = new Gtk::Adjustment( entry.CurrentValue(),
                                              entry.Min(),
                                              entry.Max(),
                                              1.0,
                                              (entry.Max()-entry.Min())/20,
                                              0 );

                Gtk::HScale* newWidget = new Gtk::HScale(*intAdj);
                assert(newWidget);

                Gtk::Label* label = manage( new Gtk::Label (entry.DisplayName(), 0, 0.5) );
                elementBox->pack_start (*label);
                elementBox->pack_start (*newWidget);
                std::ostringstream temp;
                temp << entry.Min() << "-" << entry.Max();
                AddHelp(*newWidget,&temp.str());
                elementBox->show_all();

                intAdj->value_changed.connect(slot(this,&GUIElementInteger::Callback));
            }
            break;
        case TypeBase::SHOW_AS_TOGGLE:
            toggleButton = new Gtk::ToggleButton(entry.DisplayName());
            assert(toggleButton);
            toggleButton->clicked.connect(slot(this, &GUIElementInteger::Callback));
            toggleButton->set_active(entry.CurrentValue());
            elementBox->pack_end(*toggleButton,false);
            AddHelp(*toggleButton);
            break;
        case TypeBase::SHOW_AS_DROPMENU:
            {
                {
                    using namespace Gtk::Menu_Helpers;

                    Gtk::Menu *menu_vpos=manage(new Gtk::Menu);
                    MenuList& list_vpos=menu_vpos->items();


                    char* origStrPtr = strdup(entry.EnumValues().c_str());
                    char* strPtr = origStrPtr;

                    int index=0;
                    while(*strPtr)
                    {
                        char* cursor = strPtr;
                        while(*cursor && *cursor != '|')
                            cursor++;

                        if(*cursor)
                            *cursor++ = 0;

                        list_vpos.push_back(
                                           MenuElem(strPtr,bind<int>(slot(this,&GUIElementInteger::MenuCallback),index)));

                        strPtr = cursor;
                        index++;
                    }
                    free(origStrPtr);

                    menu_vpos->set_active(entry.CurrentValue());
                    elementBox->pack_end(
                                        *manage(new 
                                                LabeledOptionMenu (entry.DisplayName(),menu_vpos)),false);
                    AddHelp(*menu_vpos);
                }
            }
            break;
        case TypeBase::SHOW_AS_RADIOBUTTONS:
            {
                Gtk::Frame* frame = new Gtk::Frame(entry.DisplayName());
                assert(frame);
                Gtk::VBox* vbox = new Gtk::VBox;
                assert(vbox);
//                Gtk::Label* label = new Gtk::Label(entry.DisplayName());
//                vbox->pack_start(*label,false);
                char* origStrPtr = strdup(entry.EnumValues().c_str());
                char* strPtr = origStrPtr;
                Gtk::RadioButton* lastButton = NULL;
                Gtk::RadioButton* newWidget = NULL;
                Gtk::RadioButton* activeWidget = NULL;

                int index=0;
                while(*strPtr)
                {
                    char* cursor = strPtr;
                    while(*cursor && *cursor != '|')
                        cursor++;

                    if(*cursor)
                        *cursor++ = 0;
                    newWidget = new Gtk::RadioButton(strPtr);
                    assert(newWidget);

                    vbox->pack_start(*newWidget);

                    AddHelp(*newWidget);                                                          // always remember this step, this tells GTK that our preparation
                    newWidget->show();

                    if(lastButton)
                        newWidget->set_group(lastButton->group());
                    if(index == entry.CurrentValue())
                    {
                        activeWidget = newWidget;
                        //cout << "!! set active on " << strPtr << " index = " << index << ", current value = " << entry.CurrentValue() << std::endl;
                    }
                    newWidget->clicked.connect(bind<int>(slot(this, &GUIElementInteger::RadioCallback), index));

                    strPtr = cursor;
                    lastButton = newWidget;
                    index++;
                }
                free(origStrPtr);
                assert(activeWidget);
                activeWidget->set_active(true);

                frame->add(*vbox);
                elementBox->pack_start(*frame);
                //elementBox->pack_start(*vbox,false);

            }
            break;
        case TypeBase::SHOW_AS_HIDDEN:
            break;
        case TypeBase::SHOW_AS_COLOR:     
            { 
                Gtk::VBox* vbox = manage(new Gtk::VBox);
                assert(vbox);
                Gtk::Label* label = manage(new Gtk::Label( entry.DisplayName()));
                assert(label);
                vbox->pack_start(*label,false);
                
                int intColor = entry.CurrentValue();

                double colorArray[4];

                cout << "intColor = " << intColor << std::endl;

                colorArray[0] = ((double)((intColor>>16) & 0xff))/256.0;
                colorArray[1] = ((double)((intColor>>8) & 0xff))/256.0;
                colorArray[2] = ((double)((intColor) & 0xff))/256.0;
                colorArray[3] = 1.0;

                cout << "color array = " << colorArray[0] << "," << colorArray[1] << "," << colorArray[2] << std::endl;

                picker = manage(new Gtk::ColorSelection());
                assert(picker);
                picker->set_color(colorArray);
                picker->set_opacity(false);
                //picker->set_update_policy(GTK_UPDATE_DISCONTINUOUS);
                picker->color_changed.connect(slot(this, &GUIElementInteger::ColorCallback));

                vbox->pack_end(*picker,false);
                AddHelp(*picker);
                elementBox->pack_end(*vbox,false);

            }
            break;
        case TypeBase::SHOW_AS_CHECKBOX:  
            {
                Gtk::Label* label = manage(new Gtk::Label( entry.DisplayName()));
                assert(label);

                toggleButton = manage(new Gtk::CheckButton());
                assert(toggleButton);
                toggleButton->set_active(entry.CurrentValue());
                toggleButton->clicked.connect(slot(this, &GUIElementInteger::Callback));

                elementBox->pack_end(*toggleButton,false);
                AddHelp(*toggleButton);
                elementBox->pack_end(*label,false);
            }
            break;
        case TypeBase::SHOW_AS_MAILBOX:     
            assert(0);
            break;
        case TypeBase::SHOW_AS_COMBOBOX:  
            assert(0);
            break;
        case TypeBase::SHOW_AS_TEXTEDITOR:
            assert(0);
            break;
        default:
            std::cerr << "showas of " << entry.GetShowAs() << " not supported" << std::endl;
            assert(0);
            break;
    }
}


void GUIElementInteger::Callback()
{
    TypeInt32* entry = (dynamic_cast<TypeInt32 *>(&oadEntry));
    assert(entry);

    if(intAdj)
        entry->CurrentValue(int(intAdj->get_value()));
    if(toggleButton)
        entry->CurrentValue(toggleButton->get_active());
    //cout << "GUIEInteger::callback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;

    GlobalUpdateButtonEnables();
}

void GUIElementInteger::RadioCallback(int entryIndex)
{
    TypeInt32* entry = (dynamic_cast<TypeInt32 *>(&oadEntry));
    assert(entry);
    entry->CurrentValue(entryIndex);
//    cout << "GUIEInteger::RadioCallback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;
    GlobalUpdateButtonEnables();
}

void GUIElementInteger::ColorCallback()
{
    TypeInt32* entry = (dynamic_cast<TypeInt32 *>(&oadEntry));
    assert(entry);

    double color[4];
    picker->get_color(color);


//     cout << color[0] << "<" << color[1] << "," << color[2] << std::endl;
//
//     cout << ((max(min(((int)(256.0*color[0])),255),0))<<16) << ",";
//     cout << ((max(min(((int)(256.0*color[1])),255),0))<<8)  << ",";
//     cout <<  (max(min(((int)(256.0*color[2])),255),0))             <<std::endl;

    int intColor = 
        ((std::max(std::min(((int)(256.0*color[0])),255),0))<<16) |
        ((std::max(std::min(((int)(256.0*color[1])),255),0))<<8)  |
         (std::max(std::min(((int)(256.0*color[2])),255),0)) 
    ;

    entry->CurrentValue(intColor);
//    cout << "GUIEInteger::ColorRadioCallback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;
    GlobalUpdateButtonEnables();
}



void GUIElementInteger::MenuCallback(int entryIndex)
{
    TypeInt32* entry = (dynamic_cast<TypeInt32 *>(&oadEntry));
    assert(entry);
    entry->CurrentValue(entryIndex);
//    cout << "GUIEInteger::MenuCallback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;
    GlobalUpdateButtonEnables();
}

//==============================================================================

GUIElementFixed::GUIElementFixed(TypeFixed32& entry,Gtk::Box& box) :
GUIElementOAD(entry, box)
{
    if(entry.GetShowAs() == TypeBase::SHOW_AS_HIDDEN)
        return;           // if hidden, do nothing

    switch(entry.GetShowAs())
    {
        case TypeBase::SHOW_AS_N_A:
        // now what?
            //assert(0);                // if na fall through to number
        case TypeBase::SHOW_AS_NUMBER:
            {
                Gtk::SpinButton* newButton;

                fixedAdj = new Gtk::Adjustment( entry.CurrentValue(),
                                                entry.Min(),
                                                entry.Max(),
                                                0.001,
                                                (entry.Max()-entry.Min())/20,
                                                0 );

                newButton = new Gtk::SpinButton(*fixedAdj,0,5);
                assert(newButton);

                std::ostringstream temp;
                temp << entry.Min() << "-" << entry.Max();
                AddHelp(*newButton,&temp.str());
                fixedAdj->value_changed.connect(slot(this,&GUIElementFixed::Callback));
                newButton->set_numeric(true);
                //widget = newButton;

                Gtk::Label* label = manage( new Gtk::Label (entry.DisplayName(), 0, 0.5) );
                elementBox->pack_end(*newButton,false);
                elementBox->pack_end(*label,false);
                elementBox->show_all();
                break;
            }
            break;
        case TypeBase::SHOW_AS_SLIDER:  
            {
                fixedAdj = new Gtk::Adjustment( entry.CurrentValue(),
                                                entry.Min(),
                                                entry.Max(),
                                                1.0,
                                                (entry.Max()-entry.Min())/20,
                                                0 );

                Gtk::HScale* newWidget = new Gtk::HScale(*fixedAdj);
                assert(newWidget);

                std::ostringstream temp;
                temp << entry.Min() << "-" << entry.Max();
                AddHelp(*newWidget,&temp.str());
                fixedAdj->value_changed.connect(slot(this,&GUIElementFixed::Callback));

                Gtk::Label* label = manage( new Gtk::Label (entry.DisplayName(), 0, 0.5) );
                elementBox->pack_start (*label);
                elementBox->pack_start (*newWidget);
                elementBox->show_all();
            }
            break;
        case TypeBase::SHOW_AS_TOGGLE:
        case TypeBase::SHOW_AS_DROPMENU:
        case TypeBase::SHOW_AS_RADIOBUTTONS:
            assert(0);
            break;
        case TypeBase::SHOW_AS_HIDDEN:
            break;
        case TypeBase::SHOW_AS_COLOR:     
            assert(0);
            break;
        case TypeBase::SHOW_AS_CHECKBOX:  
        case TypeBase::SHOW_AS_MAILBOX:     
        case TypeBase::SHOW_AS_COMBOBOX:  
        case TypeBase::SHOW_AS_TEXTEDITOR:
            assert(0);
            break;
        default:
            std::cerr << "showas of " << entry.GetShowAs() << " not supported" << std::endl;
            assert(0);
            break;
    }
}


void GUIElementFixed::Callback()
{
    TypeFixed32* entry = (dynamic_cast<TypeFixed32 *>(&oadEntry));
    assert(entry);
    entry->CurrentValue(fixedAdj->get_value());
//    cout << "GUIFixed::callback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;
    GlobalUpdateButtonEnables();
}

//==============================================================================

GUIElementString::GUIElementString(TypeString& entry,Gtk::Box& box) :
GUIElementOAD(entry, box)
{
    textEntry=NULL;

    switch(entry.GetShowAs())
    {
        case TypeBase::SHOW_AS_N_A:
            {
                int length = 20;
                textEntry = new Gtk::Entry(length);
                assert(textEntry);
                textEntry->set_text(entry.CurrentValue());

                Gtk::Label* label = new Gtk::Label(entry.DisplayName());
                assert(label);
                AddHelp(*textEntry);
                textEntry->changed.connect(slot(this, &GUIElementString::Callback));

                elementBox->pack_end(*textEntry,false);
                elementBox->pack_end(*label,false);

            }
            break;
        case TypeBase::SHOW_AS_NUMBER:
        case TypeBase::SHOW_AS_SLIDER:  
        case TypeBase::SHOW_AS_TOGGLE:
        case TypeBase::SHOW_AS_DROPMENU:
        case TypeBase::SHOW_AS_RADIOBUTTONS:
            assert(0);
            break;
        case TypeBase::SHOW_AS_HIDDEN:
            // DO NOTHING
            break;
        case TypeBase::SHOW_AS_COLOR:     
        case TypeBase::SHOW_AS_CHECKBOX:  
        case TypeBase::SHOW_AS_MAILBOX:     
        case TypeBase::SHOW_AS_COMBOBOX:  
            assert(0);
            break;
        case TypeBase::SHOW_AS_TEXTEDITOR:
            {
                Gtk::Button* btn = new Gtk::Button("Edit");
                assert(btn);

                Gtk::Label* label = new Gtk::Label(entry.DisplayName());
                assert(label);
                std::string additionalHelp = "Edit ";
                additionalHelp += entry.DisplayName();
                AddHelp(*btn,&additionalHelp);
                btn->clicked.connect(slot(this, &GUIElementString::EditCallback));

                elementBox->pack_end(*btn,false);
                elementBox->pack_end(*label,false);
            }
            break;
        case TypeBase::SHOW_AS_FILENAME:
            {
                int length = 256;
                textEntry = new Gtk::Entry(length);
                assert(textEntry);
                textEntry->set_text(entry.CurrentValue());
    
                Gtk::Label* label = new Gtk::Label(entry.DisplayName());
                assert(label);
                AddHelp(*textEntry);
                textEntry->changed.connect(slot(this, &GUIElementString::Callback));

                Gtk::Button* btn = new Gtk::Button("FR");
                assert(btn);

                std::string additionalHelp = "File Requestor ";
                additionalHelp += entry.DisplayName();
                AddHelp(*btn,&additionalHelp);
                btn->clicked.connect(slot(this, &GUIElementString::FileReqCallback));

                elementBox->pack_end(*btn,false);
                elementBox->pack_end(*textEntry,false);
                elementBox->pack_end(*label,false);
            }
            break;
        default:
            std::cerr << "showas of " << entry.GetShowAs() << " not supported" << std::endl;
            assert(0);
            break;
    }
}


void GUIElementString::Callback()
{
    TypeString* entry = (dynamic_cast<TypeString*>(&oadEntry));
    assert(entry);

    entry->CurrentValue(textEntry->get_text());
//    cout << "GUIString::callback - " << oadEntry.DisplayName() << " was set to " << entry->CurrentValue() << std::endl;
    GlobalUpdateButtonEnables();
}

//==============================================================================

void GUIElementString::EditCallback()
{
    TypeString* entry = (dynamic_cast<TypeString*>(&oadEntry));
    assert(entry);

    //entry->CurrentValue(textEntry->get_text());
    //cout << "GUIString::EditCallback - " << oadEntry.DisplayName() << std::endl;

    std::string tempFileName = tmpnam(NULL);
    tempFileName += "WorldFoundryAttribEdit.tcl";        // ought to be unique now, add .tcl so that editor will syntac highlight correctly
                                                        // kts: make extension come from the oadentry


    std::string editString =entry->CurrentValue();

    remove(tempFileName.c_str());          // just in case
    {
        std::ofstream out(tempFileName.c_str());
        assert(out.good());
        out.write(editString.c_str(),editString.size());
    }

    std::string commandName;
    pGlobalEditor->GetAttribEditProfileString("programs", "texteditor", commandName);

    std::string command;
    command += commandName;
    command += " ";
    command += tempFileName;

    //cout << "executing " << command << std::endl;
    int result = system(command.c_str());
    assert(result == 0);

    // now read result back from temp file

    std::ifstream in(tempFileName.c_str());
    assert(in.good());
    std::string resultString;
    getline(in, resultString,'\0');
    //cout << "result = " << resultString << std::endl;
    entry->CurrentValue(resultString);

    GlobalUpdateButtonEnables();
}

//==============================================================================

void GUIElementString::FileReqCallback()
{
    TypeString* entry = (dynamic_cast<TypeString*>(&oadEntry));
    assert(entry);

    std::string result = FileRequester(entry->CurrentValue());
    cout << "result = " << result << std::endl;
    entry->CurrentValue(result); 
    textEntry->set_text(entry->CurrentValue());

    GlobalUpdateButtonEnables();
}                 

//==============================================================================

