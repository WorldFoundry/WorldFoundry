#ifndef COLORPCK_H
#define COLORPCK_H

#include "../oaddlg.h"

class ColorPicker : public uiDialog
	{
public:
	ColorPicker::ColorPicker( typeDescriptor* td );
	virtual ColorPicker::~ColorPicker();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;
	virtual bool enable( bool bEnabled );
	virtual double eval() const;

private:
	virtual void reset();
	void reset( int32 );
	HWND hwndColor;
	IColorSwatch* _color;
	COLORREF _colorRef;
	};

#endif
