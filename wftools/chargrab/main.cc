//==============================================================================
// main.cc for chargrab: Copyright (c) 1995-1999, World Foundry Group  
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
// v0.3.0	?? May 96	Added PF3D support
// v0.4.0	26 Jun 96	Converted to STL, added SourceControl
// v0.5.0	07 Jul 96	Added alpha channel support
// v0.5.91	12 Jul 96	Added "per room" palettes (just like textures)
//------------------------------------------------------------------------------
// v1.0.0	16 Sep 97	24-bit Targa files / 24-bit Windows .bmp files
//						RMUV interface (asset id, material # -> x,y,w,h)
//						Removed PF3D, VRML supported, added IFF
//==============================================================================

#include "global.hp"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <stdio.h>
#include <limits.h>


#include "command.hp"
#include "rmuv.hp"
#include "allocmap.hp"
#include "profile.hp"
#include "textile.hp"

#include "textile.hp"
#include "types.h"
#include "bitmap.hp"
#include "allocmap.hp"
#include "rmuv.hp"
#include "ccyc.hp"

#include <version/version.hp>

//=============================================================================

int OUTPUT_XSIZE = 128;
int OUTPUT_YSIZE = 512;
const int TILE_SIZE = 16;

//=============================================================================

#if 0
class HTML_ofstream : public ofstream
{
public:
	HTML_ofstream( const char* );
	virtual ~HTML_ofstream();
};
#endif

ofstream* log;

int x_align = 16;
int y_align = 1;
int x_page = 512;
int y_page = 2048;
char szOutFile[ PATH_MAX ] = "room%d.bin";
char* szIniFile;
int pal_x_page = 320;
int pal_y_page = 8;
int pal_x_align = 16;
int pal_y_align = 1;
char szColourCycle[ PATH_MAX ];
char szOutDir[ PATH_MAX ] = ".";

char szTexturePath[ 2048 ] = "";
char szVrmlPath[ 2048 ] = "";

bool bDebug = false;
bool bVerbose = false;
bool bFrame = false;
bool bShowAlign = false;
bool bShowPacking = false;
bool bCropOutputImage = true;
bool bPowerOf2Size = false;
bool bAlignToSizeMultiple = false;
bool bMipMapping = false;
bool bSourceControl = false;
TargetSystem ts = TARGET_WINDOWS;
// Default transparent color is (0,0,0)
int rTransparent = 0;
int gTransparent = 0;
int bTransparent = 0;
uint16 colTransparent;
bool bForceTranslucent;
bool bFlipYOut = false;


// Command line switches
const char pagex[] = "-pagex=";
const char pagey[] = "-pagey=";
const char alignx[] = "-alignx=";
const char aligny[] = "-aligny=";
const char output[] = "-o=";
const char verbose[] = "-verbose";
const char frame[] = "-frame";
const char showalign[] = "-showalign";
const char showpacking[] = "-showpacking";
const char inifile[] = "-ini=";
const char debugflag[] = "-debug";
const char nocrop[] = "-nocrop";
const char powerof2size[] = "-powerof2size";
const char aligntosizemultiple[] = "-aligntosizemultiple";
const char systemplaystation[] = "-Tpsx";
//const char systemsaturn[] = "-Tsat";
const char systemwindows[] = "-Twin";
//const char systemdos[] = "-Tdos";
const char mipmapping[] = "-mipmap";
const char sourcecontrol[] = "-sourcecontrol";
const char transparent[] = "-transparent=";
const char colourcycle[] = "-colourcycle=";
const char outdir[] = "-outdir=";
const char translucent[] = "-translucent";
const char flipyout[] = "-flipyout";

void processRoom( const char* szRoomName, int x_page, int y_page );

//=============================================================================

