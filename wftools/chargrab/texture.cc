//==============================================================================
// texture.cc: Copyright (c) 1996-1999, World Foundry Group  
// Reads in list of 16-bit Targa files and constructs a packed bitmap of
// all of the textures, along with mapping coordinates.
// By William B. Norris IV
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
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdio.h>
#if defined(__WIN__)
#include <io.h>
#include <direct.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <malloc.h>

#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

#include "textile.hp"
#include "types.h"
#include "profile.hp"
#include "bitmap.hp"
#include "allocmap.hp"
#include "rmuv.hp"
#include "ccyc.hp"


#if !defined(__WIN__)
#define O_BINARY 0
#endif

bool bGlobalError = false;

extern char szOutFile[];
extern char* szIniFile;
extern int x_align;
extern int y_align;
extern int pal_x_page;
extern int pal_y_page;
extern int pal_x_align;
extern int pal_y_align;

extern bool bDebug;
extern bool bVerbose;
extern bool bFrame;
extern bool bShowAlign;
extern bool bShowPacking;
extern bool bCropOutputImage;
extern bool bAlignToSizeMultiple;
extern TargetSystem ts;

char* locateFile( const char* filename, const char* _path, const char* _anotherDirectory=NULL );

extern char szOutDir[];
extern char szTexturePath[];
extern char szVrmlPath[];
char szObjectName[ _MAX_FNAME ];

vector<Bitmap*> tblTextures;

bool
inTextureList( vector<string>& tblTextureNames, const string& newTexture )
{
	string* str = find( tblTextureNames.begin(), tblTextureNames.end(), newTexture );
	return bool( str != tblTextureNames.end() );
}

const int MATERIAL_NAME_LEN = 256;
typedef int32 Color;

struct _MaterialOnDisk
{
	int32 _materialFlags;
	Color _color;
	char textureName[ MATERIAL_NAME_LEN ];
};


void
parseMODL( long* pIff32, int32& len, vector<string>& tblTextureNames )
{
	while ( len > 0 )
	{
		int32 tag = *pIff32++;
		int32 size = *pIff32++;

		if ( bVerbose  )
		{
			char* szTag = (char*)&tag;
			cout << "len=" << len << "  ";
			cout << "tag=" << szTag[0] << szTag[1] << szTag[2] << szTag[3] << " size = " << size << endl;
		}

		if ( tag == 'LTAM' )
		{
			_MaterialOnDisk* pMaterial = (_MaterialOnDisk*)( pIff32 );
			_MaterialOnDisk* pMaterialEnd = (_MaterialOnDisk*)( (char*)pMaterial + size );
			for ( ; pMaterial != pMaterialEnd; ++pMaterial )
			{
				const char* szTextureName = pMaterial->textureName;
				if ( bVerbose )
					cout << "[MODL] Processing material \"" << szTextureName << "\"" << endl;
				if ( *szTextureName && !inTextureList( tblTextureNames, string( szTextureName ) ) )
					tblTextureNames.push_back( string( szTextureName ) );
			}
		}

		size = round( size, 4 );
		len -= sizeof( tag ) + sizeof( size ) + size;
		pIff32 += size/4;		// kinda silly, I know
	}
	assert( len == 0 );
}


void
parseCROW( long* pIff32, int32& len, vector<string>& tblTextureNames )
{
	while ( len > 0 )
	{
		int32 tag = *pIff32++;
		int32 size = *pIff32++;

		if ( bVerbose )
		{
			char* szTag = (char*)&tag;
			cout << "len=" << len << "  ";
			cout << "tag=" << szTag[0] << szTag[1] << szTag[2] << szTag[3] << " size = " << size << endl;
		}

		if ( tag == 'LPMB' )
		{
			long* pIff32_BMPL = pIff32;
//			int32 nFrames = *pIff32_BMPL++;
			char* pIff = (char*)pIff32_BMPL;
			char* pIffEnd = pIff + size;	// - sizeof( nFrames );
            char* szTextureName;
			for ( szTextureName = pIff; szTextureName < pIffEnd; szTextureName += strlen( szTextureName ) + 1 )
			{
				if ( bVerbose )
					cout << "[CROW] Processing material \"" << szTextureName << "\"" << endl;
				if ( *szTextureName && !inTextureList( tblTextureNames, string( szTextureName ) ) )
					tblTextureNames.push_back( string( szTextureName ) );
			}
			assert( szTextureName == pIffEnd );
		}

		size = round( size, 4 );
		len -= sizeof( tag ) + sizeof( size ) + size;
		pIff32 += size/4;		// kinda silly, I know
	}
	assert( len == 0 );
}


