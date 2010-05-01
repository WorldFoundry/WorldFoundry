#ifndef INT_H
#define INT_H

#include "../oaddlg.h"

class Int : public uiDialog
{
public:
	Int::Int( typeDescriptor* td );
	virtual Int::~Int();

	virtual int make_dialog_gadgets( HWND );
	virtual dataValidation copy_to_xdata( byte* & saveData );

	void reset( byte* & );

	//virtual void composeHelp( char* entries[] );

	virtual int storedDataSize() const;

	virtual bool enable( bool );
	virtual double eval() const;

protected:
	void reset( char* data );
	virtual void reset();
	void reset( long );
	HWND hwndEdit;
	ICustEdit* _edit;
	HWND hwndSpinner;
	ISpinnerControl* _spinner;
	};


#if 0
class Int8 : public Int
	{
public:
	Int8::Int8( typeDescriptor* td ) : Int( td )		{}
	Int8::~Int8() 										{}
	};


class Int16 : public Int
	{
public:
	Int16::Int16( typeDescriptor* td ) : Int( td )		{}
	Int16::~Int16() 										{}
	};
#endif


class Int32 : public Int
	{
public:
	Int32::Int32( typeDescriptor* td );
	Int32::~Int32() 										{}
	};

#endif