Bitmap*
LoadTexture(string filename)
{
	ifstream input( filename.c_str(), ios::binary );
	assert( input.good() );

	Bitmap* pTexture;

	char szExt[ _MAX_EXT ];
	_splitpath( filename.c_str(), NULL, NULL, NULL, szExt );
	if ( stricmp( szExt, ".tga" ) == 0 )
		pTexture = new TargaBitmap( input, filename.c_str() );
	else if ( stricmp( szExt, ".bmp" ) == 0 )
		pTexture = new WindowsBitmap( input, filename.c_str() );
	else if ( ( stricmp( szExt, ".rgb" ) == 0 )
	|| ( stricmp( szExt, ".bw" ) == 0 )
	|| ( stricmp( szExt, ".sgi" ) == 0 ) )
		pTexture = new SgiBitmap( input, filename.c_str() );
	else
	{
		cerr << "Unknown file format for texture " << filename
			<< " (must be .tga, .bmp, .rgb, .rgba, .bw, or .sgi)" << endl;
		exit( -1 );
	}

	assert( pTexture );
	if ( !pTexture->pixels )
	{
		cerr << "Unknown error loading file " << filename << endl;
		exit( -1 );
	}
	return pTexture;
}

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(const CommandLine& cl)
{
    unsigned int commandIndex;
	for (commandIndex=1; commandIndex < cl.Size() && (cl[commandIndex][0] == '-'); ++commandIndex )
	{
		string command = cl[commandIndex];
		assert(command[0] == '-');
		//DBSTREAM2( cdebug << "command line option <" << command << "> Found" << endl; )
		string restOfCommand = string(command.substr(2,command.length()-2));
		switch(command[1])
		{
			case 'p':
			case 'P':
//				RedirectStandardStream(restOfCommand.c_str());
				break;
			case 'x':
			case 'X':
				OUTPUT_XSIZE = atoi(restOfCommand.c_str());
				AssertMsg((OUTPUT_XSIZE % TILE_SIZE) == 0, "New X Size must be evenly divisble by " << TILE_SIZE);
				break;

			case 'y':
			case 'Y':
				OUTPUT_YSIZE = atoi(restOfCommand.c_str());
				AssertMsg((OUTPUT_YSIZE % TILE_SIZE) == 0, "New Y Size must be evenly divisble by " << TILE_SIZE);
				break;

			case 'd':
			case 'D':
			{

			default:
				cerr << "CharGrab Error: Unrecognized command line switch \"" <<
					command << "\"" << endl;
				break;
			}
		 }
	 }

	if(cl.Size()-(commandIndex-1) < 2)
	{
		cout << "CharGrab V0.03 Copyright 1998,99 World Foundry Group." << endl;
		cout << "Usage: chargrab {-<switches>} <infile> <output char file> <output mapfile>" << endl;
		cout << "Switches:" << endl;
		cout << "    -x<new output char file x size>" << endl;
		cout << "    -y<new output char file y size>" << endl;
		cout << "    -p<stream initial><stream output>, where:" << endl;
		cout << "        <stream initial> can be any of:" << endl;
		cout << "            w=warnings (defaults to standard err)" << endl;
		cout << "            e=errors (defaults to standard err)" << endl;
		cout << "            f=fatal (defaults to standard err)" << endl;
		cout << "            s=statistics (defaults to null)" << endl;
		cout << "            p=progress (defaults to null)" << endl;
		cout << "            d=debugging  (defaults to null)" << endl;
		cout << "        <stream output> can be any of:" << endl;
		cout << "            n=null, no output is produced" << endl;
		cout << "            s=standard out" << endl;
		cout << "            e=standard err" << endl;
		cout << "            f<filename>=output to filename" << endl;
		exit(1);
	}
	return(commandIndex);
}

//=============================================================================

class TileMap
{
public:
	TileMap(int xSize, int ySize)
	{
		assert(xSize > 0);
		assert(ySize > 0);
		_xSize = xSize;
		_ySize = ySize;
		_buffer = new(char[_xSize*_ySize]);
		assert(_buffer);
	}

	~TileMap()
	{
		delete [] _buffer;
	}

	void Set(int x, int y, char val)
	{
		RangeCheck(x,0,_xSize);
		RangeCheck(y,0,_ySize);
		_buffer[(y*_xSize)+x] = val;
	}

	void Save(ostream& output)
	{
		output.write((char*)&_xSize,8);
		output.write(_buffer,_xSize*_ySize);
	}

private:
	int _xSize;
	int _ySize;
	char* _buffer;
};

