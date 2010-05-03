// grammar.hpp

#ifndef GRAMMAR_HP
#define GRAMMAR_HP

#include <stdlib.h>
#include <iffwrite/iffwrite.hp>
#include <vector>
int yyparse();
#include <iffwrite/fixed.hp>

class Backpatch
{
public:
	enum BackpatchType {
		TYPE_SIZEOF,
		TYPE_OFFSETOF
	} _type;
	explicit Backpatch( BackpatchType, int offset = 0 );
	long pos;
	char* szChunkIdentifier;
	int _offset;

private:
	Backpatch();
};


struct State
{
	State()	{}
	int sizeOverride( int so )	{ return _sizeOverride = so; }
	int sizeOverride() const	{ return _sizeOverride; }

	size_specifier precision( size_specifier ss )	{ return _precision = ss; }
	size_specifier precision() const	{ return _precision; }

protected:
	int _sizeOverride;
	size_specifier _precision;
};


class Grammar
{
public:
	Grammar( const char* _szInputFile, const char* _szOutputFile );
	Grammar( const char* _szInputFile, std::istream* input );
	virtual ~Grammar();

	int yyparse();

	std::ofstream* error;
	void Error( const char* msg, ... );
	void Warning( const char* msg, ... );

	const char* filename() const	{ return _filename; }

	////////////////////////////////////////////////////////////////////////////////
	std::vector< State > vecState;
	std::vector< Backpatch* > _backpatchSizeOffset;

	_IffWriter* _iff;
	std::ofstream* binout;
	char szOutputFile[ PATH_MAX ];

	int _nErrors;
protected:
	char szErrorFile[ PATH_MAX ];

	char _filename[ PATH_MAX ];

	int _level;
	std::ostream* verbose;

protected:
	void printIncludeList() const;
	std::ifstream* _inp;

private:
	void construct( const char* szInputFile, const char* _szOutptuFile );
	Grammar();
};
extern Grammar* theGrammar;

#endif	// GRAMMAR_HP
