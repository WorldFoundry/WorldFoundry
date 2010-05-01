//=============================================================================
// command.cc: command line handling and parsing
// Copyright (c) 1996-1999, World Foundry Group  
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

#include <pigsys/assert.hp>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#include "command.hp"
#include <recolib/infile.hp>
#include <recolib/ktstoken.hp>

//=============================================================================
// need to add wildcarding and reading commands from environemt variable

CommandLine::CommandLine(int argc, char** argv)
{
	int index=0;
	while(argc--)
	{
		if(argv[index][0] == '@')
		{
			string fileName(&argv[index][1]);
			inputFile file(fileName);
			assert(file.good());
			while(file.good())
			{
				// read commands from file
				string line = file.ReadLine();
				// now disect string into each command

				ktsRWCTokenizer next(line);
				while(next.restOfString().length() != 0)
				{
					string cmd = next(" "," ");
					if(cmd.length())
						commands.push_back(cmd);
				}
			}
		}
		else
		{
			commands.push_back(string(argv[index]));
		}
		index++;
	}
}

//=============================================================================
