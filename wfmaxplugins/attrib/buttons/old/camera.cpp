// camera.c

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../types.h"
//#include "../debug.h"
#include "../util.h"
#include "source/oas/oad.h"

#include "dialog.h"

#include "oaddlg.h"
#include "camera.h"
#include "dialogbox.h"
#include "objref.h"
#include "fixed.h"

int
Camera::storedDataSize()
	{
	return -1;
	}


Camera::~Camera()
	{
	assert( _szCameraName );
	free( _szCameraName ), _szCameraName = NULL;
	}


dataValidation
Camera::copy_to_xdata( byte* & saveData, typeDescriptor* td )
	{
	bool bFoundExistingCamera = false;

	int status;

	int nItems;
	pxp_get_item_count( nItems );
	assert( nItems > 0 );

	ItemData id;
	for ( int i=0; i<nItems; ++i )
		{
		pxp_get_item( i, &id, status );
		assert( status == 1 );

//		debug( "item #%d = [%s]", i, id.name );
		if ( strcmp( id.name, _szCameraName ) == 0
		&& id.type == PXPCAMERA )
			{ // Found an existing camera -- update values
//			debug( "Found existing camera--update values" );
			bFoundExistingCamera = true;
			break;
			}
		}

	// Camera name (this object's position)
	id.item.c.x = theDialogBox->xObject();
	id.item.c.y = theDialogBox->yObject();
	id.item.c.z = theDialogBox->zObject();

	// Target box "FollowObject"
	ObjectReference* targetGadget = (ObjectReference*)theDialogBox->findGadget( _td->string, BUTTON_OBJECT_REFERENCE );
	if ( !targetGadget )
		{
		debug( "Follow object field [%s] does not exist", _td->string );
		exit( 0 );
		}

	ItemData* targetId = objectExists( targetGadget->referencesObject() );
	if ( !targetId )
		{
		debug( "Follow object [%s] does not exist", targetGadget->referencesObject() );
		exit( 0 );
		}
	assert( targetId );

	id.item.c.tx = targetId->item.m.matrix[3][0];
	id.item.c.ty = targetId->item.m.matrix[3][1];
	id.item.c.tz = targetId->item.m.matrix[3][2];
//	debug( "object [%s] position @ (%f,%f,%f)",
//		_td->string, targetId->item.c.tx, targetId->item.c.ty, targetId->item.c.tz );

	if ( !bFoundExistingCamera )
		{
		id.item.c.flags = 0;
		id.item.c.focal = 50.0;
		id.item.c.bank = 0.0;
		id.item.c.nearplane = 0.0;
		id.item.c.farplane = 1000.0;

		assert( strlen( _szCameraName ) <= sizeof( id.name ) );
		strcpy( id.name, _szCameraName );
		id.type = PXPCAMERA;

		pxp_create_item( &id, status );
		assert( status == 1 );
		}
	else
		{
		pxp_erase_item( id.name );

		// Change camera values based on the numbers typed in
		Fixed32* fovGadget = (Fixed32*)theDialogBox->findGadget( "FOV", BUTTON_FIXED32 );
		assert( fovGadget );
		id.item.c.focal = int( (*fovGadget)() ) / 65536.0;
//		debug( "fov td.def=%lx  focal=%f", td.def, id.item.c.focal );

		Fixed32* rollGadget = (Fixed32*)theDialogBox->findGadget( "Roll", BUTTON_FIXED32 );
		assert( rollGadget );
		id.item.c.bank = int( (*rollGadget)() ) / 65536.0;

		pxp_change_item( &id, status );
		assert( status == 1 );
		}

	//debug( "redrawing %s", id.name );
	pxp_view_redraw( id.name );
	pxp_draw_item( id.name );

	return DATA_OK;
	}


void
Camera::reset( typeDescriptor* td )
	{
	}


void
Camera::reset( void* & saveData )
	{
	//char* data = (char*)saveData;
	//reset( data );
	//saveData = (void*)( (byte*)saveData + strlen( data ) + 1 );
	}


void
Camera::make_dialog_gadgets( DialogBox* db )
	{
	assert( _td );

//	debug( "Camera::make_dialog_gadgets()" );
//	debug( "string: [%s]", _td->string );

	_szCameraName = strdup( db->objectName );
	assert( _szCameraName );
	*_szCameraName = '~';
	ItemData id;			// just to verify size (below)
	assert( strlen( _szCameraName ) <= sizeof( id.name ) );

	makeLabel( db->dlgFollow, _td->xdata.displayName, db->x, db->y );
	SET_BUTTON_INDEX( db->dlgFollow, db->idxGlue );
	SET_BUTTON_SHEET( db->dlgFollow, db->idxPropertySheet );
	++db->idxGlue;

	db->y += db->yInc;
	}
