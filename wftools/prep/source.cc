//=============================================================================
// source.cc:
// By Kevin T. Seghetti
// Copyright (c) 1995-1999,2000,2003 World Foundry Group  
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
//=============================================================================
// todo:

// re-work GetNextLine to allow reading of multiple input lines by one output line
// (this would allow macro definitions to cross lines on input file)

// make it possible to return output line while there is still some input line
// in the buffer (i.e. @n should terminate processing, and return the line,
// and on the next GetNextLine resume processing just past the @n


#include "global.hp"
#include <pigsys/assert.hp>
#include <stdlib.h>
#include <string>
#include <iomanip>
#include <strstream>
#include <map>
using namespace std;

#include <cpplib/stdstrm.hp>
#include <regexp/regexp.hp>
#include <recolib/ktstoken.hp>
#include <eval/eval.h>

#include "prep.hp"
#include "source.hp"
#include "macro.hp"

//=============================================================================

double
source::ParseExpression(const string& expression)
{
	if(expression.length() == 0)
		cerror << SourceError() << " empty expression " << endl;

	DBSTREAM2( cdebug << "ParseExpression: expression string = <" << expression << '>' << endl; )
//	fprintf(stderr,"expression string = %s\n",expression.c_str());
	double retVal = ::eval( expression.c_str(), NULL );
//	fprintf(stderr,"\nafter\n");
	fflush(stderr);
	cout << flush;
	cerr << flush;
	DBSTREAM2( cdebug << "ParseExpression: done, retVal = " << retVal << endl; )
	return retVal;
//	return(0);
}

//=============================================================================

string
source::ApplyMacrosToLine(string& inputLine,map<string,macro,less<string> >& macros)
{
	string line = _ApplyMacrosToLine(inputLine,macros);
	DBSTREAM2 ( cdebug << "line after _ApplyMacrosToLine <" << line << ">" << endl;)
	string outputLine;
	// kts 1/22/96 8:49PM final pass to deal with @@

	ktsRWCTokenizer next(line);

	while(next.ReadChar())					// until end of line
	 {
		if(next.ReadChar() == '@')
		 {
			next+=1;				// skip first @
			assert(next.ReadChar() == '@');
			next+=1;				// skip second  @
			outputLine += '@';
		 }
		else outputLine += next.GetNextChar();		// copy whatever delimiter it is
	 }
	DBSTREAM2( cdebug << "ApplyMacrosToLine finished" << endl;)
	return(outputLine);
}

//=============================================================================

bool
IsWhiteChar(char ch,const char* whiteChars)
{
	assert(whiteChars);
	while(*whiteChars)
	{
		if(ch == *whiteChars)
			return(true);
		whiteChars++;
	}
	return(false);
}

//=============================================================================

string
GetParenSubString(ktsRWCTokenizer& next, const string& error)
{
	DBSTREAM2( cdebug << "GetParenSubString called with next <" << next.restOfString() << ">" << endl; )
	string result;

	while(next.ReadChar() != '(')
	{
		if(next.ReadChar() != ' ' && next.ReadChar() != '\t')
		{
			cerror << SourceError() << error << " missing opening (, found <" << next.restOfString() << "> instead" << endl;
			return(string(""));
		}
		next.GetNextChar();
	}

//	if(next.ReadChar() != '(')
//		cerror << SourceError() << error << " missing opening (, found " << next.restOfString() << " instead" << endl;
	assert(next.ReadChar() == '(');
	next.GetNextChar();
	int nestCount = 1;
	while(nestCount)
	{
//		DBSTREAM2( cdebug << "nestCount = " << nestCount << ", next <" << next.restOfString() << ">" << endl; )
		if(next.restOfString().length() == 0)
		{
			cerror << SourceError() << error << " missing closing )" << endl;
			return result;
		}
		assert(next.restOfString().length() > 0);
		if(next.ReadChar() == '(')
			nestCount++;
		if(next.ReadChar() == ')')
			nestCount--;
		char ch = next.GetNextChar();

		if(nestCount)
			result += ch;
		else
			assert(ch == ')');
	}
	return result;
}

//=============================================================================

char 
lowercase(char input)
{
    if(input >= 'A' && input <= 'Z')
        return input + ('a'-'A');
    return input;
}

char 
uppercase(char input)
{
    if(input >= 'a' && input <= 'z')
        return input + ('A'-'a');
    return input;
}

