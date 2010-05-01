//==============================================================================
// rmuv.cc:
// Copyright ( c ) 1997,1998,1999,2000,2001 World Foundry Group.  
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

#include <gfx/rmuv.hp>
#include <pigsys/assert.hp>
#include <iff/iffread.hp>

//=============================================================================

RMUV::RMUV( void* ppRMUV, int xOffset, int yOffset )
{
	assert( ValidPtr(ppRMUV ));
	AssertMsg(sizeof(_RMUV) == 48,"size = " << sizeof(_RMUV));
	long* pRMUV = (long*)ppRMUV;

	assert(*pRMUV == IFFTAG('r','m','u','v'));
	++pRMUV;			// Skip 'RMUV'
	assert(*pRMUV  > 0);
	++pRMUV;			// Skip chunk size
	_nEntries = *pRMUV;
	if ( _nEntries )
	{
		++pRMUV;
		_tblRMUV = (_RMUV*)pRMUV;
	}
	else
	{
		_tblRMUV = NULL;
	}

// loop through all entries, adding in base offset
	for ( int ruvIndex=0; ruvIndex<_nEntries; ++ruvIndex )
	{
		_tblRMUV[ruvIndex].u += xOffset;
		_tblRMUV[ruvIndex].v += yOffset;
	}
}

//=============================================================================

_RMUV*
RMUV::GetRMUV( const char* szFilename, int /*idxMaterial*/ ) const
{
	Validate();
	for ( int i=0; i<_nEntries; ++i )
	{
		if ( strcmp( szFilename, _tblRMUV[ i ].szTextureName ) == 0 )
			return &_tblRMUV[ i ];
	}

   // kts for now, should probably remove this later
	for ( int index=0; index<_nEntries; ++index )
	{
		if ( strcasecmp( szFilename, _tblRMUV[ index ].szTextureName ) == 0 )
      {
         DBSTREAM1( cwarn << "returning different case match for texture " << szFilename << " ( returned " << _tblRMUV[ index ].szTextureName << " )" << std::endl; )
			return &_tblRMUV[ index ];
      }
	}



	AssertMsg(0,"texture named " << szFilename << " not found in " << *this);
	return 0;
}

//=============================================================================

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& s, _RMUV rmuv )
{
	s << rmuv.szTextureName << ' ';
	//s << "flags=" << int( rmuv.flags ) << ' ';
//	s << "_pad=" << int( rmuv._pad ) << " (should be zero)";
	s << "nFrame=" << rmuv.nFrame << ' ';
	s << "width=" << rmuv.w << ' ';
	s << "height=" << rmuv.h << ' ';
	s << "pal=(" << rmuv.palx << ',' << rmuv.paly << ')' << std::endl;
	s << "bTranslucent = " << bool( rmuv.bTranslucent ) << ' ';
	s << "bitdepth = " << std::dec << rmuv.bitdepth;
	s << ", uv=(" << rmuv.u << ',' << rmuv.v << ") ";

	return s;
}

//=============================================================================

std::ostream&
operator << ( std::ostream& s, RMUV rmuv )
{
	rmuv.Validate();
	s << "RMUV dump:" << std::endl;
	for ( int index=0; index<rmuv._nEntries; ++index )
		s << index << ":" << rmuv._tblRMUV[ index ] << std::endl;
	return s;
}

#endif

//=============================================================================
