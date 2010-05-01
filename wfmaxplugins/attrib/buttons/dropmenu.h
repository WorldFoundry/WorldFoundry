#ifndef DROPMENU_H
#define DROPMENU_H

#include "../oaddlg.h"

class DropMenu : public uiDialog
	{
public:
	DropMenu::DropMenu( typeDescriptor* td );
	virtual DropMenu::~DropMenu();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;

	virtual bool enable( bool bEnabled );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( int );
	HWND hwndMenu;

	int _nItems;
	enum { SEPARATOR = '|' };
	enum { MAX_MENUITEMS = 30 };
	char* _menuItems[ MAX_MENUITEMS ];
	};

#endif
