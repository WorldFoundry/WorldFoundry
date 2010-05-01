//=============================================================================
// macro.cc: C pre-processor macro (string)
// By Kevin T. Seghetti
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

#include "global.hp"
#include <string>
#include <pigsys/assert.hp>

#include <cpplib/stdstrm.hp>
#include <algorithm>
using namespace std;
#include "macro.hp"
#include "prep.hp"				// kts for now
#include <recolib/ktstoken.hp>

//=============================================================================

macro::macro()
{
//	DBSTREAM2( cdebug << "macro default constructor:" << endl; )
   whereDefined = "Bogus";
}

//=============================================================================

#define EMPTY_DEFAULT_ARGUMENT ")"

macro::macro(const string& newName,const string& newValue, const string& newParameters, const string& newWhereDefined)
{
	name = newName;
	value = newValue;
    whereDefined = newWhereDefined;

    assert(whereDefined.length());

	DBSTREAM2( cdebug << "macro constructor: name = <" << newName << ">, string = <" << newValue << "> parameters = <" << newParameters << ">, newWhereDefined = " << newWhereDefined << endl; )

	ktsRWCTokenizer next(newParameters);
	string token;

	while((token = next(", \t\n"," \t\n")).length())
	{
		DBSTREAM2( cdebug << "  Checking token " << token << endl; )
		string defaultArgument = "";
		// kts now check for default argument
		if(token.find("=") != string::npos)
		{
//			defaultArgument = token(token.find("=")+1,token.length()-(token.find("=")+1));
			defaultArgument = token;
			assert(token.find("=") != string::npos);
			defaultArgument.erase(0,token.find("="));
			assert(defaultArgument[0] == '=');
			defaultArgument.erase(0,1);
            if(defaultArgument.length() == 0)
                defaultArgument = EMPTY_DEFAULT_ARGUMENT;
//			assert(defaultArgument.length());
			DBSTREAM2( cdebug << "    default argument <" << defaultArgument << "> detected" << endl; )
			token.erase(token.find("="));
		}

		if(find(parameters.begin(),parameters.end(),token) != parameters.end())
			cerror << "Error: Macro parameter " << token << " defined more than once in macro " << newName << endl;
		else
		{
			parameters.push_back(token);
			defaultArguments.push_back(defaultArgument);
			assert(parameters.size() == defaultArguments.size());
			DBSTREAM2( cdebug << "   Creating parameter named <" << token << ">" << endl; )
		}
	}
	DBSTREAM2( cdebug << "parameter count = " << parameters.size() << endl; )
}

//=============================================================================

macro&
macro::operator= (const macro& i)
{
	DBSTREAM2( cdebug << "macro class assignment operator: copying macro with name <" << i.name << "> and value of <" << i.value << "> " << endl; )
	name = i.name;
	value = i.value;
	parameters = i.parameters;
	defaultArguments = i.defaultArguments;
    whereDefined = i.whereDefined;
	return(*this);
}

//=============================================================================

macro::~macro()
{
}

//=============================================================================

ostream& 
operator<< (ostream& out, const macro& m)
{
    out << "name = " << m.name << ", value = " << m.value << endl;
    if(m.whereDefined.length())
       out << "defined at:" << m.whereDefined << endl;
    else
       out << "defined at:" << "Unknown" << endl;

    // << ", parameters = " << m.parameters << ", default args = " << m.defaultArguments;
    return out;    
}

//=============================================================================

const string&
macro::GetName()
{
	return(name);
}

//=============================================================================

const string&
macro::GetValue()
{
	return(value);
}

//=============================================================================

const string&
macro::GetWhereDefined()
{
	return(whereDefined);
}

//==============================================================================