//=============================================================================

#if 0
command list:

@*                              // comment to end of line
@+      eat white space forward
@-      eat white space backward
@0-9                            // insert ascii constant
@={0-9ifhd}(expression)          // evaulate expression (do math on) (# = width, i=int, f=float,h=hex,d=dec)
@@                              // literal @ in output
@begincomment
@c                              // concatinate next line
@define macro body (to eol or @t)  // define macro
@definem                        // multiline define
@endcomment
@e{0-9}()               // evaluate 
@file                            // expands to the current filename
@if(expression)(body)            // if command
@ifdef(macroname)(body)
@include filename (to eol or @t) // include filename (occurs at end of this line)
@line                           // expands to the current file line 
@lowercase(expression)          // lowercases expression
@n                              // newline
@redefine                           // define macro even if already declared
@redirectoutput filename (to eol or @t) // redirect all future output (including this line) to new file
@redirectend                      // cancel output redirection, future output (including this line) will go to original file
@replace(match expression)(replace expression)(body)
@search(expression)(body)
@strcmp(expr1,expr2)            // returns 1 if equal, 0 otherwise
@strlen(expression)             // returns length of expression
@undef macro                    // undefines macro
@uppercase(expression)          // uppercases expression
@w(expression)(body)      while command

#endif

// loop through a string, doing macro replacments where appropriate

