#ifndef XDATA_H
#define XDATA_H

#include "../oaddlg.h"

class XData : public uiDialog
	{
public:
	XData::XData( typeDescriptor* td );
	virtual XData::~XData();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );
	void activate( HWND );

	int storedDataSize() const;
	virtual bool enable( bool bEnabled );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( char* );
	HWND hwndButton;
	char* _xdata;
	ICustButton* _button;
	};

#endif	/* XDATA_H */