//=============================================================================

int
main( int argc, char* argv[] )
{
	CommandLine cl(argc,argv);
	int commandIndex = ParseCommandLine(cl);

	cout << "Reading from source file " << cl[commandIndex] << endl;
	Bitmap* input = LoadTexture(cl[commandIndex]);

	AssertMsg((input->width() % TILE_SIZE) == 0,"Input width must be evenly divisible by " << TILE_SIZE);
	AssertMsg((input->height() % TILE_SIZE) == 0,"Input width must be evenly divisible by " << TILE_SIZE);

	ofstream outMap(cl[2+commandIndex].c_str(),ios::out|ios::binary);
	if(outMap.rdstate() != ios::goodbit)
	{
		cerr << "Unable to open output file " << cl[2+commandIndex] << endl;
		return 5;
	}

	int MAX_TILES = (OUTPUT_XSIZE/TILE_SIZE)*(OUTPUT_YSIZE/TILE_SIZE);
	const int TRANSPARENT_TILE = 255;
	assert((OUTPUT_XSIZE/TILE_SIZE)*(OUTPUT_YSIZE/TILE_SIZE) >= (MAX_TILES));
#define TILE_X(tile) ((tile%(OUTPUT_XSIZE/TILE_SIZE))*TILE_SIZE)
#define TILE_Y(tile) ((tile/(OUTPUT_XSIZE/TILE_SIZE))*TILE_SIZE)

	Bitmap* out_bitmap = new Bitmap( OUTPUT_XSIZE, OUTPUT_YSIZE, 0 );

	cout << "converting:" << endl;

	TileMap map(input->width()/TILE_SIZE,input->height()/TILE_SIZE);

	int maxTiles=0;

	Bitmap transparent_bitmap(TILE_SIZE,TILE_SIZE);

	for(int yOffset=0;yOffset<input->height();yOffset+=TILE_SIZE)
		for(int xOffset=0;xOffset<input->width();xOffset+=TILE_SIZE)
		{
//			cout << "checking :" << xOffset << "," << yOffset << endl;
			Bitmap tile(TILE_SIZE,TILE_SIZE);
			tile.copy(input,xOffset,yOffset,TILE_SIZE,TILE_SIZE,0,0);

			int tileIndex=0;
			if(transparent_bitmap.sameBitmap(0,0,&tile))
				tileIndex = TRANSPARENT_TILE;
			else
			{
				while(tileIndex < maxTiles && !out_bitmap->sameBitmap(TILE_X(tileIndex),TILE_Y(tileIndex),&tile))
					tileIndex++;
				if(tileIndex == maxTiles)
				{
//					cout << "	creating new tile at index " << tileIndex << endl;
					if(maxTiles >= MAX_TILES)
					{
						cerr << "Ran out of tiles, writting out partial file" << endl;
						assert( out_bitmap );
						out_bitmap->save( cl[1+commandIndex].c_str() );
						exit(1);

					}
					maxTiles++;
//					cout << "   Writting new tile at " << TILE_X(tileIndex) << "," << TILE_Y(tileIndex) << endl;
					out_bitmap->copy(&tile,0,0,TILE_SIZE,TILE_SIZE,TILE_X(tileIndex),TILE_Y(tileIndex));
					assert(out_bitmap->sameBitmap(TILE_X(tileIndex),TILE_Y(tileIndex),&tile));
				}
				else
				{
					assert(out_bitmap->sameBitmap(TILE_X(tileIndex),TILE_Y(tileIndex),&tile));
//					cout << "	repeat found! at index " << tileIndex << endl;
				}
			}
			map.Set(xOffset/TILE_SIZE,yOffset/TILE_SIZE,tileIndex);
		}

	cout << "Tiles Used: " << maxTiles << endl;
	cout << "Writing to dest file " << cl[1+commandIndex] << endl;
	assert( out_bitmap );

//	out_bitmap->copy( input, 0,0 );
	out_bitmap->save( cl[1+commandIndex].c_str() );

	map.Save(outMap);
//	Bitmap* out_bitmap = texture_fit( x_page, y_page, ar, szRoomName );

	return 0;
}

//=============================================================================