bool
parseIFF( int fh, vector<string>& tblTextureNames )
{ // Read the IFF file
	assert( fh != 0 );

	int32 len = filelength( fh );
	char* pIff = (char*)malloc( len );
	assert( pIff );
	char* _pInitialIff = pIff;
	int cbRead;
	cbRead = read( fh, pIff, len );
	assert( len == cbRead );

	int32 tag;
	int32 size;

	long* pIff32 = (long*)pIff;

	tag = *pIff32++;
	if( !( ( tag == 'LDOM' ) || ( tag == 'WORC' ) ) )
		return false;

//	assert( ( tag == 'LDOM' ) || ( tag == 'WORC' ) );
	len = *pIff32++;

	switch ( tag )
	{
		case 'LDOM':
			parseMODL( pIff32, len, tblTextureNames );
			break;

		case 'WORC':
			parseCROW( pIff32, len, tblTextureNames );
			break;

		default:
			assert( 0 );
	}

	assert( _pInitialIff );
	free( _pInitialIff );

	return true;
}


// For each object in tblObjectNames, find all texture map references
// contained in the object and add them to tblTextureNames
int
lookup_textures( vector<string>& tblObjectNames, vector<string>& tblTextureNames )
{
	bVerbose && cout << "Objects:" << endl;

	vector<string>::iterator i;
	for ( i = tblObjectNames.begin(); i != tblObjectNames.end(); ++i )
	{
		bVerbose && cout << *i << '\t' << endl;
		*log << "<a href=\"" << *i << "\">" << *i << "</a> ";

		char szExt[ _MAX_EXT ];
		_splitpath( i->c_str(), NULL, NULL, szObjectName, szExt );
		_strlwr( szExt );

		if ( ( strcmp( szExt, ".tga" ) == 0 )
		|| ( strcmp( szExt, ".bmp" ) == 0 )
		|| ( strcmp( szExt, "rgb" ) == 0 )
		|| ( strcmp( szExt, "rgba" ) == 0 )
		|| ( strcmp( szExt, "bw" ) == 0 )
		|| ( strcmp( szExt, "sgi" ) == 0 )
		)
		{
			if ( !inTextureList( tblTextureNames, *i ) )
				tblTextureNames.push_back( *i );
		}
		else
		{
			char* szNewPathName = locateFile( i->c_str(), szVrmlPath );
			if ( !*szNewPathName )
			{
				cerr << error() << "couldn't find mesh " << i->c_str() << " in path " << szVrmlPath << endl;
				exit( 1 );
			}

			int fh = open( szNewPathName, O_RDONLY|O_BINARY );
			if ( fh == -1 )
			{
				cerr << error() << "couldn't open file \"" << szNewPathName << "\"" << endl;
				exit( 1 );
			}

			if ( !parseIFF( fh, tblTextureNames ) )
				cerr << "Don't know how to process file \"" << *i << "\" (not a valid IFF binary file)" << endl;

			close( fh );
		}
	}

	return tblTextureNames.size();
}


void
load_textures( vector<string>& tblTextureNames, int x_page, int y_page )
{
	vector<string>::iterator str;
  	for ( str = tblTextureNames.begin(); str != tblTextureNames.end(); ++str )
	{
		assert( *szObjectName );
		char* szNewPathName = locateFile( str->c_str(), szTexturePath, szObjectName );
		if ( !*szNewPathName )
		{
			cerr << error() << "couldn't find texture " << *str << endl;
			continue;			// Continue and process as many as possible
		}

		ifstream input( szNewPathName, ios::binary );
		assert( input.good() );

		Bitmap* pTexture;

		char szExt[ _MAX_EXT ];
		_splitpath( szNewPathName, NULL, NULL, NULL, szExt );
		if ( stricmp( szExt, ".tga" ) == 0 )
			pTexture = new TargaBitmap( input, szNewPathName );
		else if ( stricmp( szExt, ".bmp" ) == 0 )
			pTexture = new WindowsBitmap( input, szNewPathName );
		else if ( ( stricmp( szExt, ".rgb" ) == 0 )
		|| ( stricmp( szExt, ".bw" ) == 0 )
		|| ( stricmp( szExt, ".sgi" ) == 0 ) )
			pTexture = new SgiBitmap( input, szNewPathName );
		else
		{
			cerr << "Unknown file format for texture " << szNewPathName
				<< " (must be .tga, .bmp, .rgb, .rgba, .bw, or .sgi)" << endl;
			continue;			// Continue and process as many as possible
			exit( -1 );
		}

		assert( pTexture );
		if ( !pTexture->pixels )
		{
			cerr << "Unknown error loading file " << szNewPathName << endl;
			continue;			// Continue and process as many as possible
			exit( -1 );
		}

		tblTextures.push_back( pTexture );

		if ( (pTexture->width() >= x_page * (16/pTexture->bitdepth())) || (pTexture->height() >= y_page) )
		{
			fprintf( stderr, "texture width: %d  height: %d\n", pTexture->width(), pTexture->height() );
			fprintf( stderr, "x_page: %d  y_page: %d\n", x_page, y_page );
			fprintf( stderr, "Texture \"%s\" does not even fit on page!\n", pTexture->name );
		}
//		assert( pTexture->width() <= x_page * (16/pTexture->bitdepth()) );
//		assert( pTexture->height() <= y_page );
	}
}


