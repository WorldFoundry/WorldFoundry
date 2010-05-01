//==============================================================================
// ccyc.cc:
// Copyright ( c ) 1997,1999,2000,2001 World Foundry Group.  
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

#include <gfx/ccyc.hp>
#include <pigsys/assert.hp>
#include <iff/iffread.hp>
#include <cstring>

//=============================================================================

CCYC::CCYC()
{
	_nEntries = 0;
	_tblCCYC = NULL;
}

//=============================================================================

CCYC::CCYC( void* ppCCYC, int xOffset, int yOffset )
{
	assert( ValidPtr( ppCCYC ) );
	long* pCCYC = (long*)ppCCYC;

	assert( *pCCYC == IFFTAG('c','c','y','c' ));
	++pCCYC;			// Skip 'CCYC'
	assert( *pCCYC > 0 );
	++pCCYC;			// Skip chunk size
	_nEntries = *pCCYC;
	if ( _nEntries )
	{
		++pCCYC;
		_tblCCYC = (_CCYC*)pCCYC;
	}
	else
	{
		_tblCCYC = NULL;
	}

#if 1
	(void)xOffset;
	(void)yOffset;
#else
// loop through all entries, adding in base offset
	for ( int ccycIndex=0; ccycIndex<_nEntries; ++ccycIndex )
	{
		_tblCCYC[ccycIndex].u += xOffset;
		_tblCCYC[ccycIndex].v += yOffset;
	}
#endif
	//cout << "CCYC::CCYC(): " << *this << std::endl;
}

//=============================================================================

//CCYC::~CCYC()
//{
//}

//=============================================================================


void
CCYC::Validate() const
{
	if ( _nEntries )
		assert( ValidPtr( _tblCCYC ) );
	else
		assert( _tblCCYC == NULL );
}

//=============================================================================

_CCYC*
CCYC::GetCCYC( int idx ) const
{
	Validate();
	assert( 0 <= idx && idx < _nEntries );
	return &_tblCCYC[ idx ];
}

//=============================================================================

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& s, _CCYC ccyc )
{
	s << "Colors [" << ccyc.start << ".." << ccyc.end << "] speed=" << ccyc.speed << ' ';
	return s;
}

//=============================================================================

std::ostream&
operator << ( std::ostream& s, CCYC ccyc )
{
	ccyc.Validate();
	s << "CCYC dump:" << std::endl;
	for ( int idxColourCycle=0; idxColourCycle<ccyc._nEntries; ++idxColourCycle )
		s << idxColourCycle << ":" << ccyc._tblCCYC[ idxColourCycle ] << std::endl;
	return s;
}

#endif

//=============================================================================
