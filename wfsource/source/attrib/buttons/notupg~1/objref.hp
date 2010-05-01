#ifndef OBJREF_H
#define OBJREF_H

#include "../oaddlg.h"

class ObjectReference : public uiDialog
	{
public:
	ObjectReference::ObjectReference( typeDescriptor* td );
	virtual ObjectReference::~ObjectReference();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	virtual void activate( HWND );

	int storedDataSize() const;
	virtual bool enable( bool );
	virtual double eval() const;

protected:
	bool validReference( const char* szObjectReference ) const;
	bool updateGoToObjectButton() const;

private:
	virtual void reset();
	void reset( char* );
	HWND hwndEdit;
	ICustEdit* _edit;
	HWND hwndBrowseButton;
	HWND hwndGoToObjectButton;
	};

#endif
