#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "../oaddlg.h"

class CheckBox : public uiDialog
	{
public:
	CheckBox::CheckBox( typeDescriptor* td );
	virtual CheckBox::~CheckBox();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;
	//virtual void activate( HWND );
	virtual bool enable( bool );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( int );
	HWND hwndCheckbox;
	};

#endif	/* CHECKBOX_H */
