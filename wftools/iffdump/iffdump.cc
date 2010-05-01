//=============================================================================
// iffdump.cc: kts file iff file dumper
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

#include "global.hp"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <recolib/command.hp>
#include <recolib/hdump.hp>
#include <cpplib/stdstrm.hp>
#include <streams/binstrm.hp>
#include <hal/hal.h>
#include <iff/iffread.hp>
#include "iffdump.hp"

#include <map>
using namespace std;

#if DEBUG
#define DEBUG_STRING "debug"
#else
#define DEBUG_STRING "release"
#endif

#define VERSION_STRING "0.08"

//=============================================================================

ostream* output;
int charsPerLine=16;
bool dumpBinary=true;
bool useHDump=true;
string chunkFileName("chunks.txt");

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

unsigned int
ParseCommandLine(const CommandLine& cl)
{
    unsigned int commandIndex;
	for ( commandIndex=1; commandIndex < cl.Size() && (cl[commandIndex][0] == '-'); ++commandIndex )
	{
		string command = cl[commandIndex];
		assert(command[0] == '-');
		DBSTREAM2( cdebug << "command line option <" << command << "> Found" << endl; )
		string restOfCommand = string(command.substr(2,command.length()-2));
		switch(command[1])
		{
			case 'c':
			case 'C':
				chunkFileName = restOfCommand;
				break;
			case 'f':
			case 'F':
				useHDump = command[2] == '+'?true:false;
				break;
			case 'd':
			case 'D':
				dumpBinary = command[2] == '+'?true:false;
				break;
			case 'w':
			case 'W':
				assert(command[2] == '=');
				charsPerLine = atoi(restOfCommand.c_str());
				assert(charsPerLine > 0);
				break;
			case 'p':
			case 'P':
#if SW_DBSTREAM
				RedirectStandardStream(restOfCommand.c_str());
#else
				cerr << "stream redirection not supported in non-debug version" << endl;
#endif
				break;
			default:
				cerr << "iffdump Error: Unrecognized command line switch \"" <<
					command << "\"" << endl;
				break;
		 }
	 }

	if(cl.Size()-(commandIndex-1) < 2)
	{
		cout << "iffdump V" VERSION_STRING " " DEBUG_STRING " Copyright 1998,1999,2010 World Foundry Group." << endl;
		cout << "Usage: iffdump {-<switches>} <infile> {<outfile>}" << endl;
		cout << "Switches:" << endl;
		cout << "    -c<filename> override default chunk file" << endl;
		cout << "    -d<+|-> enable/disable dump binary" << endl;
		cout << "    -f<+|-> enable hdump vs iffcomp compatible hdump" << endl;
		cout << "    -w=<#> # of chars per line in binary dump" << endl;
		cout << "    -p<stream initial><stream output>, where:" << endl;
		cout << "        <stream initial> can be any of:" << endl;
		cout << "            w=warnings (defaults to standard err)" << endl;
		cout << "            e=errors (defaults to standard err)" << endl;
		cout << "            f=fatal (defaults to standard err)" << endl;
		cout << "            s=statistics (defaults to null)" << endl;
		cout << "            p=progress (defaults to null)" << endl;
		cout << "            d=debugging  (defaults to null)" << endl;
		cout << "        <stream output> can be any of:" << endl;
		cout << "            n=null, no output is produced" << endl;
		cout << "            s=standard out" << endl;
		cout << "            e=standard err" << endl;
		cout << "            f<filename>=output to filename" << endl;
		exit(1);
	}
	return(commandIndex);
}

//=============================================================================

// load chunk list
class ChunkList
{
	vector<ChunkID> chunks;
public:
	ChunkList() {}
	ChunkList(istream& in) { Load(in); }
	~ChunkList() {}
	void Load(istream& in);
  void Dump();

	int Lookup(ChunkID id);
};


void
ChunkList::Dump()
{
  cout << "ChunkList::Dump:";
  // SMELL: use iterator
  for(unsigned int index=0;index<chunks.size();index++)
  {
    cout << ' ' << chunks[index];
  }
  cout << endl;
}


void
ChunkList::Load(istream& in)
{
	DBSTREAM3( cdebug << "ChunkList::Load: " << endl; )
	char stringBuffer[100];

	while(in.good())
	 {
		stringBuffer[0] = 0;
		stringBuffer[1] = 0;
		stringBuffer[2] = 0;
		stringBuffer[3] = 0;
		in.getline(stringBuffer,100);

		if(strlen(stringBuffer) != 0 && stringBuffer[strlen(stringBuffer)-1] == '\n')
			stringBuffer[strlen(stringBuffer)-1] = 0;
		if(strlen(stringBuffer) != 0 && (stringBuffer[0] != '/' && stringBuffer[1] != '/'))
		 {
			// only read until whitespace
			stringBuffer[ strcspn(stringBuffer, " \t") ] = 0;
			DBSTREAM3( cdebug << "ChunkList::Load: parsing line <" << stringBuffer << ">" << endl; )

			assert(strlen(stringBuffer) <= 4);
			int32 id = *(int32*)(&stringBuffer[0]);
			ChunkID cid(id);
			chunks.push_back(cid);
		 }
	 }
}


