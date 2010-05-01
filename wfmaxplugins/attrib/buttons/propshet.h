#ifndef PROPSHET_H
#define PROPSHET_H

#include "../oaddlg.h"

class RollUp : public uiDialog
	{
public:
	RollUp::RollUp( typeDescriptor* td );
	virtual RollUp::~RollUp();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	int storedDataSize() const;

	virtual bool enable( bool );
	virtual double eval() const;

protected:
	virtual void reset();
	void reset( int32 );
	int _nLinesInRollup;

//	IRollupWindow* _rollup;
//	int _idxRollup;
	};

#endif	/* PROPSHET_H */
