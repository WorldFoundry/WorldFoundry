#ifndef GROUP_H
#define GROUP_H

#include "../oaddlg.h"

class Group : public uiDialog
{
public:
	Group::Group( typeDescriptor* td );
	virtual Group::~Group();

	virtual dataValidation copy_to_xdata( byte* & saveData );
	virtual int make_dialog_gadgets( HWND );
	void reset( byte* & );
	//virtual void composeHelp( char* entries[] );

	virtual int storedDataSize() const;
	virtual bool enable( bool bEnabled );
	virtual double eval() const;

	void endGroup();

protected:
	virtual void reset();
	HWND hwndGroup;
	int _yStart;
	enum { _GROUP_X_INC = 5 };
};

#endif	/* GROUP_H */

