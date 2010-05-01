#ifndef HELP_H
#define HELP_H

#include "oaddlg.h"

class HelpButton : public uiDialog
	{
public:
	HelpButton::HelpButton( typeDescriptor* td ) : uiDialog( td )	{}
	HelpButton::~HelpButton();

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* )	{}
	void reset( void* & )			{}
	void refresh()					{}
	void feel( int mouse );
	void see()						{}

	int storedDataSize();

private:
	Dialog* _dlg;
	};

#endif	/* HELP_H */

















