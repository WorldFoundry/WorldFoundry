#ifndef TIP_H
#define TIP_H

#include "oaddlg.h"

class TipButton : public uiDialog
	{
public:
	TipButton::TipButton( typeDescriptor* td );
	TipButton::~TipButton();

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* )	{}
	void reset( void* & )				{}
	void refresh()					{}
	void feel( int mouse );
	void see()						{}

	void nextTip();
	void previousTip();

	int storedDataSize();

private:
	void drawTipText();
	char _tipsFile[ _MAX_PATH ];
	int _nTips;
	int _nTipOfTheDay;

	Dialog* _dlg;
	};

#endif	/* TIP_H */