string
source::_ApplyMacrosToLine(string& inputLine,map<string,macro,less<string> >& macros)
{
    static int entryCount=0;
    entryCount++;
    if(entryCount > 1000)
    {
        cerr << SourceError() << "Fatal error: macro loop (entered applymacros over 1000 times)" << endl;
        exit(10);
    }
	string outputLine;
	string token;

	DBSTREAM2( cdebug << "ApplyMacrosToLine called with line <" << inputLine << ">" << endl; )

	int substCount = 0;
	ktsRWCTokenizer next(inputLine);

	while(next.ReadChar())					// until end of line
	{
        DBSTREAM2( cdebug << "AMTL:ReadChar: rest = <" << next.restOfString() << ">" << endl; )
		while(next.MatchNextChar(delimiters))		// walk through delimiters until a token is found
		{
            DBSTREAM2( cdebug << "AMTL:MNC: NextChar = <" << next.ReadChar() << ">,rest = <" << next.restOfString() << ">" << endl; )
			if(next.MatchNextChar(whiteChars))
				outputLine += next.GetWhiteSpace(whiteChars);
			else								// must be a delimiter match
			{
                DBSTREAM2( cdebug << "AMTL:delimeter found: rest = <" << next.restOfString() << ">" << endl; )
				if(next.ReadChar() == '@')
				{
					next+=1;				// skip @
					DBSTREAM2( cdebug << "Parsing @ command, rest of line is <" << next.restOfString() << "> " << endl; )
//					char command = next.ReadChar();
//					DBSTREAM2( cdebug << "Command = <" << command << ">" << endl; )

					ktsRWCTokenizer cmdNext(next.restOfString());

					token = cmdNext(" \t\n("," \t\n");
					DBSTREAM2( cdebug << "InLineParseCommand: token <" << token << ">" << endl; )

					if(token == "begincomment")			// starting with the next line, ignore all lines until endcomment
					{
						next+=token.length();		// eat the command

                        DBSTREAM2( cdebug << "begincomment commentcount++" << endl; )
                        commentCount++;
					}

					else if(token == "endcomment")
					{
						next+=token.length();		// eat the command
                        //cerror << SourceError() << "@endcomment encountered with no @begincomment" << std::endl;
                        assert(commentCount == 0);   
					}

					else if(token[0] == 'e')
					{
                        next+=1;            // skip e
                        string skippedPortion = next.GetWhiteSpace("@e1234567890");           // skip to open paren and continue
                        string expr = GetParenSubString(next,string("@e expression"));
                        DBSTREAM2( cdebug << "@e in mainline, outputLine = <" << outputLine << ">, expr = <" << expr << ">" << endl; )
                        outputLine += _ApplyMacrosToLine(expr,macros);     // yes
                        DBSTREAM2( cdebug << "outputLine after @e expression <" << outputLine << ">" << endl; )
					}
					else if(token[0] == '+')
					{
						next+=1;		//eat the + sign
						// now eat white space up until the next char
						if(next.MatchNextChar(whiteChars))
							next.GetWhiteSpace(whiteChars);
						DBSTREAM2( cdebug << "token = +, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
					}
					else if(token[0] == '-')
					{
						next+=1;		//eat the - sign
						// now eat white space backwards to begin of line or the previous non-white space character

						size_t size = outputLine.length();
						while(size > 0 && IsWhiteChar(outputLine[size-1]," \t"))
							size--;
						outputLine.resize(size,' ');
					}
					else if(token[0] == 'w')			// while command
					{
						next+=1;		// eat the w sign
										// first read in looping expression
						string expr = GetParenSubString(next,string("@w expression"));
						// now read body
						string body = GetParenSubString(next,string("@w body "));
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "expr = <" << expr << ">, body = <" << body << '>' << endl; )

						int cnt = 1;
						while(cnt)
						{
							string tempExpr = _ApplyMacrosToLine(expr,macros);
							cnt = int(ParseExpression(tempExpr));
							DBSTREAM2( cdebug << "  cnt = " << cnt << ", tempExpr = <" << tempExpr << '>' << endl; )
							if(cnt)
								outputLine += _ApplyMacrosToLine(body,macros);
						}
					}
					else if(token == "if")			// inline @if command
					{
						next+=token.length();		// eat the if
										// first read in looping expression
						string expr = GetParenSubString(next,string("@w expression"));
						// now read body
						string body = GetParenSubString(next,string("@if body "));
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "expr = <" << expr << ">, body = <" << body << '>' << endl; )

						string tempExpr = _ApplyMacrosToLine(expr,macros);
						if(ParseExpression(tempExpr))
						   	outputLine += _ApplyMacrosToLine(body,macros);
					}

					else if(token == "ifdef")			// inline @ifdef command
					{
						next+=token.length();		// eat the ifdef
										// first read in looping expression
						string expr = GetParenSubString(next,string("@if macro name"));
						// now read body
						string body = GetParenSubString(next,string("@ifdef body "));
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "expr = <" << expr << ">, body = <" << body << '>' << endl; )
                        if(macros.find(expr) != macros.end())
						   	outputLine += _ApplyMacrosToLine(body,macros);
					}

					else if(token == "ifndef")			// inline @ifndef command
					{
						next+=token.length();		// eat the ifdef
										// first read in looping expression
						string expr = GetParenSubString(next,string("@ifndef macro name"));
						// now read body
						string body = GetParenSubString(next,string("@ifndef body "));
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "expr = <" << expr << ">, body = <" << body << '>' << endl; )

						string tempExpr = _ApplyMacrosToLine(expr,macros);
                        if(macros.find(expr) == macros.end())
						   	outputLine += _ApplyMacrosToLine(body,macros);
					}


					else if(token == "search")			// inline regular expression
					{
						next+=token.length();		// eat the search
										// first read in expression
						string matchExpr = GetParenSubString(next,string("@search match expression"));
						string body = GetParenSubString(next,string("@search body "));
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "matchExpr = <" << matchExpr << ">, body = <" << body << '>' << endl; )

						regexp* re = regcomp(matchExpr.c_str());
						assert(ValidPtr(re));

						int resultCode = regexec(re,body.c_str());
						if(resultCode)
						{
							outputLine += '1';
						}
						else
						{
							outputLine += '0';
						}
					}
					else if(token == "replace")			// inline regular expression
					{
						next+=token.length();		// eat the replace
										// first read in expression
						string matchExpr = GetParenSubString(next,string("@replace match expression"));
						matchExpr = _ApplyMacrosToLine(matchExpr,macros);
						string repExpr = GetParenSubString(next,string("@replace replace expression"));
						repExpr = _ApplyMacrosToLine(repExpr,macros);
						string body = GetParenSubString(next,string("@replace body "));
						body = _ApplyMacrosToLine(body,macros);
						DBSTREAM2( cdebug << "token = w, restOfString after eatwhite <" << next.restOfString() << ">" << endl; )
						DBSTREAM2( cdebug << "matchExpr = <" << matchExpr << ">, body = <" << body << '>' << endl; )

						regexp* re = regcomp(matchExpr.c_str());
						assert(ValidPtr(re));

						int resultCode = regexec(re,body.c_str());
						assert(resultCode == 1);

						char tempBuffer[1000];
						regsub(re,repExpr.c_str(),tempBuffer);
						string result(tempBuffer);
					   	outputLine += result;
					}

					else if(token[0] == '=')				// evaluate expression
					{
						next+=1;		//eat the = sign
						enum
						{
							FLOAT,
							INT
						};
						enum
						{
							DEC,
							HEX
						};
						int output = INT;
						int base = DEC;
						int width=0;
						char last;
						while(next.ReadChar() != '(')
						{	// read output formatting info
							switch(last = next.GetNextChar())
							{
								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':
									width = last - '0';
									break;
								case 'i':
									output = INT;
									break;
								case 'f':
									output = FLOAT;
									break;
								case 'h':
									base = HEX;
									break;
								case 'd':
									base = DEC;
									break;
								default:
									cerror << SourceError() << "expected \"(\" \"i\" or \"f\", found <" << last << next.restOfString() << '>' << endl;
									assert(0);
							}
						}

						// now scan forward to the closing paren
						string sub = GetParenSubString(next,string("@= expression "));

						// now evaluate expression
						sub = _ApplyMacrosToLine(sub,macros);

                        DBSTREAM2( cdebug << "AMTL:Calling ParseExpression with  <" << sub << ">" << endl; )

						double result = ParseExpression(sub);

						ostrstream formattedOutput;
						formattedOutput.width(width);
						formattedOutput << setfill('0');

						if(base == HEX)
							formattedOutput << hex;

						switch(output)
						{
							case FLOAT:
								formattedOutput.setf(ios::showpoint);
								formattedOutput << result;
								break;
							case INT:
								formattedOutput << int(result);
								break;
							default:
								assert(0);
						}
					formattedOutput << '\0';
					outputLine += formattedOutput.str();
					}
#if 1				                    // kts 9/17/97 15:16this was 0, why?
					else if(token[0] == '@')
					{
						next+=1;		// eat the @
						DBSTREAM2( cdebug << "Inserting @" << endl; )
						outputLine += "@@";
					}
#endif
					else if(token[0] == 'c')		// concatenate next line
					{
						next+=1;		// eat the c
						concatNextLine = true;
						DBSTREAM2( cdebug << "setting concatNextLine" << endl; )
					}
					else if(token[0] == 'n')
					{
						next+=1;		// eat the n
						DBSTREAM2( cdebug << "Inserting newline" << endl; )
						outputLine += '\n';
					}
					else if(token == "strlen")
					{
						next+=token.length();		// eat the strlen
						if(next.MatchNextChar(whiteChars))
							next.GetWhiteSpace(whiteChars);
						string val = GetParenSubString(next,string("strlen first "));
						ostrstream formattedOutput;
						formattedOutput << val.length();
						outputLine += formattedOutput.str();
					}
					else if(token == "strcmp")
					{
						next+=token.length();		// eat the strcmp
						if(next.MatchNextChar(whiteChars))
							next.GetWhiteSpace(whiteChars);
						string first = GetParenSubString(next,string("strcmp first "));
						first = _ApplyMacrosToLine(first,macros);
						string second = GetParenSubString(next,string("strcmp second"));
						second = _ApplyMacrosToLine(second,macros);
						if(first == second)
							outputLine += '1';
						else
							outputLine += '0';
					}
					else if(token == "uppercase")					// @uppercase
					{
						next+=token.length();		// eat the uppercase
						if(next.MatchNextChar(whiteChars))
							next.GetWhiteSpace(whiteChars);
						string val = GetParenSubString(next,string("uppercase string"));
						val = _ApplyMacrosToLine(val,macros);

                        // hmm, string class doesn't have lower case function, just do it by hand
                        for(unsigned int index=0;index<val.length();index++)
                            val[index] = uppercase(val[index]);
						outputLine += val;
                    }
					else if(token == "lowercase")					// @lowercase
					{
						next+=token.length();		// eat the uppercase
						if(next.MatchNextChar(whiteChars))
							next.GetWhiteSpace(whiteChars);
						string val = GetParenSubString(next,string("lowercase string"));
						val = _ApplyMacrosToLine(val,macros);

                        // hmm, string class doesn't have lower case function, just do it by hand
                        for(unsigned int index=0;index<val.length();index++)
                            val[index] = lowercase(val[index]);
						outputLine += val;
                    }

					else if(token == "undef")
					{
						next+=token.length();		// eat the undef
						token = next(" \t\n(",whiteChars);								// get name
						DBSTREAM2( cdebug << "Removing macro <" << token << ">" << endl; )
 						macros.erase(token);
					}
					else if(token[0] ==  '*')
					{					// comment to end of line
						while(next.ReadChar())			// eat rest of line
							next+=99999;
					}
					else if(token == "include")		// this causes the include to occur on the next line
				 	{
						next+=token.length();		// eat the include
						assert(includeDepth < MAXIMUM_INCLUDE_DEPTH);

						// copy string to fileName
						string fileName;
						while(next.ReadChar() && next.ReadChar() != '\n' &&
							  (!(next.ReadChar() == '@' && next.ReadChar(1) == 't')))
						 {
							fileName += next.GetNextChar();
						 }
						fileName = _ApplyMacrosToLine(fileName,macros);
						unsigned int offset = 0;
						while(fileName[offset] == ' ' && (offset < fileName.length()) )
							offset++;
						fileName.erase(0,offset);
						includeDepth++;
						OpenFile(fileName);
						assert(input[includeDepth]);
				 	}
					else if(
                            token == "define" ||
                            token == "redefine"
                            )					// define
					{
                        bool redef = (token == "redefine");
						next+=token.length();		// eat the define or redefine
						int defineNestCount=1;
						// copy string to sub
						string sub;
                        bool skipChar;            // kts kludge to handle @e case
						DBSTREAM2( cdebug << "copying portion of string <" << next.restOfString() << "> to sub" << endl; )
						while(next.ReadChar() && next.ReadChar() != '\n' &&
							  (
							  	(!(next.ReadChar() == '@' && next.ReadChar(1) == 't')) ||
								defineNestCount > 1
							  )
							 )
						{
                            skipChar = false;
							DBSTREAM2( cdebug << "next = <" << next.restOfString() << ">, sub = <" << sub << ">" << endl; )
							if((next.ReadChar() == '@' && next.ReadChar(1) == 't'))
								defineNestCount--;

							if(next.ReadChar() == '@' && next.ReadChar(1) == '@')  // if @@,skip them both
								sub += next.GetNextChar();

							if((next.ReadChar() == '@' && next.ReadChar(1) == 'e'))		// force evaluation section
							{
                                // check eval nest level first
                                string tempString = next.restOfString();
                                ktsRWCTokenizer tempNext(tempString);
                                tempNext+=2;            // skip @e
                                int evalNestCount = 32767;
                                string numberString = "";
                                if(tempNext.ReadChar() != '(')              // handle nest counter
                                {
                                    numberString = tempNext.GetWhiteSpace(string("1234567890"));      // kts rename GetWhiteSpace to something more generic
                                    if(tempNext.ReadChar() != '(')
                                        cerror << SourceError() << "expected 0-9, found <" << next.restOfString() << '>' << endl;
                                    evalNestCount = atoi(numberString.c_str());
                                }
                                DBSTREAM2( cdebug << "@e expression, evalNestCount = " << evalNestCount << ", defineNestCount = " << defineNestCount << endl; )
                                string skippedPortion = next.GetWhiteSpace("@e1234567890");           // skip to open paren and continue
                                string expr = GetParenSubString(next,string("@e expression inside of @define "));
                                DBSTREAM2( cdebug << "sub = <" << sub << ">, expr = <" << expr << ">" << endl; )
                                if(evalNestCount == defineNestCount)            // is it time to evauluate it?
                                    sub += _ApplyMacrosToLine(expr,macros);     // yes
                                else            
                                {   
                                    sub += skippedPortion;
                                    sub += "(";
                                    sub += expr;        // otherwise just copy it through unmodified                 
                                    sub += ")";
                                }

                                skipChar = true;
                                DBSTREAM2( cdebug << "sub after @e expression <" << sub << ">" << endl; )
							}

                            {
							const int len=strlen("@define");
							int start = sub.length()-len;
							if(start >= 0)
								if(sub.substr(start,len) == "@define")
									defineNestCount++;
                            }
                            {
                            const int len=strlen("@redefine");
                            int start = sub.length()-len;
                            if(start >= 0)
                                if(sub.substr(start,len) == "@redefine")
                                    defineNestCount++;
                            }
                            if(!skipChar)
							    sub += next.GetNextChar();
						}
                        if(defineNestCount != 1)
                            cerror << SourceError() << "unbalanced @define/@t pairs" << endl;
						assert(defineNestCount == 1);
						// we now have the define command in sub
						DBSTREAM2( cdebug << "Inline define command found, sub = <" << sub << ">" << endl; )
						if((next.ReadChar() == '@' && next.ReadChar(1) == 't'))
							next += 2;				// skip @t
						ktsRWCTokenizer subNext(sub);

						DBSTREAM2( cdebug << "starting on parameter names" << endl; )
						string parameters;
						token = subNext(" \t\n(",whiteChars);								// get name
						string macroString = subNext.restOfString();
						if(macroString.length() && macroString[0] == '(')					// then handle parameter names
						 {
							DBSTREAM2( cdebug << "processing parameter names" << endl; )
							parameters = subNext(")\t\n",whiteChars);
							assert(parameters[0] == '(');
							parameters.erase(0,1);
							macroString = subNext.restOfString();
							if(macroString[0] != ')')
								cerror << SourceError() << "@define parameter list missing closing )" << endl;
							else
								macroString.erase(0,1);			// dump closing paranthesis
						 }

						// overwrite existing macro, or create a new one
						string body = macroString;
                        unsigned int offset;
						for(offset=0;offset < body.length() && body[offset] == ' ';offset++)
							;
						body.erase(0,offset);

						DBSTREAM2( cdebug << "checking if already exists" << endl; )
						if(!redef && macros.find(token) != macros.end())
							DBSTREAM1( cwarn << SourceError() << "redefining macro <" << token << ">, was <" << macros[token] << ">, now <" << macro(token,body,parameters,SourceError()) << ">" << endl ) ;

						macros[token] = macro(token,body,parameters,SourceError());
						DBSTREAM2( cdebug << "done with define" << endl; )
					}
					else if(token == "definem")					// define multiline
					{
						next+=token.length();		// eat the define
						int defineNestCount=1;
						// copy string to sub
						string sub;
						while(next.ReadChar() &&
							  (
							  	(!(next.ReadChar() == '@' && next.ReadChar(1) == 't')) ||
								defineNestCount > 1
							  )
							 )
						 {
							if((next.ReadChar() == '@' && next.ReadChar(1) == 't'))
								defineNestCount--;

							if(next.ReadChar() == '@' && next.ReadChar(1) == '@')  // if @@,skip them both
								sub += next.GetNextChar();

							const int len=strlen("@define");
							int start = sub.length()-len;
							if(start >= 0)
								if(sub.substr(start,len) == "@define")
									defineNestCount++;
							sub += next.GetNextChar();
						 }
                        if(defineNestCount != 1)
                            cerror << SourceError() << "unbalanced @define/@t pairs" << endl;
						assert(defineNestCount == 1);
						// we now have the define command in sub
						DBSTREAM2( cdebug << "Inline define command found, sub = <" << sub << ">" << endl; )
						if((next.ReadChar() == '@' && next.ReadChar(1) == 't'))
							next += 2;				// skip @t
						ktsRWCTokenizer subNext(sub);

						DBSTREAM2( cdebug << "starting on parameter names" << endl; )
						string parameters;
						token = subNext(" \t\n(",whiteChars);								// get name
						string macroString = subNext.restOfString();
						if(macroString.length() && macroString[0] == '(')					// then handle parameter names
						 {
							DBSTREAM2( cdebug << "processing parameter names" << endl; )
							parameters = subNext(")\t\n",whiteChars);
							assert(parameters[0] == '(');
							parameters.erase(0,1);
							macroString = subNext.restOfString();
							if(macroString[0] != ')')
								cerror << SourceError() << "@define parameter list missing closing )" << endl;
							else
								macroString.erase(0,1);			// dump closing paranthesis
						 }
						DBSTREAM2( cdebug << "checking if already exists" << endl; )
						if(macros.find(token) != macros.end())
							DBSTREAM1(cwarn << SourceError() << "redefining macro <" << token << ">" <<endl) ;

						// overwrite existing macro, or create a new one
						string body = macroString;
                        unsigned int offset;
						for(offset=0;offset < body.length() && body[offset] == ' ';offset++)
							;
						body.erase(0,offset);
						macros[token] = macro(token,body,parameters,SourceError());
						DBSTREAM2( cdebug << "done with define" << endl; )
					}
					else if(token == "redirectoutput")		// this causes output to get redirected to filename
                    {
                        next+=token.length();		// eat "redirectoutput"
                        if(alternateOutput)
                            delete alternateOutput;

                        DBSTREAM2( cdebug << "Stashing output line <" << outputLine << ">, clearing output line. Rest of input line <" << inputLine << ">" << std::endl; )

                        partialLine = outputLine;
                        outputLine = "";
						string fileName = GetParenSubString(next,string("@redirectoutput filename"));
                        fileName = _ApplyMacrosToLine(fileName,macros);
                        unsigned int offset = 0;
                        while(fileName[offset] == ' ' && (offset < fileName.length()) )
                            offset++;
                        fileName.erase(0,offset);
                        alternateOutput = new ofstream(fileName.c_str());
                        if(!alternateOutput)
                            cerror << SourceError() << "cannot open file " << fileName << " for output" << endl;
//                        assert(alternateOutput);
                        DBSTREAM2( cdebug << "Opened alternate output file <" << fileName << ">" << endl; )
                    }
					else if(token == "redirectend")		// cancels output redirection
                    {
                        next+=token.length();		// eat "redirectend"
                        if(alternateOutput)
                        {
                            (*alternateOutput) << _ApplyMacrosToLine(outputLine,macros) << endl;   // last bit to output to old file 
                            delete alternateOutput;
                            outputLine = partialLine;                   // get bit from before the redirection back
                            partialLine = "";       
                            alternateOutput = NULL;
                            DBSTREAM2( cdebug << "Restored output line <" << outputLine << ">, clearing partialline" << std::endl; )

                        }
                        else
                            cerror << SourceError() << "output not redirected, cannot end redirection" << endl;
                    }

					else if(token == "file")		// output current filename
                    {
                        next+=token.length();		// eat "file"

                        outputLine += input[includeDepth]->Name();
                    }
					else if(token == "line")		// output current file line
                    {
                        next+=token.length();		// eat "file"

                        char buffer[50];
                        sprintf(buffer,"%ld",input[includeDepth]->LineNum());
                        outputLine += buffer;
                    }

					else if(token[0] >= '0' && token[0] <= '9')					// ascii character 
                    {
                        string numberString = next.GetWhiteSpace(string("1234567890"));      // kts rename GetWhiteSpace to something more generic
                        char constant = atoi(numberString.c_str());
                        outputLine += constant;
                    }
					else
					{
						cerror << SourceError() << "Invalid command <@" << token << ">" <<endl;
						next += token.length();
					}
				}
				else outputLine += next.GetNextChar();		// copy whatever delimiter it is
			}
		}

