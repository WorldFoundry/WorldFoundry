//==============================================================================
// main.cc for textile
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
#if defined(__WIN__)
#include <ostream>
#endif
#include <fstream>
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string>
#include <stdio.h>

using namespace std;

void usage( int argc, char* argv[] );
#include "rmuv.hp"
#include "allocmap.hp"
#include "profile.hp"
#include "textile.hp"
extern bool bGlobalError;

//#include "memcheck.h"
//MC_SET_ERF( erf_find_leaks );

#if 0
class HTML_ofstream : public ofstream
{
public:
	HTML_ofstream( const char* );
	virtual ~HTML_ofstream();
};
#endif

ofstream* textilelog;

int x_align = 16;
int y_align = 1;
int x_page = 512;
int y_page = 2048;
int perm_x_page = 128;
int perm_y_page = 512;

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

extern char szVersion[];


// Command line switches
const char pagex[] = "-pagex=";
const char pagey[] = "-pagey=";
const char permpagex[] = "-permpagex=";
const char permpagey[] = "-permpagey=";
const char palx[] = "-palx=";
const char paly[] = "-paly=";

const char alignx[] = "-alignx=";
const char aligny[] = "-aligny=";
const char clOutput[] = "-o=";
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
const char systemlinux[] = "-Tlinux";
//const char systemdos[] = "-Tdos";
const char mipmapping[] = "-mipmap";
const char sourcecontrol[] = "-sourcecontrol";
const char transparent[] = "-transparent=";
const char colourcycle[] = "-colourcycle=";
const char outdir[] = "-outdir=";
const char translucent[] = "-translucent";
const char flipyout[] = "-flipyout";

void processRoom( const char* szRoomName, int x_page, int y_page );


void
usage( int argc, char* argv[] )
{
	cout << "textile v" << szVersion << " Copyright 1995,96,97,98,99,2000,2001 World Foundry Group." << endl;
	cout << "by William B. Norris IV" << endl;
	cout << "Constructs single Targa file composite of all input texture maps" << endl;
	cout << endl;
	cout << "Usage: textile [switches] (texture.(tga|bmp))*  []=default value" << endl;
	cout << '\t' << outdir << "\tOutput directory [.]" << endl;
	cout << '\t' << pagex << "\t\tPage width [" << x_page << "]" << endl;
	cout << '\t' << pagey << "\t\tPage height [" << y_page << "]" << endl;
	cout << '\t' << permpagex << "\tPermenent Page width [" << perm_x_page << "]" << endl;
	cout << '\t' << permpagey << "\tPermenent Page height [" << perm_y_page << "]" << endl;
	cout << '\t' << palx << "\t\tPalette width [" << pal_x_page << "]" << endl;
	cout << '\t' << paly << "\t\tPalette height [" << pal_y_page << "]" << endl;
	cout << '\t' << alignx << "(#|w)\tX-alignment required [" << x_align << "] (w=texture width)" << endl;
	cout << '\t' << aligny << "(#|h)\tY-alignment required [" << y_align << "] (h=texture height)" << endl;
	cout << '\t' << transparent << "\tr,g,b Interpret specified color as transparent [" << rTransparent << ',' << gTransparent << ',' << bTransparent << ']' << endl;
	cout << '\t' << colourcycle << "\tSpecify colour cycle specification .ini file" << endl;
	cout << '\t' << powerof2size << "\tEnsure texture width and height are powers of two" << endl;
	cout << '\t' << mipmapping << "\t\tGenerate mip-mapping bitmaps" << endl;
//	cout << '\t' << aligntosizemultiple << "\t" << endl;
	cout << '\t' << systemplaystation << "\t\tTarget system Playstation" << endl;
//	cout << '\t' << systemsaturn << "\t\tTarget system Saturn" << endl;
//	cout << '\t' << systemdos << "\t\tTarget system DOS4G" << endl;
	cout << '\t' << systemwindows << "\t\tTarget system Windows 95/Windows NT" << endl;
	cout << '\t' << systemlinux << "\t\tTarget system Linux" << endl;
	cout << '\t' << clOutput << "\t\tOutput data file [" << szOutFile << "]" << endl;
	cout << '\t' << verbose << "\tEnable verbose messages" << endl;
	cout << '\t' << frame << "\t\tShow frames around textures" << endl;
	cout << '\t' << showalign << "\tShow waste from alignment" << endl;
	cout << '\t' << showpacking << "\tShow waste from texture packing" << endl;
	cout << '\t' << nocrop << "\t\tDon't crop output bitmap" << endl;
	cout << '\t' << flipyout << "\tFlip output image vertically" << endl;
	cout << '\t' << translucent << "\tForce all textures translucent" << endl;
	cout << '\t' << sourcecontrol << "\tVerify input files are in source code control system" << endl;
	cout << '\t' << inifile << '*' << "\t\tRead room descriptions from file" << endl;
	cout << "\t\t\t(last .ini file needs to contain room descriptions)" << endl;
//	cout << '\t' << texture << "\tProcess list as textures (instead of objects)" << endl;
//	cout << '\t' <<
}


