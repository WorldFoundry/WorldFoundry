//=============================================================================
// Copyright ( c ) 1995,96,97,99,2000 World Foundry Group  
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
// ===========================================================================

#include "global.hp"
#include "scriptinterpreter_perl.hp"

#include <EXTERN.h>
#include <perl.h>

static PerlInterpreter* _perl;

ScriptInterpreterPerl::ScriptInterpreterPerl() : _ScriptInterpreter()
{
  _perl = perl_alloc();
  assert( ValidPtr( _perl ) );
  perl_construct( _perl );

  static char* embedding[] = { "", "-e", "0" };
  perl_parse( _perl, NULL, 3, embedding, NULL );
  // assert( code == ? );

  perl_run( _perl );
}


ScriptInterpreterPerl::~ScriptInterpreterPerl()
{
  assert( ValidPtr( _perl ) );
  perl_destruct( _perl );
  perl_free( _perl );
}


Scalar
ScriptInterpreterPerl::Eval( const char* szScript )
{
  SV* val = eval_pv( szScript, TRUE );
  STRLEN n_a;
  float f = atof( SvPV( val, n_a ) );
  return Scalar( int32( f * 65536 ) );
}


