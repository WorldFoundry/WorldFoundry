//=============================================================================
// console.cc
//=============================================================================

#include <console/console.hp>
#include <hal/hal.h>
#include <pigsys/pigsys.hp>

Console::Console()
{
	_pTextBuffer = new TextBuffer( 40, 25 );
	assert( _pTextBuffer );

	clear();
	home();
	flush();
	validate();
}


Console::~Console()
{
	assert( _pTextBuffer );
	delete _pTextBuffer;
}


void
Console::validate() const
{
	// range check cursor (x,y) position
}

void
Console::clear()
{
	_pTextBuffer->clear();
	validate();
}

void
Console::print( const char* buf, ... )
{
	assert( buf );

	static char buffer[ 1024 ];
	va_list arglist;

	va_start( arglist, buf );
	vsprintf( buffer, buf, arglist );
	va_end( arglist );

	char* msg = buffer;
	const char* pMsgEnd = msg + strlen( msg );
	char* pMsg = (char*)msg;
	do
	{
		char* pMsgLinefeed = strchr( pMsg, '\n' );
		if ( pMsgLinefeed )
		{	// Has a \n at end
			int nChars = pMsgLinefeed - pMsg;
			_pTextBuffer->print( _x, _y, pMsg, nChars );
			pMsg += nChars + 1;		// + 1 for '\n'
			_x = 0;
			++_y;
		}
		else
		{
			_pTextBuffer->print( _x, _y, pMsg, strlen( pMsg ) );
			_x += strlen( pMsg );
			pMsg = (char*)pMsgEnd;
		}
	}
	while ( pMsg < (char*)pMsgEnd );
}


void
Console::home()
{
	_x = _y = 0;
}


void
Console::flush()
{
//	UpdateSimpleDisplay();
	validate();
	_pTextBuffer->flush();
	home();
	validate();
}
