//============================================================================
// hdump.cc: formated hex and ascii data dumper
// Copyright(c) 1994/5 Cave Logic Studios
// By Kevin T. Seghetti
// Copyright (c) 1994-1999, World Foundry Group  
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
//============================================================================

//#include <iostream.h>
//#include <iomanip.h>
#include "hdump.hp"

//============================================================================

char
charTable[256] =
{
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
	'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
	'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
	'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.'
};

//============================================================================

ulong
HDumpLine(const char* buffer, ulong bufferSize,ostream& out)
{
	ulong i = bufferSize;
	unsigned char* dataBuffer = (unsigned char *)buffer;
	unsigned char ch;
	unsigned int value;
	uint count = 0;
	while(i--)					// first print hex #'s
	 {
		out.flags(out.flags()|ios::uppercase);
		value = *dataBuffer++;

		out << setfill('0') << setw(2) << hex << value;
		if(!((count+1) % 4))
			out << ' ';
		count++;
	 }
	out << "    ";

	i = bufferSize;
	dataBuffer = (unsigned char *)buffer;
	while(i--)					// now print ascii
	 {
		ch = *dataBuffer++;
		assert(charTable[ch] != 0);
		out << charTable[ch];
	 }
	out << endl;
 	return(bufferSize);
}

//============================================================================

void
HDump(const void* buffer, ulong bufferSize, int indent, char* indentString, ostream& out)
{
	if (buffer && bufferSize)
	{
		ulong len;
		ulong offset = 0;
		int temp;
		char* dataBuffer = (char*)buffer;

	//	printf("buffersize %lu\n", bufferSize);
		while(bufferSize)
	 	{
			temp = 0;
			while(temp++ < indent)
				out << indentString;
			out << hex << setw(4) << setfill('0') << offset << ": ";
			len = HDumpLine(dataBuffer,bufferSize >= 16?16:bufferSize,out);
			dataBuffer += len;
			offset += len;
			bufferSize -= len;
	 	}
		out << dec;
	}
	else
		out << "HDump: Nothing to dump! (buffer = " << hex << buffer << dec
			<< ", bufferSize = " << bufferSize << ")" << endl;
}

//============================================================================
