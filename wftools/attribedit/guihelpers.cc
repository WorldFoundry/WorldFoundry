//==============================================================================
// guihelpers.cc: helper code
//==============================================================================
                  
#include "guihelpers.hp"
#include <gtk--/window.h>
#include <gtk--/optionmenu.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/entry.h>
#include <gtk--/main.h>
#include <gtk--/fileselection.h>
#include <iostream>
                             
//From <iostream> using std::cout;

using SigC::slot;
                                                    
//==============================================================================

LabeledOptionMenu::LabeledOptionMenu(const Gtk::string &menutitle,
				     Gtk::Menu *menu,
				     bool homogeneous,
				     gint spacing) :
    Gtk::HBox(homogeneous, spacing),
    m_label(menutitle),
    m_menu(menu)
{
  pack_start(m_label, false, false, 0);
  om=manage(new Gtk::OptionMenu);
  om->set_menu(m_menu);
  pack_start(*om);
}

//==============================================================================

class filesel: public Gtk::FileSelection
{
public:
    filesel(const std::string& def);
    void run();
    std::string result;
private:
    /* Get the selected filename and print it to the console */
    void file_ok_sel() {
	std::cout << "file_ok_sel: " << get_filename() << std::endl;
    result = get_filename();
	Gtk::Main::quit();  
    }
    gint delete_event_impl(GdkEventAny*) { 
	Gtk::Main::quit(); return 0; 
    }
};

filesel::filesel(const std::string& def):
    Gtk::FileSelection("File selection")
{

   result = def; 
   set_filename(def);

    /* Connect the ok_button_ to file_ok_sel function */
    get_ok_button()->clicked.connect(slot(this, &filesel::file_ok_sel));
    /* Connect the cancel_button_ to hiding the window */
    get_cancel_button()->clicked.connect(hide.slot());
    /* Connect hiding the window to exit the program */
    hide.connect(Gtk::Main::quit.slot());
}


void
filesel::run()
  {
    Gtk::Kit::run();
  }

//==============================================================================

class MyDialog: public Gtk::Window
  {
      Gtk::Entry* entry;
      bool canceled;
      void okay();
      void cancel();
      gint key(GdkEventKey* key);
    public:
      MyDialog(const std::string &label);
      ~MyDialog();
      std::string run();
      gint delete_event_impl(GdkEventAny*) {return true;}
  };

MyDialog::MyDialog(const std::string &l)
  : canceled(false)
  {
    set_modal(true);

    entry=manage(new Gtk::Entry);
    entry->key_press_event.connect(slot(*this,&MyDialog::key));

    Gtk::Button *okay_b=manage(new Gtk::Button("Okay"));
    Gtk::Button *cancel_b=manage(new Gtk::Button("Cancel"));
    okay_b->clicked.connect(slot(*this,&MyDialog::okay));
    cancel_b->clicked.connect(slot(*this,&MyDialog::cancel));

    Gtk::Box *box2=manage(new Gtk::HBox());
    box2->pack_start(*okay_b,false,false,10);
    box2->pack_end(*cancel_b,false,false,10);

    Gtk::Box *box=manage(new Gtk::VBox());
    box->pack_start(*manage(new Gtk::Label(l)),true,true,10);
    box->pack_start(*entry,true,true,10);
    box->pack_start(*box2,true,true,10);
    add(*box);
    set_border_width(10);
    set_usize(200,150);
    show_all();
  }

MyDialog::~MyDialog() {}

std::string MyDialog::run()
  {
    Gtk::Kit::run();
    if (canceled) 
      return "";
    return entry->get_text(); 
  }

void MyDialog::cancel()
  {
    canceled=true;
    Gtk::Kit::quit();
  }

void MyDialog::okay()
  {
    Gtk::Kit::quit();
  }

gint MyDialog::key(GdkEventKey* ke)
  {
    if (ke&&ke->keyval==GDK_Return) Gtk::Kit::quit();
    return false;
  }

/******************************************************/
  
std::string
FileRequester(const std::string& current)
{
   /* Create a new file selection widget */
   filesel filew(current);
   filew.show();
   filew.run();

   return filew.result;
}

//==============================================================================

void pop_dialog()
  {

    MyDialog dialog("What is your name?");
    std::cout << dialog.run() <<std::endl;
  }

//==============================================================================


