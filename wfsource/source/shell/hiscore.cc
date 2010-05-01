// hiscore.cc
// High Score Table
// Copyright 1995 Cave Logic Studios, Ltd.  

#include <fstream.h>
#include <pigsys/assert.hp>
#include <cstring>
#include <cstdlib>

#include "score.hp"

// This implementation uses dynamically allocated strings for the
// names.  Although using fixed-sized entries could speed up the
// loading and saving (theoretically), but I think that the space
// issues are much more important for embedded systems since our
// applications have to share NVRAM with many other games.  Besides,
// as with all correctly designed C++ classes, changing the
// implementation without affecting the interface.
// If a fixed-size implementation is desired, however, it might be
// wise to make a common class and then derive the fixed and variable
// version classes.


// NOTES: Need an iterator (how about generic list classes?)

// ISSUES: How do I get the name of the application (game)?


typedef char* HighScoreName;


class HighScoreEntry
	{
public:
	HighScoreEntry( HighScoreName name="", CScore score = CScore(0) );
	~HighScoreEntry();

	HighScoreName _name;
	CScore _score;
	};


HighScoreEntry::HighScoreEntry( HighScoreName name, CScore score )
	{
	_name = strdup( name );
	assert( _name );
	_score = score;
	}


HighScoreEntry::~HighScoreEntry()
	{
	}



class HighScoreTable
	{
public:
	HighScoreTable( const int nEntries );	// constructor for a blank table
	HighScoreTable();						// constructor to load saved high score table
	~HighScoreTable();

	Bool isHighScore( HighScoreEntry& hse ) const;
	int addHighScore( HighScoreEntry& hse );
	int nEntries() const;

	void render() const;
	Bool load( const char* szFilename );
	Bool save() const;

//private:
	int _nEntries;
	HighScoreEntry* _hst[];
	};


HighScoreTable::HighScoreTable( const int nEntries )
	{
	_nEntries = nEntries;

	*_hst = (HighScoreEntry*)malloc( _nEntries * sizeof( HighScoreEntry* ) );
	assert( _hst );
	for ( int i=0; i<nEntries; ++i )
		_hst[i] = new HighScoreEntry();
	}


// For PC version, use .ini files.  For embedded systems, use portable
// file routines.
HighScoreTable::HighScoreTable()
	{
	load( "hiscore.tbl" );
	//assert?
	}


Bool
HighScoreTable::load( const char* szFilename )
	{
	uint32 score;
	char name[ 256 ];

	ifstream in( szFilename );
	assert( in );

	in >> _nEntries;
	for ( int i=0; i<nEntries(); ++i )
		{
		in >> name;
		in >> score;
		_hst[i] = new HighScoreEntry( name, CScore( score ) );
		}
	return TRUE;
	}


HighScoreTable::~HighScoreTable()
	{
	for ( int i=0; i<nEntries(); ++i )
		{
		delete _hst[i]->_name;
		_hst[i]->_name = NULL;
		}
	}


void
HighScoreTable::render() const
	{
	cout << endl << "HIGH SCORES" << endl;
	for ( int i=0; i<nEntries(); ++i )
		{
		if ( *_hst[i]->_name )
			cout << '[' << _hst[i]->_name << "]\t"
			     << _hst[i]->_score.asString() << endl;
		}
	}


Bool
HighScoreTable::save() const
	{
	ofstream out( "hiscore.tbl" );
	assert( out );

	out << nEntries() << endl;
	for ( int i=0; i<nEntries(); ++i )
		out << _hst[i]->_name << '\t' << _hst[i]->_score() << endl;
		// !! Note-- change from score() to just score (with stream
		// operators defined in the CScore class)
	return TRUE;
	}


int
HighScoreTable::nEntries() const
	{
	return _nEntries;
	}


Bool
HighScoreTable::isHighScore( HighScoreEntry& hse ) const
	{
	return (Bool)( hse._score() > _hst[ _nEntries-1 ]->_score() );
	}


HighScoreTable::addHighScore( HighScoreEntry& hse )
	{
	assert( isHighScore( hse ) );

	for ( int i=0; i<nEntries(); ++i )
		{
		if ( hse._score() > _hst[ i ]->_score() )
			{
			// Move all of the entries down one space to make room for new
			for ( int j=i+1; j<nEntries(); ++j )
				_hst[ j ] = _hst[ j-1 ];
			_hst[ i ] = &hse;
			return i;
			}
		}
	return 0;		// shouldn't get here
	}



#define TEST_MAIN

#if defined( TEST_MAIN )
int
main( int argc, char* argv[] )
	{
	HighScoreEntry hs1( "WBN", 10000 );
	HighScoreEntry hs2( "KTS", 9000 );

	{
	HighScoreTable hst_loaded;
	hst_loaded.render();
	}


	HighScoreTable hst( 10 );

	hst.render();

	if ( hst.isHighScore( hs1 ) )
		hst.addHighScore( hs1 );

	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();

	Bool rc = hst.save();
	assert( rc );

	hst.addHighScore( hs2 );
	hst.render();
	hst.addHighScore( hs2 );
	hst.render();

	return 0;
	}
#endif
