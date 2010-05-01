#ifndef STRING_H
#define STRING_H

#include "../oaddlg.h"


class String : public uiDialog
{
public:
	String::String( typeDescriptor* td );
	virtual String::~String();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

//	virtual void activate( HWND );

	int storedDataSize() const;
	virtual bool enable( bool );
	virtual double eval() const;

protected:
//	bool validReference( const char* szObjectReference ) const;

private:
	virtual void reset();
	void reset( char* );
	HWND hwndEdit;
	ICustEdit* _edit;
};

#endif	// STRING_H

