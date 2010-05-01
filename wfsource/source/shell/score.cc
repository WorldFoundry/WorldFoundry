
#include <cstdio>
#include <cstdlib>

#include "score.hp"


CScore::CScore()
	{
	}


CScore::CScore( uint32 theScore )
	{
	_theScore = theScore;
	}


CScore::~CScore()
	{
	}


CScore&
CScore::operator=( const CScore& other )
	{
	_theScore = other._theScore;
	return *this;
	}


Bool
CScore::operator<( const CScore& score2 )
	{
	return (Bool)( _theScore < score2._theScore );
	}


Bool
CScore::operator<=( const CScore& score2 )
	{
	return (Bool)( _theScore <= score2._theScore );
	}


Bool
CScore::operator>( const CScore& score2 )
	{
	return (Bool)( _theScore > score2._theScore );
	}


Bool
CScore::operator>=( const CScore& score2 )
	{
	return (Bool)( _theScore >= score2._theScore );
	}


Bool
CScore::operator==( const CScore& score2 )
	{
	return (Bool)( _theScore == score2._theScore );
	}


uint32
CScore::operator()()
	{
	return _theScore;
	}


const char*
CScore::asString()
	{
	static char buffer[ 15 ];

	ltoa( _theScore, buffer, 10 );
	return buffer;
	}


#if defined( TEST_MAIN )
int
main( int argc, char* argv[] )
	{
	CScore score1( 450 );
	CScore score2( 10000 );

	printf( "score1: %d\n", score1() );
	printf( "score2: %d\n", score2() );
	score1 = score2;
	printf( "score1: %d\n", score1 = CScore(666) );

	printf( "score1 < score2: %d\n", score1 < score2 );
	printf( "score2 < score1: %d\n", score2 < score1 );

	score1 = score2;
	printf( "score1 == score2: %d\n", score1 == score2 );

	return 0;
	}
#endif