template <class T>
struct greater_p : public binary_function< T, T, bool >
{
	bool operator()( const T& x_, const T& y_ ) const
	{
		return (*x_) > (*y_);
	}
};


Bitmap*
texture_fit( int x_page, int y_page, const AllocationRestrictions& ar, const char* szRoomName )
{
	Bitmap* out_bitmap = new Bitmap( x_page, y_page, bShowPacking ? 0x7777 : 0 );
	assert( out_bitmap );

	greater_p<Bitmap*> g;
	sort( tblTextures.begin(), tblTextures.end(), g );

	AllocationMap allocated( ar );

	int i;
	vector<Bitmap*>::iterator tex;
  	for ( i=0, tex = tblTextures.begin(); tex != tblTextures.end(); ++tex, ++i )
		out_bitmap->texture_fit( *tex, allocated, szRoomName, "texture", i );

	if ( bCropOutputImage )
		out_bitmap->crop( allocated.xExtent() * x_align / (16/4), allocated.yExtent() * y_align );

	return out_bitmap;
}


template <class T>
struct greater_palette_p : public binary_function< T, T, bool >
{
	bool operator()( const T& x_, const T& y_ ) const
	{
		return ((*x_).coloursUsed()) > ((*y_).coloursUsed());
	}
};


class Point
{
public:
	Point()		{}
	Point( int x, int y )		{ _x = x; _y = y; }
	~Point()	{}
	int _x, _y;
	bool operator==( const Point& pt2 )
	{
		return (_x == pt2._x) && (_y == pt2._y);
	}
	bool operator!= ( const Point& pt2 )	{ return !( *this == pt2 ); }
    friend ostream& operator<<( ostream& o, const Point& point )
	{
		o << '(' << point._x << ',' << point._y << ')';
		return o;
	}
};


char*
range( int start, int end )
{
	static char szRange[ 20 ];
	sprintf( szRange, "[%d..%d]", start, end );
	return szRange;
}