//		macro var;
		map<string,macro,less<string> >::iterator iter;
		token = next(delimiters,whiteChars);
		DBSTREAM2( cdebug << "Token <" << token << ">" << endl; )
		iter  = macros.find(token);
		if(iter != macros.end())
		 {
			DBSTREAM2( cdebug << "Macro detected, named <" << token << "> with a value of <" << (*iter).second.GetValue() << "> " << endl; )
			outputLine += (*iter).second.DoSubstitution(next,substCount);
		 }
		else
			outputLine += token;
		DBSTREAM2( cdebug << "Input line after macro <" << next.restOfString() << ">, output line = <" << outputLine << ">" << endl; )
	 }
	DBSTREAM2( cdebug << "Output line at end of apply loop <" << outputLine << ">, substCount = " << substCount << endl; )
	if(substCount > 0)
		outputLine = _ApplyMacrosToLine(outputLine,macros);
	DBSTREAM2( cdebug << "Output line at end of ApplyuMacrosToLine <" << outputLine << ">" << endl; )
    entryCount--;
	return(outputLine);
}

//=============================================================================
//=============================================================================

void
source::OpenFile(const string& filename)
{
	input[includeDepth] = new inputFile(filename.c_str());
	assert(input[includeDepth]);
	DBSTREAM2( cdebug << "Opened file <" << filename << ">" << endl; )
}

