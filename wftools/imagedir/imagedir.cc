//==============================================================================
// imagedir.cc
// Copyright (c) 1998-1999, World Foundry Group  
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

#include <iostream>
#include <istream>
#include <ostream>
#include <pigsys/assert.hp>
#include <io.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <cstring>
#include <vector>
using namespace std;
#include <gfxfmt/tga.hp>
// temp kludges for pigs inclusion
bool bVerbose = false;
bool bDebug = false;
uint16 colTransparent = 0;

//using namespace std;

int col = 0;

#if 0
struct tm
{
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */

struct _finddata_t
{
    time_t      time_create;    /* -1 for FAT file systems */
    time_t      time_access;    /* -1 for FAT file systems */
#endif

struct ImageSpecification
{
	int width;
	int height;
	int bitdepth;
	bool bCompressed;
	char szCompressionType[ 256 ];
};


const char*
sizeString( int size )
{
	static char szSize[ 100 ];

	sprintf( szSize, "%d bytes", size );

	return szSize;
}


#pragma message( "TODO: move BitmapFactory to gfxfmt library" )
Bitmap*
BitmapFactory( istream& input, const char* szFilename )
{
	Bitmap* bm = NULL;

	const char* szExt = strrchr( szFilename, '.' );
	if ( szExt )
	{
		++szExt;

		ifstream* input = new ifstream( szFilename, ios::binary );
		assert( input );

		if ( stricmp( szExt, "bmp" ) == 0 )
		{
		}
		else if ( stricmp( szExt, "tga" ) == 0 )
		{
			bm = new TargaBitmap( *input, szFilename );
			assert( bm );
		}
		else if ( stricmp( szExt, "gif" ) == 0 )
		{
		}
		else if ( stricmp( szExt, "jpg" ) == 0 )
		{
		}

		delete input;
	}

	return bm;
}


void
GetImageSpecification( struct _finddata_t& fd, ImageSpecification& is )
{
	// default is unknown
	is.width = is.height = is.bitdepth = 0;
	is.bCompressed = false;
	*is.szCompressionType = '\0';

	ifstream* input = new ifstream( fd.name, ios::binary );
	assert( input );

	Bitmap* bm = BitmapFactory( *input, fd.name );
	if ( bm )
	{
		is.width = bm->width();
		is.height = bm->height();
		is.bitdepth = bm->bitdepth();
		delete bm;
	}

	delete input;
}


void
processFile( const std::string szFullDir, /*const std::string szDir,*/ struct _finddata_t& fd )
{
	if ( col == 0 )
		std::cout << "<tr>" << std::endl;

	struct tm tm;
	tm = *localtime( &fd.time_write );

	std::string szFullName = szFullDir + "/" + std::string( fd.name );

	// ** FIRST COLUMN

	std::cout << "<td width=\"180\" valign=\"top\" align=\"left\" bgcolor=\"black\">"
		<< "<a href=\"" << fd.name << "\"><h3><strong><font size=\"2\" color=\"white\">" << fd.name << "</font></strong></h3></a>"
		<< "<p>"
		<< "<font size=\"1\" color=\"white\">"
		<< sizeString( fd.size ) << "<br>";

	ImageSpecification is;
	GetImageSpecification( fd, is );

#pragma message( "TODO: Convert # colours display to function" )
	std::cout << is.width << " x " << is.height << "<br>"
		<< ( 1L << is.bitdepth ) << " colours<br>";
	if ( is.bCompressed )
		std::cout << "Compressed<br>" << std::endl;

	// FILE TIMESTAMPS
	std::cout << "<HR>" << std::endl;
	std::cout << "Written: " << asctime( &tm ) << "<br>";

	if ( fd.time_create != -1 )
	{
		struct tm created_tm;
		created_tm = *localtime( &fd.time_create );
		std::cout << "Created: " << asctime( &created_tm ) << "<br>";
	}

	if ( fd.time_access != -1 )
	{
		struct tm accessed_tm;
		accessed_tm = *localtime( &fd.time_access );
		std::cout << "Accessed: " << asctime( &accessed_tm ) << "<br>";
	}

    std::cout << "</font>"
		<< "</td>" << std::endl;

	// ** NEXT COLUMN

	// IMAGE
	std::cout << "<td width=\"200\" valign=\"top\" align=\"left\">"
		<< "<a href=\"\" onclick=\"return openImageWindow( '" << szFullName.c_str() << "', " << is.width << ", " << is.height << " );\">" // szFullName.c_str() << "\">"
		<< "<img src=\"" << szFullDir.c_str() << "/" << fd.name << "\" "
		<< "border=\"0\""
		<< "alt=\"" << fd.name << "\" "
		<< "width=\"200\" height=\"130\" "
		<< "></a></td>" << std::endl;

	if ( ++col == 2 )
	{
		col = 0;
		std::cout << "</tr>" << std::endl;
	}
}

std::vector< std::string > _tblDirs;

void
processDir( std::string szFullPath )	//, const char* szDir )
{
	col = 0;

	std::cout << std::endl;

	std::string szBaseDir( szFullPath, szFullPath.rfind( '/' ) + 1, ~0 );

	std::cout << "<h2>" << szBaseDir << "</h2>" << std::endl;
	std::cout << "<p><a href=\"" << szFullPath << "\">" << szFullPath << "</a></p>" << std::endl;
	std::cout << "<div align=\"left\">" << std::endl;
	std::cout << std::endl;
	std::cout << "<table border=\"0\" width=\"100%\">" << std::endl;
	std::cout << std::endl;

	std::string szFilespec( szFullPath );
	szFilespec += "/*.*";

	struct _finddata_t fd;

	int handle;
	handle = _findfirst( szFilespec.c_str(), &fd );
	while ( handle != -1 )
	{
		if ( fd.attrib & _A_SUBDIR )
		{
			if ( !( ( strcmp( fd.name, "." ) == 0 ) || ( strcmp( fd.name, ".." ) == 0 ) ) )
				_tblDirs.push_back( szFullPath + "/" + std::string( fd.name ) );
		}
		else
		{
			const char* szExt = strrchr( fd.name, '.' );
			if ( szExt )
			{
				++szExt;
				if ( 0
					|| ( stricmp( szExt, "bmp" ) == 0 )
					|| ( stricmp( szExt, "tga" ) == 0 )
					|| ( stricmp( szExt, "gif" ) == 0 )
					|| ( stricmp( szExt, "jpg" ) == 0 )
				)
					processFile( szFullPath, fd );
			}
		}
		if ( _findnext( handle, &fd ) == -1 )
			break;
	}

	std::cout << "</table>" << std::endl;
	std::cout << "</div>" << std::endl;

	while ( _tblDirs.size() )
	{
		std::string szDir = *_tblDirs.begin();
		_tblDirs.erase( _tblDirs.begin() );
		processDir( szDir );
	}
}


void
imagedir( const char* szDir )
{
	std::cout << "<html>" << std::endl;
	std::cout << std::endl;
	std::cout << "<head>" << std::endl;
	std::cout << "<title>Image Directory</title>" << std::endl;
	std::cout << "<meta name=\"GENERATOR\" content=\"ImageDir\">" << std::endl;
	std::cout << "</head>" << std::endl;
	std::cout << std::endl;

	std::cout << "<SCRIPT TYPE=\"text/JavaScript\">" << std::endl;
	std::cout << "function openImageWindow( szImage, width, height )" << std::endl;
	std::cout << "{" << std::endl;
	std::cout << "\twindow.status = \"\";" << std::endl;
	std::cout << "\tif ( width == 0 ) width = 350;" << std::endl;
	std::cout << "\tif ( height == 0 ) height = 400;" << std::endl;
	std::cout << std::endl;
	std::cout << "// for the margin" << std::endl;
	std::cout << "\twidth += 20;" << std::endl;
	std::cout << "\theight += 25;" << std::endl;
	std::cout << std::endl;
	std::cout << "\tobjNewWindow = window.open( szImage, \"image\", \"top=100,left=100,width=\"+width+\",height=\"+height+\",toolbar=no,menubar=no,location=yes,resizable=yes\" );" << std::endl;
	std::cout << "\twindow.status = \"Opened a new browser window.\";" << std::endl;
	std::cout << "\twindow.event.cancelBubble = true;" << std::endl;
	std::cout << "\treturn ( window.event.returnValue = false );" << std::endl;
	std::cout << "}" << std::endl;
	std::cout << "</SCRIPT>" << std::endl;
	std::cout << std::endl;

	std::cout << "<body>" << std::endl;
	std::cout << "<font face=\"Tahoma\">" << std::endl;
	std::cout << std::endl;

	std::cout << "<div align=\"left\">" << std::endl;
	std::cout << "<table border=\"0\" width=\"100%\">" << std::endl;
	std::cout << "<tr>" << std::endl;
	std::cout << "<td width=\"100%\" bgcolor=\"black\">" << std::endl;
	std::cout << "<h1><font color=\"white\">Image Directory</font></h1>" << std::endl;
	std::cout << "<p><em><font size=\"2\" color=\"white\">by William B. Norris IV<br></font>";
	std::cout << "<font size=\"1\" color=\"white\">Copyright &copy; 1997-1999 World Foundry Group.</font>";
	std::cout << "</em>";
	std::cout << "</td>" << std::endl;
	std::cout << "</tr>" << std::endl;
	std::cout << "</table>" << std::endl;
	std::cout << "</div>" << std::endl;

	processDir( szDir );

	std::cout << "</font>" << std::endl;
	std::cout << "</body>" << std::endl;
	std::cout << "</html>" << std::endl;
}

void
usage( int argc, char* argv[] )
{
	std::cout << "ImageDir v1.1.0  Copyright 1997-1999 World Foundry Group." << std::endl;
	std::cout << "by William B. Norris IV" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "\timagedir <startDir>" << std::endl;
}


int
main( int argc, char* argv[] )
{
	if ( argc != 2 )
	{
		usage( argc, argv );
		return 1;
	}

	char* szInitialDir = argv[ 1 ];

	imagedir( szInitialDir );

	return 0;
}
