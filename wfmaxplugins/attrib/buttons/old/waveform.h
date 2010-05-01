#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "oaddlg.h"
#include "dialogbox.h"

class Waveform : public uiDialog
	{
public:
	Waveform::Waveform( typeDescriptor* td ) : uiDialog( td )		{}
	Waveform::~Waveform() 											{}

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* );
	void reset( void* & );
	void refresh()	{}
	void feel( int mouse );
	void see();

	int storedDataSize();

protected:
	void drawNode( int nNode, int colorLine, int colorNode );
	int hitKnob( int x, int y, int radius );
	void drawKnob( int x, int y, int radius, int color );
	void check_xbounds( int nKnob );
	void check_ybounds( int nKnob );
	enum { _nNodes = 4 };
	int _x[_nNodes],_y[_nNodes];

	int _xWaveform, _yWaveform;

	int _wScreen, _hScreen;
	int xMouse, yMouse;
	int xScrMouse, yScrMouse;
	};

#endif	/* WAVEFORM_H */