//=============================================================================

void
source::construct(const string& newFileName)
{
	for(int i=0;i<MAXIMUM_INCLUDE_DEPTH;i++)
		input[i] = NULL;
	includeDepth = 0;
	OpenFile(newFileName);
	assert(input[includeDepth]);
	commentCount = 0;
}

//=============================================================================

source::source(const string& newFileName) : macros()
{
	construct(newFileName);
	concatNextLine = false;
    alternateOutput = NULL;
    partialLine = "";
}

//=============================================================================

source::source(const string& newFileName,map<string,macro, less<string> >& newMacros) : macros()
{
	construct(newFileName);
	macros = newMacros;
    alternateOutput = NULL;
    partialLine = "";
}

//=============================================================================

source::~source()
{
	while(includeDepth)
	{
		delete[] input[includeDepth];
		input[includeDepth] = NULL;
		includeDepth--;
	}
    if(alternateOutput)
        delete alternateOutput;
}

//=============================================================================

#if defined(__LINUX__)
// linux doesn't seem to have itoa


void
reverse(char s[])
{
    char c;

    int i;
    int j = strlen(s);
    if(j > 0)
        for(i = 0; i < (j-1); i++, j--)
        {
            c = s[i];
            s[i] = s[j];
            s[j] = c;
        }
}