int
main( int argc, char* argv[] )
{
	assert( ( sizeof( _RMUV ) % 4 ) == 0 );

	int nRooms = 1;

	if ( argc == 1 )
	{
		usage( argc, argv );
		return -1;
	}

	//mc_startcheck( NULL );

	textilelog = new ofstream( "textile.log.htm" );
	assert( textilelog );
	{ // Construct the HTML log
	*textilelog << "<html>" << endl;
	*textilelog << endl;
	*textilelog << "<head>" << endl;
	*textilelog << "<title>Textile Log</title>" << endl;
	*textilelog << "<meta name=\"GENERATOR\" content=\"World Foundry Textile v" << szVersion << "\">" << endl;

	*textilelog << "<style>" << endl;
	*textilelog << "\t.thd { color: \"#0000FF\"; background: \"#808000\"; font-weight: \"bold\"; cursor: \"s-resize\" }" << endl;
	*textilelog << "</style>" << endl;

	*textilelog << "</head>" << endl;
	*textilelog << endl;
	*textilelog << "<body>" << endl;
	}

	char szConfig[] = "Configuration";

	for ( ++argv; *argv; ++argv )
	{
		if ( strcmp( *argv, "-?" ) == 0 )
		{
			usage( argc, argv );
			return -1;
		}
		else if ( strcmp( *argv, systemplaystation ) == 0 )
			ts = TARGET_PLAYSTATION;
//		else if ( strcmp( *argv, systemsaturn ) == 0 )
//			ts = TARGET_SATURN;
//		else if ( strcmp( *argv, systemdos ) == 0 )
//			ts = TARGET_DOS;
		else if ( strcmp( *argv, systemwindows ) == 0 )
			ts = TARGET_WINDOWS;
		else if ( strcmp( *argv, systemlinux ) == 0 )
			ts = TARGET_LINUX;
		else if ( strcmp( *argv, debugflag ) == 0 )
			bDebug = true;
		else if ( strcmp( *argv, verbose ) == 0 )
			bVerbose = true;
		else if ( strcmp( *argv, frame ) == 0 )
			bFrame = true;
		else if ( strcmp( *argv, showalign ) == 0 )
			bShowAlign = true;
		else if ( strcmp( *argv, showpacking ) == 0 )
			bShowPacking = true;
		else if ( strcmp( *argv, nocrop ) == 0 )
			bCropOutputImage = false;
		else if ( strcmp( *argv, translucent ) == 0 )
			bForceTranslucent = true;
		else if ( strcmp( *argv, flipyout ) == 0 )
			bFlipYOut = true;
		else if ( strncmp( *argv, pagex, strlen( pagex ) ) == 0 )
			x_page = atoi( *argv + strlen( pagex ) );
		else if ( strncmp( *argv, pagey, strlen( pagey ) ) == 0 )
			y_page = atoi( *argv + strlen( pagey ) );
		else if ( strncmp( *argv, permpagex, strlen( permpagex ) ) == 0 )
			perm_x_page = atoi( *argv + strlen( permpagex ) );
		else if ( strncmp( *argv, permpagey, strlen( permpagey ) ) == 0 )
			perm_y_page = atoi( *argv + strlen( permpagey ) );

		else if ( strncmp( *argv, palx, strlen( palx ) ) == 0 )
			pal_x_page = atoi( *argv + strlen( palx ) );
		else if ( strncmp( *argv, paly, strlen( paly ) ) == 0 )
			pal_y_page = atoi( *argv + strlen( paly ) );

		else if ( strncmp( *argv, alignx, strlen( alignx ) ) == 0 )
		{
			if ( *( *argv + strlen( alignx ) ) == 'w' )
				x_align = y_align = 16, bAlignToSizeMultiple = true;
			else
				x_align = atoi( *argv + strlen( alignx ) );
		}
		else if ( strncmp( *argv, aligny, strlen( aligny ) ) == 0 )
		{
			if ( *( *argv + strlen( aligny ) ) == 'h' )
				y_align = 16, bAlignToSizeMultiple = true;
			else
				x_align = y_align = atoi( *argv + strlen( aligny ) );
		}
		else if ( strncmp( *argv, transparent, strlen( transparent ) ) == 0 )
		{
			sscanf( *argv + strlen( transparent ), "%d,%d,%d",
				&rTransparent, &gTransparent, &bTransparent );
		}
		else if ( strncmp( *argv, colourcycle, strlen( colourcycle ) ) == 0 )
			strcpy( szColourCycle, *argv + strlen( colourcycle ) );
		else if ( strncmp( *argv, outdir, strlen( outdir ) ) == 0 )
			strcpy( szOutDir, *argv + strlen( outdir ) );
		else if ( strcmp( *argv, powerof2size ) == 0 )
			bPowerOf2Size = true;
		else if ( strcmp( *argv, sourcecontrol ) == 0 )
			bSourceControl = true;
		else if ( strncmp( *argv, clOutput, strlen( clOutput ) ) == 0 )
			strcpy( szOutFile, *argv + strlen( clOutput ) );
		else if ( strncmp( *argv, inifile, strlen( inifile ) ) == 0 )
		{
			szIniFile = *argv + strlen( inifile );

			x_page = GetPrivateProfileInt( szConfig, "pagex", x_page, szIniFile );
			y_page = GetPrivateProfileInt( szConfig, "pagey", y_page, szIniFile );
			perm_x_page = GetPrivateProfileInt( szConfig, "permpagex", perm_x_page, szIniFile );
			perm_y_page = GetPrivateProfileInt( szConfig, "permpagey", perm_y_page, szIniFile );
			x_align = GetPrivateProfileInt( szConfig, "alignx", x_align, szIniFile );
			y_align = GetPrivateProfileInt( szConfig, "aligny", y_align, szIniFile );
			bDebug = bool( GetPrivateProfileInt( szConfig, "debug", bDebug, szIniFile ) );
			bVerbose = bool( GetPrivateProfileInt( szConfig, "verbose", bVerbose, szIniFile ) );
			bFrame = bool( GetPrivateProfileInt( szConfig, "showframe", bFrame, szIniFile ) );
			bShowAlign = bool( GetPrivateProfileInt( szConfig, "showalign", bShowAlign, szIniFile ) );
			bShowPacking = bool( GetPrivateProfileInt( szConfig, "showpacking", bShowPacking, szIniFile ) );
			bCropOutputImage = !bool( GetPrivateProfileInt( szConfig, "crop", !bCropOutputImage, szIniFile ) );
			GetPrivateProfileString( szConfig, "out", szOutFile, szOutFile, sizeof( szOutFile ), szIniFile );
			nRooms = GetPrivateProfileInt( "Rooms", "nRooms", nRooms, szIniFile );
			bPowerOf2Size = bool( GetPrivateProfileInt( szConfig, "powerof2size", bPowerOf2Size, szIniFile ) );
			bSourceControl = bool( GetPrivateProfileInt( szConfig, "sourcecontrol", bSourceControl, szIniFile ) );

			// Target system
			ts = TargetSystem( GetPrivateProfileInt( szConfig, "TargetSystem", int( ts ), szIniFile ) );

			// Palette configuration
			char szPalette[] = "Palette";
			pal_x_page = GetPrivateProfileInt( szPalette, "pagex", pal_x_page, szIniFile );
			pal_y_page = GetPrivateProfileInt( szPalette, "pagey", pal_y_page, szIniFile );
			pal_x_align = GetPrivateProfileInt( szPalette, "alignx", pal_x_align, szIniFile );
			pal_y_align = GetPrivateProfileInt( szPalette, "aligny", pal_y_align, szIniFile );
			char szTransparent[ 128 ];
			sprintf( szTransparent, "%d,%d,%d", rTransparent, gTransparent, bTransparent );
			GetPrivateProfileString( szPalette, "transparent", szTransparent, szTransparent, sizeof( szTransparent ), szIniFile );
			sscanf( szTransparent, "%d,%d,%d", &rTransparent, &gTransparent, &bTransparent );
			// TODO: Add colour cycle file specification

			// Paths
			const char szPath[] = "Path";
                        GetPrivateProfileString( "Texture", (char*)szPath, szTexturePath, szTexturePath, sizeof( szTexturePath ), szIniFile );
                        GetPrivateProfileString( "VRML", (char*)szPath, szVrmlPath, szVrmlPath, sizeof( szVrmlPath ), szIniFile );
		}
		else
		{
			cerr << "Unknown command line option \"" << *argv << "\"" << endl;
		}
	}

	assert( szIniFile );

	assert( x_page > 0 );
	assert( y_page > 0 );

	colTransparent = BR_COLOUR_RGB_555( rTransparent, gTransparent, bTransparent );

	for ( int room=0; room<nRooms; ++room )
	{
		char szRoomName[ 32 ];
		sprintf( szRoomName, "Room%d", room );
		processRoom( szRoomName, x_page, y_page );
	}

	processRoom( "Perm", perm_x_page, perm_y_page );

//	levelPalette->save( "palette.tga" );
//	levelPaletteAlloc->print();
//	delete levelPalette;

	{ // Destruct the textilelog
	*textilelog << "</body>" << endl;
	*textilelog << "</html>" << endl;
	}
	delete textilelog;

	//mc_endcheck();

	return bGlobalError ? 10 : 0;
}