int
ChunkList::Lookup(ChunkID id)
{
	for(unsigned int index=0;index<chunks.size();index++)			// kts does STL have a better way to do this?
	 {
		DBSTREAM3( cdebug << "ChunkList::Lookup: checking index " << index << " which contains " << chunks[index] << endl; )
		if(chunks[index] == id)
		 {
			DBSTREAM3( cdebug << "found, index = " << index << endl; )
			return(index);
		 }
	 }
//	AssertMessageBox(0,"ChunkList::Lookup: failed, looked for extension " << extension );
	return(-1);
}

ChunkList* chunkList;

//=============================================================================

ostream&
IndentF(ostream& out,int indent)
{
	while(indent--)
		out << '\t';
	return out;
}

class Indent
{
public:
	Indent(int indent) { _indent = indent; }
	friend ostream& operator<<(ostream& out,Indent in) { return IndentF(out,in._indent); }
	void operator++() { _indent++;}
	void operator--() { _indent--; assert(_indent >= 0); }
	void operator++(int) { _indent++;}
	void operator--(int) { _indent--; assert(_indent >= 0); }
	int Value() { return _indent; }
private:
	int _indent;
};

//=============================================================================

bool
IsWrapperChunk(IFFChunkIter& iter)
{
	assert(ValidPtr(chunkList));
	int index = chunkList->Lookup(iter.GetChunkID());
	return index != -1;	// found?
}

//=============================================================================

void
DumpBinary(ostream& out, IFFChunkIter& topIter,Indent& ind)
{
	out << ind << "{ '" << topIter.GetChunkID() << "'		// Size = " << topIter.Size() << endl;
	ind++;

	assert(topIter.Size() == topIter.BytesLeft());
	char* data = new char[topIter.Size()];
	assert(ValidPtr(data));
	topIter.ReadBytes(data,topIter.Size());

	out << ind << "// unknown chunk" << endl;
	if(dumpBinary)
	{
		if(useHDump && topIter.Size())
			HDump(data, topIter.Size(), ind.Value(), "\t", out,charsPerLine);
		else
		{
			int lineCounter=0;
			out << setfill('0') << hex;
			for ( int i=0; i<topIter.Size(); ++i )
			{
				if(!(lineCounter % 64))
					out << endl << ind;

				unsigned char value = data[ i ];
				out << "$" << setw(2) << ((unsigned int)value) << " ";

				lineCounter++;
			}
			out << endl;
		}
	}
	ind--;
	out << ind << "}" << endl;
	delete [] data;
}

//=============================================================================

void
DumpIFF(ostream& out, IFFChunkIter& topIter,Indent& ind)
{
//	cout << "topIter = " << hex << topIter.GetChunkID().ID() << "," << topIter.GetChunkID() << endl;
	out << ind << "{ '" << topIter.GetChunkID() << "'		// Size = " << topIter.Size() << endl;
	ind++;
	while(topIter.BytesLeft())
	{
		IFFChunkIter* chunkIter = topIter.GetChunkIter(HALLmalloc);
//		cout << "chunkIter = " << hex << chunkIter->GetChunkID().ID() << "," << chunkIter->GetChunkID() << endl;

		if(IsWrapperChunk(*chunkIter))
			DumpIFF(out,*chunkIter,ind);
		else
			DumpBinary(out,*chunkIter,ind);

		delete chunkIter;
	}
	ind--;
	out << ind << "}" << endl;
}

void
DumpIFF(ostream& out, binistream& input,Indent& ind)
{
	while(!input.eof())
	{
		IFFChunkIter topIter(input);
		if(IsWrapperChunk(topIter))
			DumpIFF(out,topIter,ind);
		else
			DumpBinary(out,topIter,ind);
	}
}

//=============================================================================

int
main(int argc, char** argv)
{
	CommandLine cl(argc,argv);
	output = &cout;

	unsigned int commandIndex = ParseCommandLine(cl);
//	cout << "commandIndex = " << commandIndex << endl;

	cdebug << "iffdump Debugging stream active!" << endl;

#pragma message(__FILE__ ": kts: temporary, make a binistream that reads from disk")

	binistream input(cl[commandIndex].c_str());
	assert(input.good());
	cstats << "Reading from source file " << cl[commandIndex] << endl;

	ifstream chunkNames(chunkFileName.c_str());
	chunkList = new ChunkList(chunkNames);
	//chunkList->Dump();

	if(cl.Size() > 1+commandIndex)
	{
		output = new ofstream(cl[1+commandIndex].c_str());
		if(output->rdstate() != ios::goodbit)
	  	{
			cerror << "Unable to open output file " << cl[1+commandIndex] << endl;
			return 5;
	  	}
		cstats << "Writing to dest file " << cl[1+commandIndex] << endl;
	}
	ostream& out = *output;

	out << "//=============================================================================" << endl;
	out << "// ";
	if(cl.Size() > 1+commandIndex)
		out << cl[1+commandIndex];
	out << " Created by iffdump v" VERSION_STRING << endl;
	out << "//=============================================================================" << endl;

    Indent indent(0);
	DumpIFF(out, input,indent);

	// SMELL: should be more self-contained, smart, magical
	if ( output != &cout ) {
	  delete output;
	}
	delete chunkList;
	return 0;
}

//=============================================================================
