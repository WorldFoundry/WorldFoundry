#ifndef RADIO_H
#define RADIO_H

#include "oaddlg.h"

class RadioButton : public uiDialog
	{
public:
	RadioButton::RadioButton( typeDescriptor* td ) : uiDialog( td )	{}
	RadioButton::~RadioButton();

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* );
	void reset( int );
	void reset( void* & );
	void refresh()				{}
	void feel( int mouse )		{}
	void see()					{}

	int storedDataSize();

protected:
	int radioVariable;
	int _nItems;
	enum { SEPERATOR = '|' };
	enum { MAX_MENUITEMS = 30 };
	char* _menuItems[ MAX_MENUITEMS ];
	};

#endif	/* RADIO_H */