string
macro::DoSubstitution(ktsRWCTokenizer& next,int& substCount)
{
	string outputLine;
	string token;
	string restOfString;
	string params;

	if(ParameterCount())
	{
		DBSTREAM2( cdebug << "Macro parameters detected, with a value of <" << GetValue() << "> " << endl; )
		DBSTREAM2( cdebug << "parameter count = " << parameters.size() << endl; )
		assert(parameters.size() == defaultArguments.size());

		restOfString = next.restOfString();
		if((restOfString.length() == 0) || restOfString[0] != '(')
			cerror << SourceError() << "Macro invocation missing parameters " << endl;
		next += 1;				// skip opening paren
		if(next.ReadChar() != ')')
			params = next.NextTrackParens(")");
		else
			params = "";						// this is in case () are empty
		if(next.ReadChar() != ')')
			cerror << SourceError() << "Macro invocation missing closing )" << endl;
//		if((params = next.NextTrackParens(")")).isNull())
//			cerror << SourceError() << "Macro invocation missing closing )" << endl;
		next+=1;				// skip )
		DBSTREAM2( cdebug << "paramaters exist: <" << params << ">" << endl; )
		// here we create a new dictionary of parameters
		ktsRWCTokenizer nextParam(params);
		map<string,string, less<string> > paramDict;
		for(unsigned int parameterIndex=0;parameterIndex<ParameterCount();)
		{
			DBSTREAM2( cdebug << "default argument for index <" << parameterIndex << "> is <" << DefaultArgument(parameterIndex) << ">" << endl; )

			if(((token = nextParam(",",whiteChars)).length() == 0) && (DefaultArgument(parameterIndex).length() == 0))
				cerror << SourceError() << "Incorrect # of parameters in macro invocation <"
				<< next.restOfString() << ">, expected " << ParameterCount()
				<< ", only found " << parameterIndex << endl;

			if(token.length() == 0)
			{
				token = DefaultArgument(parameterIndex);
				DBSTREAM2( cdebug << "Null parameter found, using default <" << token << "> with an index of <" << parameterIndex << ">" << endl; )
                // looped this way so that later paramters can refer to previous parameters
				for ( ;; )
				{
					assert( token.length() );
                    if(token == EMPTY_DEFAULT_ARGUMENT)
                        token = "";
//					int idxParameter = parameters.index( token );
//					if ( idxParameter == RW_NPOS )
//						break;

					vector<string>::iterator found = find(parameters.begin(),parameters.end(),token);
					if(found == parameters.end())
						break;
					unsigned int idxParameter = found - parameters.begin();
					DBSTREAM2( cdebug << "found parameter named " << token << " at idxParameter " << idxParameter << " which maps to " << Parameter(idxParameter) <<  ", index = " << parameterIndex << endl; )
					assert(idxParameter < parameterIndex);				// cannot refer to something we haven't parsed
					//assert( !DefaultArgument( idxParameter ).isNull() );
					//token = DefaultArgument( idxParameter );
					assert( paramDict[ Parameter( idxParameter ) ].length() != 0 );
					token = paramDict[ Parameter( idxParameter ) ];
				}
			}


            // check for named parameter
            unsigned delimiterIndex = token.find("=>");
            if(delimiterIndex != std::string::npos)
            {                                                    
                // named parameter found
                string name = token.substr(0,delimiterIndex);
                string value = token.substr(delimiterIndex+2);
                
                while(name.size() && name[name.size()-1] == ' ')
                    value.erase(name.size()-1,1);

                while(value.size() && value[0] == ' ')
                    value.erase(1,1);

                // now confirm that this name matches an existing parameter
                AssertMsg(ValidParameter(name),"There is no parameter named " << name);

                DBSTREAM2( cdebug << "Adding named parameter containing <" << value << "> matching <" << name << "> to parameter list for macro <" << GetName() << ">" << endl; )
                paramDict[name] = value;
            }
            else 
            {
                if(paramDict.find(Parameter(parameterIndex)) == paramDict.end())           // only set if not already set (to prevent defaults from overwritting named parameters)
                {
                    DBSTREAM2( cdebug << "Adding parameter containing <" << token << "> matching <" << Parameter(parameterIndex) << "> to parameter list for macro <" << GetName() << ">" << endl; )
                    // overwrite existing macro, or create a new one
                    paramDict[Parameter(parameterIndex)] = token;

                }
                parameterIndex++;
            }
		}
		// here we copy the line, replacing parameters as found
		ktsRWCTokenizer macroNext(GetValue().c_str());
		string macroToken;

		DBSTREAM2( cdebug << "macro value <" << macroNext.restOfString() << ">" << endl; )

		while(macroNext.ReadChar())					// until end of string
		{
			if(macroNext.MatchNextChar(delimiters))
			{
				string temp;
				temp = macroNext.GetWhiteSpace(delimiters);
				DBSTREAM2( cdebug << "Whitespace found <" << temp << ">" << endl; )
				DBSTREAM2( cdebug << "output line before <" << outputLine << ">" << endl; )
				outputLine += temp;
				DBSTREAM2( cdebug << "output line after <" << outputLine << ">" << endl; )
//				outputLine += macroNext.GetWhiteSpace(whiteChars);
			}
			macroToken = macroNext(delimiters,whiteChars);
			DBSTREAM2( cdebug << "Checking macro <" << GetName() << "> for parameter <" << macroToken << ">" << endl; )
			if(paramDict.find(macroToken) != paramDict.end() )
			{
				DBSTREAM2( cdebug << "Parameter <" << macroToken << "> found, resulting in <" << paramDict[macroToken] << ">" << endl; )
				outputLine += paramDict[macroToken];
			}
			else
				outputLine += macroToken;
		}
	}
	else								// if no parameters, just copy string
	{
		outputLine += GetValue();
	}
	substCount++;
	return(outputLine);
}

//=============================================================================
