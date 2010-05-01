#include <stdarg.h>
#include <windowsx.h>
#include "select.h"

////////////////////////////////////////////////////////////////////////////////

extern Class_ID plugInWorldFoundryAttrib;

static const char*
ClassName( INode* pObject )
{
	assert( pObject );

	static char szClassName[ 64 ];

	AppDataChunk* adClass = pObject->GetAppDataChunk(
		Attrib_ClassID, UTILITY_CLASS_ID, 0 );

	strcpy( szClassName, adClass ? (char*)adClass->data : DEFAULT_CLASS "*" );

	return szClassName;
}


void
_eval( const char* szExpression, char* szValue )
{
	char szObjectName[ 100 ];
	char szAttribute[ 100 ];

	char* pPeriod = strchr( szExpression, '.' );
	assert( pPeriod );

	// name
	strcpy( szObjectName, szExpression );
	*( strchr( szObjectName, '.' ) ) = '\0';

	// attribute
	strcpy( szAttribute, pPeriod+1 );

	assert( theSelect.ip );
	INode* pNode = theSelect.ip->GetINodeByName( szObjectName );
	assert( pNode );

	if ( strcmp( szAttribute, "!Name" ) == 0 )
	{
		strcpy( szValue, pNode->GetName() );
	}
	else if ( strcmp( szAttribute, "!Class" ) == 0 )
	{
		strcpy( szValue, ClassName( pNode ) );
	}
	else if ( strcmp( szAttribute, "!Selected" ) == 0 )
	{
		sprintf( szValue, "%d", bool( pNode->Selected() ) );
	}
	else
	{
		char szClassName[ 100 ];
		strcpy( szClassName, ClassName( pNode ) );
		strcat( szClassName, ".oad" );

		std::vector< Oad* >::iterator pOad;
		for ( pOad=theSelect._oad.begin(); pOad != theSelect._oad.end(); ++pOad )
		{
			if ( _stricmp( szClassName, (*pOad)->ClassName() ) == 0 )
				break;
		}

		if ( pOad != theSelect._oad.end() )
		{
			strcpy( szValue, (*pOad)->find( pNode, szAttribute ) );
		}
		else
		{
//			strcpy( szValue, szClassName );
//			strcat( szValue, " not found" );
			*szValue = '\0';
		}
	}
}


Item::Item( HWND hwnd, const char* szExpression, INode* pObject, int i, int col, LPARAM lParam )
{
	assert( pObject );

	assert( szExpression );
	_szExpression = _strdup( szExpression );
	assert( _szExpression );

	static char szText[ 32768 ];
	_eval( _szExpression, szText );
	_szText = _strdup( szText );
	assert( _szText );

	_col = col;

	if ( col == 0 )
	{
		LV_ITEM lvl;

		lvl.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
		lvl.state = pObject->Selected() ? LVIS_SELECTED : 0;
		lvl.stateMask = 0;

		lvl.iItem = i;
		lvl.iSubItem = 0;
		lvl.iImage = 0;
		lvl.pszText = LPSTR_TEXTCALLBACK;

		lvl.cchTextMax = sizeof( _szText );
		lvl.cchTextMax = 255;

		lvl.lParam = (LPARAM)i;
		//lvl.lParam = (LPARAM)this;
		//lvl.lParam = (LPARAM)( theSelect._columns[ col ] );
		//assert( lvl.lParam );

		int error = ListView_InsertItem( hwnd, &lvl );
		assert( error != -1 );
	}
	else
	{
		ListView_SetItemText( hwnd, i, col, LPSTR_TEXTCALLBACK );
	}
}


const char*
Item::GetText() const
{
	assert( _szText );
	return _szText;
}


Item::~Item()
{
	assert( _szExpression );
	free( _szExpression );

	assert( _szText );
	free( _szText );
}
