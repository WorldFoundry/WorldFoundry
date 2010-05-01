//==============================================================================
// sample.cc
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

#include <cstdio>
#include <cassert>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

#include "symbol.hp"
#include "sample.hp"
typedef unsigned long uint32;

extern vector<Symbol> tblSymbol;
extern double cutoffPercentage;
extern ofstream* log;
extern bool bBarGraph;
extern bool bHtml;
extern bool bRelative;

Symbol*
xfind( unsigned long addr )
{
//	cout << "Looking for " << hex << addr << endl;
	vector<Symbol>::iterator symbol;
	for ( symbol = tblSymbol.begin(); symbol != tblSymbol.end(); ++symbol )
	{
		if ( addr >= (*symbol).Address() )
		{
#if 0
			cout << (*symbol).Function() << '=' <<
				hex << (*symbol).Address() <<
				" len=" << (*(symbol-1)).Address() - (*symbol).Address() <<
				endl;
#endif
			return &*symbol;
			break;
		}
	}
//	cout << endl;
	return NULL;
}



Sample::Sample( FILE* fpSample )
{
	assert( fpSample );

	int cbRead;

	unsigned long tag;
	fread( &tag, sizeof( unsigned long ), 1, fpSample );
	assert( tag == 'SAMP' );
	unsigned long size;
	fread( &size, sizeof( unsigned long ), 1, fpSample );
	size -= 3*4;

	fread( &unknownHits, sizeof( unsigned long ), 1, fpSample );
	fread( &nSampleRate, sizeof( unsigned long ), 1, fpSample );
	fread( &__text, sizeof( unsigned long ), 1, fpSample );
	fread( &__textlen, sizeof( unsigned long ), 1, fpSample );
	assert( (__textlen % 4) == 0 );
	nEntries = __textlen / 4;

	textdata = (unsigned long*)malloc( __textlen );
	assert( textdata );
	cbRead = fread( textdata, 1, __textlen, fpSample );
	assert( cbRead == __textlen );
	fclose( fpSample );

	nSamplesTotal = 0;
	uint32* pFollow = textdata;
	for ( int i=0; i<nEntries; ++i )
		nSamplesTotal += *pFollow++;

	nSamplesTotal += unknownHits;
}


Sample::~Sample()
{
	assert( textdata );
	free( textdata );
}


template <class T>
struct greater_freq : public binary_function< T, T, bool >
{
	bool operator()( const T& x_, const T& y_ ) const
	{
		if ( y_._Frequency == x_._Frequency )
			return x_.Function() < y_.Function();
		else
			return y_._Frequency < x_._Frequency;
	}
};


void
Sample::calculatePercentages()
{
	uint32* pFollow = textdata;
	int i;
	for ( i=0; i<nEntries; ++i, ++pFollow )
	{
		if ( *pFollow )
		{
			Symbol* sym = xfind( (unsigned long)( (char*)pFollow - (char*)textdata + (char*)__text ) );
			if ( sym )
				sym->_Frequency += *pFollow;
		}
	}

	// Sort by frequency
	tblSymbol = tblSymbol;
	greater_freq<Symbol> ffreq;
	sort( tblSymbol.begin(), tblSymbol.end(), ffreq );
}


const char*
barGraph( double percentage, int nWidth )
{
	static char szBar[ 80 ];
	char* pszBar = szBar;

	int nHalfChars = _MAX( 1, int( percentage * nWidth * 2 ) );
	for ( int i=0; i<nHalfChars-1; i+=2 )
		*pszBar++ = 'Û';
	if ( nHalfChars & 1 )
		*pszBar++ = 'Ý';
	*pszBar = '\0';

	return szBar;
}


ios& showpoint( ios& strios )
{
	strios.setf( ios::showpoint );
	return strios;
}


