// langlex.hpp
#ifndef LANGLEX_HPP
#define LANGLEX_HPP

#include <vector>
#include "fileline.hpp"

#undef yyFlexLexer
#include <FlexLexer.h>

class strFlexLexer : public yyFlexLexer
{
public:
	strFlexLexer( std::istream* arg_yyin = 0, std::ostream* arg_yyout = 0 );
	virtual ~strFlexLexer()		{}
  	int yywrap();

	void push_include( const char* szIncludeFile );
	void push_system_include( const char* szIncludeFile );
	bool pop_include();

	void nextLine()		       	{ ++include()->nLine; }
	int line()		       	{ return include()->nLine; }
	const char* filename();

	const char* currentLine();
	const char* currentLine( const char* );

	std::vector< FileLineInfo* > _fileLineInfo;
protected:
	FileLineInfo* include();
};

#endif 	// LANGLEX_HPP
