#ifndef CLASSREF_H
#define CLASSREF_H

#include "../oaddlg.h"

class ClassReference : public uiDialog
	{
public:
	ClassReference::ClassReference( typeDescriptor* td );
	virtual ClassReference::~ClassReference();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );
	double eval() const;
	virtual bool enable( bool );

	int storedDataSize() const;

protected:
	virtual void reset()	{ /* No default in _typeDescriptor */ }
	void reset( char* );
	HWND hwndMenu;
	};

#endif
