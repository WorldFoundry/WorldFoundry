//=============================================================================
// infile.cc:
// By Kevin T. Seghetti
// Copyright (c) 1995,96,97,98 Kevin T. Seghetti All Rights Reserved
//=============================================================================

#include "global.hp"
#include "infile.hp"

#include <cpplib/stdstrm.hp>

//=============================================================================

inputFile::inputFile(const std::string& newFileName) :
	instream(newFileName.c_str()),
	fileName(newFileName)
{
	lineNum = 0;
//	cerror << "infile: good = " << good() << newFileName << endl;
	if(!instream.good())
	 {
		std::cerr << "Error: Source file not found <" << fileName << ">" << std::endl;
		exit(5);
	 }
}

//=============================================================================

inputFile::~inputFile()
{
}

//=============================================================================

std::string
inputFile::ReadLine()
{
	char inputBuffer[1000];

	instream.getline(inputBuffer,1000);
	assert(!instream.bad());
	std::string line(inputBuffer);
	lineNum++;
	return(line);
}

//=============================================================================


bool
inputFile::good()
{
	return instream.good();
}

//==============================================================================


const std::string&
inputFile::Name()
{
	return(fileName);
}

//=============================================================================

long
inputFile::LineNum()					// returns current line #
{
	return(lineNum);
}

//=============================================================================
