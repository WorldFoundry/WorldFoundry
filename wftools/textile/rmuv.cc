//==============================================================================
// rmuv.cc
// Copyright (c) 1995-1999, World Foundry Group  
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

#include "global.hp"

#include "rmuv.hp"
#include <iostream>
#include <pigsys/assert.hp>
#include <string>

RMUV::RMUV( void* ppRMUV )
{
	assert( ppRMUV );
	long* pRMUV = (long*)ppRMUV;

	// TODO: assert( *pRMUV == ID( 'RMUV' ) );
	++pRMUV;			// Skip 'RMUV'
	++pRMUV;			// Skip chunk size
	_nEntries = *pRMUV;
	if ( _nEntries )
	{
		++pRMUV;
		_tblRMUV = (_RMUV*)pRMUV;
	}
	else
		_tblRMUV = 0;
}


RMUV::~RMUV()
{
}


_RMUV*
RMUV::GetRMUV( const char* szFilename, int idxMaterial ) const
{
	for ( int i=0; i<_nEntries; ++i )
	{
		if ( strcmp( szFilename, _tblRMUV[ i ].szTextureName ) == 0 )
			return &_tblRMUV[ i ];
	}

	return NULL;
}



ostream&
operator << ( ostream& s, const _RMUV rmuv )
{
	s << rmuv.szTextureName << ' ';
	s << "nFrame=" << int( rmuv.nFrame ) << ' ';
	s << "(u,v)=(" << rmuv.u << ',' << rmuv.v << ") ";
	s << "width=" << rmuv.w << ' ';
	s << "height=" << rmuv.h << ' ';
	s << "bitdepth=" << rmuv.bitdepth << ' ';
	s << "pal=(" << rmuv.palx << ',' << rmuv.paly << ')' << endl;

	return s;
}
