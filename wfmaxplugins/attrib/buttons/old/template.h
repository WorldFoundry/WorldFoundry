#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "oaddlg.h"

class Template : public uiDialog
	{
public:
	Template::Template( typeDescriptor* td ) : uiDialog( td )	{}
	Template::~Template() 										{}

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* );
	void reset( bool );
	void reset( void* & );
	void refresh()				{}
	void feel( int mouse )		{}
	void see()					{}

	int storedDataSize();

protected:
	Dialog* _dlg;
	};

#endif	/* TEMPLATE_H */