void
Sample::print()
{
	double unknownPercentage = ((double)unknownHits ) / nSamplesTotal;
	double firstPercentage = (double)(*(tblSymbol.begin()))._Frequency / nSamplesTotal;
	int maxWidth = int( 78.0 / _MAX( unknownPercentage, firstPercentage ) );

	showpoint( cout );
	cout.precision( 4 );

	if ( bHtml )
	{
		cout << "<html>" << endl;
		cout << endl;
		cout << "<head>" << endl;
		cout << "<title>psxprof</title>" << endl;
		cout << "</head>" << endl;
		cout << endl;
		cout << "<script>" << endl;
		cout << "function expandCollapse( id )" << endl;
		cout << "{" << endl;
		cout << "\tobj = document.all[ id ];" << endl;
		cout << "\tisDisplayed = obj.style.display;" << endl;
		cout << "\tif ( isDisplayed == \"none\" )" << endl;
		cout << "\t\tisDisplayed = \"\";" << endl;
		cout << "\telse" << endl;
		cout << "\t\tisDisplayed = \"none\";" << endl;
		cout << "\tobj.style.display = isDisplayed;" << endl;
		cout << "}" << endl;
		cout << "</script>" << endl;
		cout << endl;
		cout << "<body>" << endl;
		cout << endl;
		cout << "<h1>psxprof</h1>" << endl;
		cout << "<p>__text = $" << hex << __text << dec << "</p>" << endl;
		cout << endl;

		double cumulativePercentage = 0.0;

		double highestPercentage = (*(tblSymbol.begin()))._Frequency;
		highestPercentage /= nSamplesTotal;

		vector<Symbol>::iterator symbol;
		for ( symbol = tblSymbol.begin(); symbol != tblSymbol.end(); ++symbol )
		{
			if ( symbol->_Frequency >= cutoffPercentage )
			{
				double percentage = ((double)( symbol->_Frequency)) / nSamplesTotal * 100.0;
				cout << "<p>" << endl;
				cout << "<INPUT TYPE=BUTTON VALUE=\"" << percentage << "%\" ONCLICK=\"expandCollapse( '" << symbol->Function() << "' );\"> ";
				cout << "<font face=\"Courier New\"><small>" << symbol->Function() << "()<br>" << endl;

				if ( percentage )
				{
					char* colorBar;
					if ( 0 )
						;
					else if ( percentage > 20.0 )
						colorBar = "purple";
					else if ( percentage > 10.0 )
						colorBar = "red";
					else if ( percentage > 5.0 )
						colorBar = "orange";
					else if ( percentage < 1.0 )
						colorBar = "green";
					else
						colorBar = "yellow";

					const height = 18;

					if ( bRelative )
					{
						double relativePercentage = percentage / highestPercentage;
						cout << "<hr width=\"" << relativePercentage << "%\" align=\"left\" size=\"" << int( height ) << "\" noshade color=\"" << colorBar << "\">" << endl;
					}
					else
					{
						cout << "<table border=\"0\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
						cout << "<tr>" << endl;
						cout << "<td width=\"" << cumulativePercentage << "%\"></td>" << endl;
    					cout << "<td width=\"" << percentage << "%\" bgcolor=\"" << colorBar << "\" height=\"18\"></td>" << endl;
						double remainingPercentage = 100.0 - cumulativePercentage - percentage;
    					cout << "<td width=\"" << remainingPercentage << "%\"></td>" << endl;
  						cout << "</tr>" << endl;
						cout << "</table>" << endl;
					}

					cout << "<div STYLE=\"display: 'none';\" ID=" << symbol->Function() << ">" << endl;

					cout << "$" << hex << symbol->Address() << "..$" << symbol->Address() + symbol->_size << dec << "</small></font></p>" << endl;

					// Output address within range
					uint32 lower = symbol->Address();
					uint32 upper = lower + symbol->_size;

					int nFunctionHits = 0;
					uint32* pFollow = &( textdata[ ( lower - __text ) / sizeof( uint32 ) ] ) ;
					for ( int i=lower; i<upper; i+=sizeof( uint32 ), ++pFollow )
						nFunctionHits += *pFollow;

					pFollow = &( textdata[ ( lower - __text ) / sizeof( uint32 ) ] ) ;
					for ( i=lower; i<upper; i+=sizeof( uint32 ), ++pFollow )
					{
						if ( *pFollow )
						{
							cout << "<table border=\"0\" width=\"100%\" cellspacing=\"1\" cellpadding=\"0\">" << endl;
					 			cout << "<tr>" << endl;
									cout << "<td bgcolor=\"#808000\"><font face=\"Courier New\"><small>$" << hex << i << dec << "</small></font></td>" << endl;
									cout << "<td bgcolor=\"#C0C0C0\"><small>" << setw( 3 ) << *pFollow << "</small></td>" << endl;
									double functionPercentage = *pFollow * 100.0 / nFunctionHits;
		    						cout << "<td width=\"" << functionPercentage << "%\" bgcolor=\"" << colorBar << "\" height=\"6\"></td>" << endl;
									double remainingPercentage = 100.0 - functionPercentage;
    								cout << "<td width=\"" << remainingPercentage << "%\"></td>" << endl;
	  							cout << "</tr>" << endl;
							cout << "</table>" << endl;
						}
					}
					cout << "</div>" << endl;
				}

				cumulativePercentage += percentage;
			}
		}

		cout << "</body>" << endl;
		cout << "</html>" << endl;
	}
	else
	{
		cout << unknownPercentage*100.0 << "% ";
		cout << setw( -LABEL_WIDTH ) << "Unknown" << endl;
		if ( bBarGraph )
			cout << barGraph( unknownPercentage, maxWidth ) << endl;

		showpoint( *log );
		(*log).precision( 4 );

		*log << unknownPercentage*100.0 << "% ";
		*log << setw( -LABEL_WIDTH ) << "Unknown" << endl;
		if ( bBarGraph )
			*log << barGraph( unknownPercentage, maxWidth ) << endl;

		vector<Symbol>::iterator symbol;
		for ( symbol = tblSymbol.begin(); symbol != tblSymbol.end(); ++symbol )
		{
			if ( symbol->_Frequency >= cutoffPercentage )
			{
				double percentage = ((double)( symbol->_Frequency)) / nSamplesTotal;
				cout << setw( 6 ) << percentage*100.0 << "% ";
				cout << setw( -LABEL_WIDTH ) << symbol->Function() << "()" << endl;
				if ( bBarGraph )
					cout << barGraph( percentage, maxWidth ) << endl;

				*log << setw( 6 ) << percentage*100.0 << "% ";
				*log << setw( -LABEL_WIDTH ) << symbol->Function() << "()" << endl;
				if ( bBarGraph )
					*log << barGraph( percentage, maxWidth ) << endl;
			}
		}
	}
}
