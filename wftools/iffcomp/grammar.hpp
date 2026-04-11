// grammar.hpp

#ifndef GRAMMAR_HP
#define GRAMMAR_HP

#include <cstdlib>
#include <iosfwd>
#include <string>
#include <vector>
#include <iffwrite/iffwrite.hp>
#include <iffwrite/fixed.hp>

class LangLexer;

class Backpatch
{
public:
    enum BackpatchType {
        TYPE_SIZEOF,
        TYPE_OFFSETOF
    } _type;

    explicit Backpatch( BackpatchType, int offset = 0 );

    long         pos;
    std::string  szChunkIdentifier;
    int          _offset;

private:
    Backpatch();
};


struct State
{
    State() : _sizeOverride( 0 ), _precision{ 1, 15, 16 } {}

    int sizeOverride( int so )               { return _sizeOverride = so; }
    int sizeOverride() const                 { return _sizeOverride; }

    size_specifier precision( size_specifier ss ) { return _precision = ss; }
    size_specifier precision() const              { return _precision; }

protected:
    int             _sizeOverride;
    size_specifier  _precision;
};


class Grammar
{
public:
    Grammar( const char* szInputFile, const char* szOutputFile );
    virtual ~Grammar();

    int yyparse();

    void Error( const char* msg, ... );
    void Warning( const char* msg, ... );

    const char* filename() const { return _filename; }

    ////////////////////////////////////////////////////////////////////////////
    std::vector< State >       vecState;
    std::vector< Backpatch* >  _backpatchSizeOffset;

    _IffWriter*     _iff;
    std::ofstream*  binout;
    char            szOutputFile[ PATH_MAX ];

    std::ofstream*  error;
    int             _nErrors;

private:
    void construct( const char* szInputFile, const char* szOutputFile );
    void printIncludeList() const;

    Grammar() = delete;
    Grammar( const Grammar& ) = delete;
    Grammar& operator=( const Grammar& ) = delete;

    char            szErrorFile[ PATH_MAX ];
    char            _filename[ PATH_MAX ];
    int             _level;
    LangLexer*      _lex;    // only non-null during yyparse()
};

#endif  // GRAMMAR_HP
