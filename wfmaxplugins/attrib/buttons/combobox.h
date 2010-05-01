#ifndef COMBOBOX_H
#define COMBOBOX_H

#include "../oaddlg.h"

class ComboBox : public uiDialog
	{
public:
	ComboBox::ComboBox( typeDescriptor* td );
	virtual ComboBox::~ComboBox();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;
	virtual bool enable( bool bEnable );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( char* );
	HWND hwndMenu;

	int _nItems;
	enum { MAX_MENUITEMS = 300 };
	char* _menuItems[ MAX_MENUITEMS ];
	};

#endif	// COMBOBOX_H