void
write_texture_log()
{
	vector<Bitmap*> tblPalettes = tblTextures;

	greater_palette_p<Bitmap*> g;
	sort( tblPalettes.begin(), tblPalettes.end(), g );

	vector<Point> tblPaletteEntry;
	{
	vector<Bitmap*>::iterator tex;
	Point invalidPoint( -1, -1 );
  	for ( tex = tblTextures.begin(); tex != tblTextures.end(); ++tex )
	{
		Point point( (*tex)->xPal, (*tex)->yPal );
		if ( point != invalidPoint )
		{
			vector<Point>::iterator f = find( tblPaletteEntry.begin(),
				tblPaletteEntry.end(), point );
			if ( f == tblPaletteEntry.end() )
				tblPaletteEntry.push_back( point );
		}
	}
	}

	int nPalettes = tblPalettes.size();

	if ( tblTextures.size() )
	{
		*log << "<script language=\"JavaScript\">" << endl;
		*log << "function sort()" << endl;
		*log << "{" << endl;
		*log << "\tvar curElement = event.srcElement;" << endl;
		*log << "\tif ( curElement.id != \"\" )" << endl;
		*log << "\t{" << endl;
		*log << "\t\tif ( texturetbl.lastElement )" << endl;
		*log << "\t\t\ttexturetbl.lastElement.style.color = \"blue\";" << endl;
		*log << "\t\tstocklist.Sort = curElement.id;" << endl;
		*log << "\t\tcurElement.style.color = \"red\";" << endl;
		*log << "\t\tstocklist.Reset();" << endl;
		*log << "\t\ttexturetbl.lastElement = curElement;" << endl;
		*log << "\t}" << endl;
		*log << "}" << endl;
		*log << "</script>" << endl;
		*log << endl;

		*log << "<object id=\"stocklist\" width=\"0\" height=\"0\" classid=\"clsid:333C7BC4-460F-11D0-BC04-0080C7055A83\">" << endl;
		*log << "\t<param name=\"DataURL\" value=\"textile.dat\">" << endl;
		*log << "\t<param name=\"FieldDelim\" value=\",\">" << endl;
		*log << "\t<param name=\"TextQualifier\" value=\"\">" << endl;
		*log << "\t<param name=\"UseHeader\" value=\"True\">" << endl;
		*log << "</object>" << endl;
		*log << endl;

		*log << "<div align=\"left\">" << endl;
		*log << "<table id=\"texturetbl\" datasrc=\"#stocklist\" border=1>" << endl;
		*log << "<thead>" << endl;
		*log << "\t<tr onclick=sort()>" << endl;
		*log << "\t\t<td class=thd><div id=#>#</div></td>" << endl;
		*log << "\t\t<td class=thd><div id=Bitmap>Bitmap</div></td>" << endl;
		*log << "\t\t<td class=thd><div id=Dimensions>Dimensions</div></td>" << endl;
		*log << "\t\t<td class=thd><div id=Colours Used>Colours Used</div></td>" << endl;
		*log << "\t\t<td class=thd><div id=Palette #>Palette #</div></td>" << endl;
		*log << "\t\t<td class=thd><div id=Translucent>Translucent</div></td>" << endl;
		*log << "\t</tr>" << endl;
		*log << "</thead>" << endl;

		*log << "<tbody>" << endl;
		*log << "<tr>" << endl;
		*log << "\t<td><a datasrc=\"Website\"><div datafld=\"#\"></div></a></td>" << endl;
		*log << "\t<td><div datafld=\"Bitmap\" DATAFORMATAS=HTML></div></td>" << endl;
		*log << "\t<td><div datafld=\"Dimensions\"></div></td>" << endl;
		*log << "\t<td><div datafld=\"Colours Used\"></div></td>" << endl;
		*log << "\t<td><div datafld=\"Palette #\"></div></td>" << endl;
		*log << "\t<td><div datafld=\"Colour Range\"></div></td>" << endl;
		*log << "\t<td><div datafld=\"Translucent\"></div></td>" << endl;
		*log << "</tr>" << endl;

		ofstream data( "textile.dat", ios::out );
		assert( data.good() );
		data << "#,Bitmap,Dimensions,Colours Used,Palette #,Colour Range,Translucent" << endl;

		vector<Bitmap*>::iterator tex;
		vector<Bitmap*>::iterator pal;
		int i;
  		for ( i = 0, tex = tblTextures.begin(), pal = tblPalettes.begin();
				tex != tblTextures.end(); ++i, ++tex, ++pal )
		{
			Bitmap* pTex = *tex;
			Bitmap* pPal = *pal;

			char szTextureName[ PATH_MAX ];
			char szTextureExtensionName[ _MAX_EXT ];
			char szTexture[ PATH_MAX ];

			// #
			data << i+1 << ",";

			// Bitmap filename
			_splitpath( pTex->name, NULL, NULL, szTextureName, szTextureExtensionName );
			_makepath( szTexture, NULL, NULL, szTextureName, szTextureExtensionName );
			data << "<a href=\"" << szTexture << "\">" << szTexture << "</a>,";

			// Dimensions
			char szDimensions[ 16 ];
			sprintf( szDimensions, "%dx%dx%d", pTex->width(), pTex->height(), 1 << pTex->bitdepth() );
			data << szDimensions << ",";

			// Colours used
			if ( pPal->bitdepth() <= 8 )
				data << pPal->coloursUsed();
			data << ",";

			// Palette #
			Point* palEntry;
			palEntry = find( tblPaletteEntry.begin(), tblPaletteEntry.end(), Point( pPal->xPal, pPal->yPal ) );
			if ( palEntry != tblPaletteEntry.end() )
				data << palEntry - tblPaletteEntry.begin();
			data << ",";

			// Colour Range
			if ( pPal->bitdepth() <= 8 )
				data << range( pPal->_startColour, pPal->_endColour );
			data << ",";

			// Translucent
			data << ",";

			data << endl;
		}
		assert( pal == tblPalettes.end() );

		*log << "</tbody>" << endl;
		*log << "</table>" << endl;
		*log << "</div>" << endl;
	}
}


