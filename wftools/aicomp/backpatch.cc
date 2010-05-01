//==============================================================================
// backpatch.cc: Copyright (c) 1996-1999, World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//==============================================================================

#include <cassert>
#include <cstdio>
#include <stdarg.h>
#include <fstream>

#include "grammar.hp"
#include "ailexer.hp"
#include "function.hp"
extern aiFlexLexer* theLexer;
extern bool bVerbose;
extern bool bDebug;

////////////////////////////////////////////////////////////////////////////////

//Backpatch::Backpatch( int nLine, int nOffset )
Backpatch::Backpatch( Function* fn )
{
	assert( fn );
	_fn = fn;
	_nOffset = _fn->lineBuffer.size();
}


Backpatch::Backpatch()
{
	assert( 0 );
}


Backpatch::~Backpatch()
{
}


void
Backpatch::doBackpatch( Function* fn )
{
	assert( fn );
	assert( fn == _fn );

	_fn->lineBuffer[ _nOffset ] = ( fn->lineBuffer.size() - _nOffset - 1 );
}


#if 0
void
Backpatch::backpatch( int nEndLine, int nEndOffset )
{
#if 0
	int32* pProgramLine;

	if ( _nLine == theGrammar->nProgramLines )
		pProgramLine = theGrammar->lineBuffer;
	else
		pProgramLine = theGrammar->program[ _nLine ];

	if ( bDebug )
		cout << "backpatch @ " << nEndLine << ',' << nEndOffset << " for " << _nLine << ',' << _nOffset << endl;

	assert( pProgramLine );
	*( pProgramLine + _nOffset ) = (nEndOffset - _nOffset - 1) << 16;
#endif
}
#endif
