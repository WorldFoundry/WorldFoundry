//============================================================================
// hdump.cc: formated hex and ascii data dumper
// Copyright(c) 1994/5 Cave Logic Studios
// By Kevin T. Seghetti
//============================================================================

#include <iostream>
#include <iomanip>
using namespace std;
#include <pigsys/pigsys.hp>
#include <pigsys/assert.hp>
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
HDumpLine(char* buffer, ulong bufferSize,ostream& out)
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
HDump(void* buffer, ulong bufferSize, int indent, char* indentString, ostream& out,unsigned int charsPerLine)
{
	assert(ValidPtr(buffer));
	assert(bufferSize > 0);
	assert(indent >= 0);
	assert(ValidPtr(indentString));
	assert(out.good());
	assert(charsPerLine > 0);
	ulong len;
	ulong offset = 0;
	int temp;
	char* dataBuffer = (char*)buffer;
	assert(buffer);

//	printf("buffersize %lu\n", bufferSize);
	while(bufferSize)
	 {
		temp = 0;
		while(temp++ < indent)
			out << indentString;
		out << hex << setw(4) << setfill('0') << offset << ": ";
		len = HDumpLine(dataBuffer,bufferSize >= charsPerLine?charsPerLine:bufferSize,out);
		dataBuffer += len;
		offset += len;
		bufferSize -= len;
	 }
	out << dec;
}

//============================================================================