Bitmap*
palette_fit( int x_page, int y_page, const AllocationRestrictions& ar, const char* szRoomName )
{
	Bitmap* out_bitmap = new Bitmap( x_page, y_page, 0 );
	assert( out_bitmap );

	vector<Bitmap*> tblPalettes = tblTextures;

	greater_palette_p<Bitmap*> g;
	sort( tblPalettes.begin(), tblPalettes.end(), g );

	AllocationMap allocated( ar );

	vector<Bitmap*>::iterator pal;
  	for ( pal = tblPalettes.begin(); pal != tblPalettes.end(); ++pal )
	{
		Bitmap* pPal = *pal;
		assert( pPal->xPal == -1 );
		assert( pPal->yPal == -1 );
		assert( pPal->_idxPal == -1 );
		if ( pPal->bitdepth() <= 8 )
		{
			Bitmap* bmPalette = new Bitmap( **pal );
			assert( bmPalette );
			// transfered control of colour cycles to palette. remove
			out_bitmap->texture_fit( bmPalette, allocated, szRoomName, "palette for" );
            pPal->xPal = bmPalette->_subx * 16;
			pPal->yPal = bmPalette->_suby;
			delete bmPalette;
		}
	}

	return out_bitmap;
}


void
write_rmuv( const char* szRoomName )
{
	// RMUV [size ]
	//  long nTextures;
	//  char
	char szRMUV[ _MAX_FNAME ];
	sprintf( szRMUV, "%s/%s.ruv", szOutDir, szRoomName );
	ofstream rmuv( szRMUV, ios::binary | ios::out );
	assert( rmuv.good() );

	rmuv << 'r' << 'm' << 'u' << 'v';
	assert( ( sizeof( _RMUV ) % 4 ) == 0 );
	unsigned long cbRMUV = sizeof( unsigned long ) + sizeof( _RMUV ) * tblTextures.size();
	rmuv.write( (char const*)&cbRMUV, sizeof( unsigned long ) );

	int nTextures = tblTextures.size();
	rmuv.write( (char const*)&nTextures, sizeof( unsigned long ) );

	vector<Bitmap*>::iterator tex;
  	for ( tex = tblTextures.begin(); tex != tblTextures.end(); ++tex )
	{
		Bitmap* pTex = *tex;
		assert( pTex );

		_RMUV chunk;

		memset( chunk.szTextureName, 0, sizeof( chunk.szTextureName ) );
		char szFilename[ _MAX_FNAME ];
		char szExt[ _MAX_EXT ];
		_splitpath( pTex->name, NULL, NULL, szFilename, szExt );

		strcpy( chunk.szTextureName, szFilename );
		strcat( chunk.szTextureName, szExt );
		assert( strlen( chunk.szTextureName ) <= _RMUV::TEXTURE_NAME_LENGTH );

		chunk.u = pTex->_subx;
		chunk.v = pTex->_suby;
		chunk.w = pTex->width();
		chunk.h = pTex->height();

		assert( pTex->bitdepth() == 4 || pTex->bitdepth() == 8
			|| pTex->bitdepth() == 15 || pTex->bitdepth() == 16 );
		switch ( pTex->bitdepth() )
		{
			case 4:
			case 8:
				chunk.bitdepth = pTex->bitdepth();
				break;
			case 15:
			case 16:
				chunk.bitdepth = 15;
				assert( pTex->xPal == -1 );
				assert( pTex->yPal == -1 );
				break;
		}

		chunk.nFrame = 0;
		chunk.bTranslucent = pTex->hasTransparentColour();

		chunk.palx = pTex->xPal / pal_x_align;
		chunk.paly = pTex->yPal / pal_y_align;

		rmuv.write( (char const*)&chunk, sizeof( _RMUV ) );
	}
}


