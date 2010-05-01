//=============================================================================
// command.cc: command line handling and parsing
//=============================================================================

#include "global.hp"
#include <fstream>
#include "command.hp"
#include "infile.hp"
#include "ktstoken.hp"

//=============================================================================
// need to add wildcarding and reading commands from environemt variable

CommandLine::CommandLine(int argc, char** argv)
{
	int index=0;
	while(argc--)
	{
		if(argv[index][0] == '@')
		{
			std::string fileName(&argv[index][1]);
			inputFile file(fileName);
			assert(file.good());
			while(file.good())
			{
				// read commands from file
				std::string line = file.ReadLine();
				// now disect string into each command

				ktsRWCTokenizer next(line);
				while(next.restOfString().length() != 0)
				{
					std::string cmd = next(" "," ");
					if(cmd.length())
						commands.push_back(cmd);
				}
			}
		}
		else
		{
			commands.push_back(std::string(argv[index]));
		}
		index++;
	}
}

//=============================================================================
