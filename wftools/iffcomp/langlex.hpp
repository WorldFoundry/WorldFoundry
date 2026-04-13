// langlex.hpp
#ifndef LANGLEX_HPP
#define LANGLEX_HPP

#include <cstdio>
#include <string>
#include <vector>
#include "location.hh"
#include "fileline.hpp"

class Grammar;

class LangLexer
{
public:
    LangLexer( Grammar& g, const char* szInputFile );
    ~LangLexer();

    void push_include( const char* szIncludeFile );
    void push_system_include( const char* szIncludeFile );
    bool pop_include();

    void        nextLine()          { ++include()->nLine; }
    int         line()              { return include()->nLine; }
    const char* filename();
    const char* currentLine();
    const char* currentLine( const char* );

    yy::location& loc() { return _loc; }

    std::vector< FileLineInfo* > _fileLineInfo;

private:
    FileLineInfo* include();

    Grammar&    _g;
    std::FILE*  _rootFile;
    yy::location _loc;
};

#endif  // LANGLEX_HPP
