#ifndef CAMERA_H
#define CAMERA_H

#include "oaddlg.h"
#include "dialogbox.h"

class Camera : public uiDialog
	{
public:
	Camera::Camera( typeDescriptor* td ) : uiDialog( td )		{}
	Camera::~Camera();

	dataValidation copy_to_xdata( byte* & saveData, typeDescriptor* );
	void make_dialog_gadgets( DialogBox* );
	void reset( typeDescriptor* );
	void reset( void* & );
	void refresh()	{}
	void feel( int mouse )		{}
	void see()			{}

	int storedDataSize();

private:
	char* _szCameraName;
	};

#endif	/* CAMERA_H */