#if 0
void
itoa(int n, char s[],int len)
{
    int i;
    int sign;

    if((sign = n) < 0)
        n = -n;
    i = 0;
    do
    {
        s[i++] = n % 10 + '0';
    } while( (n/10) > 0 && i < len);
    if(sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}
#endif


#endif
string
source::FileNameAndLine(int incDepth)
{
	assert(incDepth <= includeDepth);
	assert(input[incDepth]);

	string string;
	char buffer[50];

	string += "<";
	string += input[incDepth]->Name().c_str();
	string += ">, Line #";

#if 1
    sprintf(buffer,"%ld",input[incDepth]->LineNum());
#else
	itoa(input[incDepth]->LineNum(),buffer,10 );
#endif
	string += buffer;
	return(string);
}

//=============================================================================

string
source::SourceError()
{
	string error;
	error += "Prep Error:";
	for(int index=0;index<=includeDepth;index++)
	 {
		if(index != includeDepth)
			error += "\nincluded by: ";
		else if(includeDepth != 0)
			error += "\nin file    : ";
		else
			error += " File: ";

		error += FileNameAndLine(index);
	 }
	error += ": ";
	return(error);
}

//=============================================================================
// returns true if there are lines left, false otherwise

bool
source::LinesLeft() const
{
	return(!(input[0]->rdstate() & ios::eofbit));
}

//=============================================================================

string
source::ReadSourceLine()
{
	string inputLine;
	if(input[includeDepth]->rdstate() & ios::eofbit)
	 {
		if(includeDepth == 0)
		 {
			cerror << SourceError() << "Ran off end of root file" << input[includeDepth]->Name()  << endl;
			exit(10);
		 }
		DBSTREAM2( cdebug << "ReadSourceLine: end of file " << input[includeDepth]->Name() << " to file " << input[includeDepth-1]->Name() << endl; )
		delete input[includeDepth];
		input[includeDepth] = NULL;
		includeDepth--;
	 }
	inputLine = input[includeDepth]->ReadLine();
	return(inputLine);
}

//=============================================================================

string
source::GetNextLine()
{
	string line;
	string result;

	DBSTREAM2( cdebug << "source::GetNextLine:" << endl; )
	// Parse Commands
	do 
	{
		bool cont = true;
		concatNextLine = false;
		while(cont)
		 {
			line = ReadSourceLine();
			DBSTREAM2( cdebug << "================================================================================" << endl; )
			DBSTREAM2( cdebug << "Read source line " << FileNameAndLine(includeDepth) << " <" << line << "> " << endl; )
			if(line.length() == 0)
				return (line);

            // kts kludge to make comment lines which start at the beginning of the line not emit a newline
			if(line.length() && line[0] == '@' && line[1] == '*')
			 {
               // do nothing
               line = "";
			 }
			else
            {
               if(commentCount == 0)
               {
                  cont = false;
               }
               else
               {
                  if(line.find("@endcomment") != std::string::npos)
                  {
                     commentCount = 0;
                  }
               }
            }
               
			if((commentCount == 0) && line.length() > 0)
				cont = false;
		 }
		// kts temporary kludge until I turn this inside out and make it possible for ApplyMacrosToLine to request the next line
		while(line.length() > 2 && line[line.length()-1] == '\\'  && line[line.length()-2] == '@')
		{
			line[line.length()-1] = ' ';
			line[line.length()-2] = ' ';
			line += ReadSourceLine();
		}

		result += ApplyMacrosToLine(line,macros);


        if(alternateOutput)
        {
            (*alternateOutput) << result << endl;
            result = "";
            concatNextLine = true;
        }

	} while(concatNextLine); 

	return(result);
}

//=============================================================================