void
write_ccyc( const char* szRoomName, Bitmap* pBitmap )
{
	assert( pBitmap );

	// CCYC [size ]
	//  long nColourCycles;
	//  char
	char szCCYC[ _MAX_FNAME ];
	sprintf( szCCYC, "%s/%s.cyc", szOutDir, szRoomName );
	ofstream ccyc( szCCYC, ios::binary | ios::out );
	assert( ccyc.good() );

	ccyc << 'c' << 'c' << 'y' << 'c';
	assert( ( sizeof( CCYC ) % 4 ) == 0 );
	int nColourCycles = pBitmap->_nColourCycles;
	unsigned long cbCCYC = sizeof( unsigned long ) + sizeof( CCYC ) * nColourCycles;
	ccyc.write( (char const*)&cbCCYC, sizeof( unsigned long ) );

	ccyc.write( (char const*)&nColourCycles, sizeof( unsigned long ) );
	for ( int i=0; i<nColourCycles; ++i )
	{
		CCYC chunk;

		chunk.start = pBitmap->_colourCycle[ i ]._start;
		chunk.end = pBitmap->_colourCycle[ i ]._end;
		chunk.speed = int32( pBitmap->_colourCycle[ i ]._speed * 65536.0 );

		ccyc.write( (char const*)&chunk, sizeof( CCYC ) );
	}
}


////////////////////////////////////////////////////////////////////////////////
// from level .ini file, for given room name, construct list of objects
// in the room, and then from that list, construct the list of textures
// (by calling lookup_textures).  Then, fit the textures (based on area),
// fit the palettes, and output the texture map and (u,v) description file.
void
processRoom( const char* szRoomName, int x_page, int y_page )
{
	assert( szRoomName );
	assert( *szRoomName );

	vector<string> tblObjectNames;
	vector<string> tblTextureNames;

	char delimns[] = ",";
	static char szObjects[ 2048 ];

	GetPrivateProfileString( "Rooms", (char*)szRoomName, "", szObjects, sizeof( szObjects ), szIniFile );

	for ( char* p = strtok( szObjects, delimns ); p; p = strtok( NULL, delimns ) )
		tblObjectNames.push_back( p );

	*log << "<p>Room &quot;" << szRoomName << "&quot; contains " << tblObjectNames.size() <<
		" object" << ( tblObjectNames.size()==1 ? "" : "s" ) << " [ ";
	lookup_textures( tblObjectNames, tblTextureNames );
	*log << "]</p>" << endl;

	load_textures( tblTextureNames, x_page, y_page );

	AllocationRestrictions ar;
	// Texture AllocationRestrictions
	ar.x_align = x_align;
	ar.y_align = y_align;
	ar.width = x_page / ar.x_align * (16/4);		// 4 bits-per-pixel
	ar.height = y_page / ar.y_align;
	ar.bAlignToSizeMultiple = bAlignToSizeMultiple;
	ar.k = (16/4);
	Bitmap* out_bitmap = texture_fit( x_page, y_page, ar, szRoomName );
	assert( out_bitmap );

	// Palette AllocationRestrictions
	ar.x_align = 16;
	ar.x_align = 64;
	ar.y_align = 1;
	ar.width = pal_x_page / ar.x_align * (16/4);	// palettes are 16 bits-per-pixel
	ar.height = pal_y_page / ar.y_align;
	ar.bAlignToSizeMultiple = false;
	ar.k = (16/4);
	Bitmap* out_palette = palette_fit( pal_x_page, pal_y_page, ar, szRoomName );
	assert( out_palette );

	write_texture_log();

	{ // Save composite Targa file
	char szTgaFile[PATH_MAX ];
	sprintf( szTgaFile, "%s.tga", szRoomName );
	out_bitmap->removeColourCycles();				// this is stored in the palette file
	out_bitmap->save( szTgaFile );
	}

	{ // Save room palette file
	char szPalFile[ PATH_MAX ] = "pal";
	strcat( szPalFile, strnicmp( szRoomName, "room", strlen( "room" ) ) == 0 ?
		szRoomName + strlen( "room" ) : szRoomName );
	strcat( szPalFile, ".tga" );
	out_palette->save( szPalFile );
	}

	write_rmuv( szRoomName );
	write_ccyc( szRoomName, out_palette );

	// Cleanup
	delete out_bitmap;
	delete out_palette;

	vector<Bitmap*>::iterator tex = tblTextures.begin();
	while ( tblTextures.begin() != tblTextures.end() )
	{
		delete *tex;
		tblTextures.erase( tblTextures.begin() );
	}
//  tblTextures.erase( tblTextures.begin(), tblTextures.end() );
	assert( tblTextures.size() == 0 );

	*log << endl;
}
