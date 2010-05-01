#ifndef FILENAME_H
#define FILENAME_H

#include "../oaddlg.h"

class Filename : public uiDialog
{
public:
	Filename::Filename( typeDescriptor* td );
	virtual Filename::~Filename();

	dataValidation copy_to_xdata( byte* & saveData );
	int make_dialog_gadgets( HWND );
	void reset( byte* & );

	virtual void activate( HWND );

	int storedDataSize() const;

	virtual bool enable( bool bEnabled );
	virtual double eval() const;

protected:
	bool validReference( const char* szFilename ) const;

	virtual void reset();
	void reset( char* );
	HWND hwndEdit;
	ICustEdit* _edit;
	HWND hwndBrowseButton;
};

#endif	/* FILENAME_H */
