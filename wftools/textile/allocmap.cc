//==============================================================================
// allocmap.cc
// By William B. Norris IV
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
 
#include <string>
#include <iostream>
#include <fstream>

#include "allocmap.hp"
#include "bitmap.hp"
#include "textile.hp"

//#include "memcheck.h"

extern bool bDebug;

AllocationMap::AllocationMap( const AllocationRestrictions& ar )
{
	assert( ar.width > 0 );
	assert( ar.height > 0 );

	_x_slots = ar.width;
	_y_slots = ar.height;

	_x_align = ar.x_align;
	_y_align = ar.y_align;

	_bAlignToSizeMultiple = ar.bAlignToSizeMultiple;

	_theTable = new Allocation[ _x_slots * _y_slots ];
	assert( _theTable );

	_xMaxAllocated = 0;
	_yMaxAllocated = 0;

	_k = ar.k; 	                        // hack

	reset();
}


AllocationMap::~AllocationMap()
{
	delete[] _theTable;
}


void
AllocationMap::reset()
{
	if ( bDebug )
	{
		cout << "_theTable: " << _theTable << endl;
		cout << _x_slots << " by " << _y_slots << " = " << _x_slots * _y_slots << endl;
	}

	for ( int i=0; i<_x_slots * _y_slots; ++i )
		_theTable[ i ] = UNALLOCATED;
}


int
AllocationMap::xExtent() const
{
	return _xMaxAllocated + 1;
}


int
AllocationMap::yExtent() const
{
	return _yMaxAllocated + 1;
}


bool
AllocationMap::checkIfFits( Bitmap* out_bitmap, Bitmap* srcBitmap,
	int x, int y, int srcWidth, int srcHeight )
{
	if ( srcWidth + x > _x_slots )
		return false;

	if ( srcHeight + y > _y_slots )
		return false;

	if ( _bAlignToSizeMultiple && ( (x % srcWidth) != 0 ) )
		return false;

	if ( _bAlignToSizeMultiple && ( (y % srcHeight) != 0 ) )
		return false;

	for ( int yy=y; yy<y+srcHeight; ++yy )
	{
		for ( int xx=x; xx<x+srcWidth; ++xx )
		{
			if ( _theTable[ yy*_x_slots + xx ] != UNALLOCATED )
				return false;
		}
	}

	return true;
}


void
AllocationMap::mark( const int x, const int y, const int srcWidth, const int srcHeight, const int i )
{
	for ( int yAlloc=0; yAlloc<srcHeight; ++yAlloc )
	{
		for ( int xAlloc=0; xAlloc<srcWidth; ++xAlloc )
		{
			assert( x+xAlloc < _x_slots );
			assert( y+yAlloc < _y_slots );

			_theTable[ (y+yAlloc)*_x_slots + (x+xAlloc) ] = i;

			if ( x+xAlloc > _xMaxAllocated )
				_xMaxAllocated = x+xAlloc;
			if ( y+yAlloc > _yMaxAllocated )
				_yMaxAllocated = y+yAlloc;
		}
	}
}


bool
AllocationMap::find( Bitmap* out_bitmap, Bitmap* srcBitmap, int i, int& x, int& y )
{
	int xBitmapAdj = round( srcBitmap->width(), _x_align );
	int yBitmapAdj = round( srcBitmap->height(), _y_align );
	assert( ( xBitmapAdj % _x_align ) == 0 );
	assert( ( yBitmapAdj % _y_align ) == 0 );

	// want 1 entry for 4-bit, 2 for 8-bit, and 4 for 16-bit
	int srcWidth = (xBitmapAdj / _x_align) * (srcBitmap->bitdepth()/(16/4));
	int srcHeight = yBitmapAdj / _y_align;

	if ( bDebug )
		cout << "Looking for entry of " << srcWidth << " slots by " << srcHeight << " slots" << endl;

	for ( y=0; y<_y_slots; ++y )
	{
		for ( x=0; x<_x_slots; x += (16/srcBitmap->bitdepth()) )
		{
			if ( checkIfFits( out_bitmap, srcBitmap, x, y, srcWidth, srcHeight ) )
			{
				mark( x, y, srcWidth, srcHeight, i );
				return true;
			}
		}
	}
	return false;
}


void
AllocationMap::print() const
{
	Allocation* pFollow = _theTable;

	*textilelog << "<pre>" << endl;
	// Show allocation map
	for ( int y=0; y<yExtent(); ++y )
	{
		for ( int x=0; x<xExtent(); ++x, ++pFollow )
		{
			if ( *pFollow == UNALLOCATED )
				*textilelog << ".";
			else if ( *pFollow & 0x8000 )
				*textilelog << '*';
			else
				*textilelog << char( *pFollow + '0' );
		}
		*textilelog << endl;
	}
	*textilelog << "</pre>" << endl;
}
