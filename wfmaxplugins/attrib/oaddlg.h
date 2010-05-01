#ifndef OADDLG_H
#define OADDLG_H

#include <assert.h>
#include <string.h>

#include "types.h"
#include "oas/oad.h"
#include "attrib.h"
#include <iffwrite.hp>

#undef TOOLTIPS

#define LAST_BUTTON_WIDTH( __x__ )	( 155 - (__x__) )

enum dataValidation
{
	DATA_OK,
	DATA_ERROR
};

class uiDialog
{
public:
	uiDialog::uiDialog( typeDescriptor* td );
	virtual uiDialog::~uiDialog();

	bool canSave() const;

	virtual void activate( HWND );
	virtual dataValidation copy_to_xdata( byte* & saveData );
	virtual void reset( byte* & ) = 0;
	virtual int make_dialog_gadgets( HWND hPanel );
	virtual int storedDataSize() const;

	bool enable() const;
	virtual bool enable( bool );
	bool checkEnable();
	virtual double eval() const = 0;

#if 0
	virtual void composeHelp( char* entries[] )
	{
		if ( *_td->helpMessage )
			entries[ 0 ] = strdup( _td->helpMessage );
		else
			entries[ 0 ] = strdup( "(No help available)" );
		assert( entries[ 0 ] );
	}
#endif

	const char* fieldName() const;
	const char* variableName() const;

	_IffWriter& _print( _IffWriter& ) const;

	typeDescriptor* _td;

	bool _bUserOverrideEnable;
protected:
	bool _bLastEnable;
	HWND _hwndMenuButton;

protected:
	bool _bEnabled;
	virtual void reset() = 0;
	bool _bSave;
	HWND hwndLabel;
	HWND _hPanel;
#if defined( TOOLTIPS )
	ICustButton* _label;
#endif
	char _szVariableName[ 64 ];

private:
	uiDialog::uiDialog()						{}
};


inline _IffWriter&
operator << ( _IffWriter& iff, const uiDialog& ui )
{
	return ui._print( iff );
}

#if 0
// declared in velocity/levels/oas/oad.h
// ** duplicate reference copy here for convience only
typedef struct _typeDescriptor
{
	buttonType type;				// float, int, fixed, string
	int32	cbSize;

	char name[32];					// label (and structure name)

	int32 min;				// ranged numbers
	int32 max;              // ranged numbers
	int32 def;			// number

	int16 len;				// string only
	char string[512];

	visualRepresentation showAs;

	int16 x, y;

	char helpMessage[ 128 ];

	char pad[ 256 ];
} typeDescriptor;
#endif

#endif	/* OADDLG_H */
