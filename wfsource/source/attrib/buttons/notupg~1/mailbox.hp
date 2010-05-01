#ifndef MAILBOX_H
#define MAILBOX_H

#include "../oaddlg.h"

class Mailbox : public uiDialog
{
public:
	Mailbox::Mailbox( typeDescriptor* td );
	virtual Mailbox::~Mailbox();

	virtual int make_dialog_gadgets( HWND );
	virtual dataValidation copy_to_xdata( byte* & saveData );

	void reset( byte* & );

	virtual int storedDataSize() const;
	virtual bool enable( bool bEnable );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( long );
	HWND hwndEdit;
	ICustEdit* _edit;
	HWND hwndSpinner;
	ISpinnerControl* _spinner;
	};

#endif	/* MAILBOX_H */
