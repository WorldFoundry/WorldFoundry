//==============================================================================
// function.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include <pigsys/assert.hp>
#include "grammar.hp"
#include "airun.hp"
extern bool bVerbose;
#include "obj/tokens_tab.h"
#include <math/scalar.hp>

extern aiGrammar* theGrammar;


Function::Function( const string& szName ) :
	_szName( szName )
{
}


Function::~Function()
{
	// TODO: erase lineBuffer
}


void
Function::out_int32( int32 val )
{
	theGrammar->parseLog() << "out_int32: linebuffersize: " << hex << lineBuffer.size() << "[</i>$" << hex << val << dec << "</i>] ";
	lineBuffer.push_back( val );
}


void
Function::out_opcode( int32 opcode, const char* szOpcodeName )
{
	int32 outOpcode = (opcode << 16) | OPCODE_NAME;

	theGrammar->parseLog() << "<table border=1 width=100%><tr><td bgcolor=\"white\"><font color=\"black\">";
	theGrammar->parseLog() << "<code><b>" << szOpcodeName << "</b></code> ";
	theGrammar->parseLog() << hex << lineBuffer.size() << "[</i>$" << hex << opcode << dec << "</i>] ";
	out_int32( outOpcode );
}


void
Function::end_opcode()
{
	theGrammar->parseLog() << "</td></tr></table>" << endl;
}


void
Function::out_class( int32 class_name )
{
	theGrammar->parseLog() << "<i>CLASS</i>=&quot;" << class_name << "&quot; ";
	assert( 0 <= class_name && class_name <= 0xFFFF );
#pragma message( "probably just output class as a regular scalar" )
	class_name <<= 16;
	out_int32( class_name );
}


void
Function::out_actor( const char* szActorName )
{
	if ( strcmp( szActorName, "self" ) == 0 )
		out_opcode( SELF, "self" );
	else
	{
		Symbol* symbol = theGrammar->find_mailbox( szActorName );
		if ( !symbol )
			symbol = theGrammar->find_symbol( szActorName );
		if ( !symbol )
			theGrammar->Error( "Undefined symbol \"%s\"", szActorName );

		theGrammar->parseLog() << "<i>" << szActorName << "</i> ";
		if ( symbol )
			theGrammar->parseLog() << "<font color=\"purple\">" << symbol->nSymbolValue << "</font> ";
		else
			theGrammar->parseLog() << "[undefined] ";

		Scalar actor = symbol ? symbol->nSymbolValue : Scalar::zero;
		out_int32( actor.AsLong() );
	}
}


void
Function::out_number( Scalar _number, const char* szNumberName )
{
	int32 number = _number.AsLong();
	number &= ~3;					// lose 2 bits on fractional part
	out_int32( number );
}


void
Function::out_string( const char* szString )
{
	int offset = theGrammar->_stringTable.add( szString );
	out_int32( (offset<<2) | STRING_NAME );
	theGrammar->parseLog() << "<font color=\"#008000\">&quot;<i>" << szString << "</i>&quot;</font>";
}
