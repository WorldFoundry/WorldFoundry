#ifndef CYCLE_H
#define CYCLE_H

#include "../oaddlg.h"

class CycleButton : public uiDialog
	{
public:
	CycleButton::CycleButton( typeDescriptor* td );
	virtual CycleButton::~CycleButton();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;
	virtual void activate( HWND );

	virtual bool enable( bool );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( int );
	HWND hwndMenu;

	int _selected;
	int _nItems;
	enum { SEPARATOR = '|' };
	enum { MAX_MENUITEMS = 30 };
	char* _menuItems[ MAX_MENUITEMS ];
	};

#endif	/* CYCLE_H */
