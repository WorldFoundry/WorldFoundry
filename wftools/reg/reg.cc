// reg.cc

#include <iostream>
using namespace std;
#include <windows.h>

#include <registry/registry.hp>

void
usage( int argc, char* argv[] )
{
	cout << "reg  Registry reader/writer" << endl;
	cout << "Copyright 1998 Recombinant Limited.  All Rights Reserved." << endl;
	cout << "by William B. Norris IV" << endl;
	cout << endl;
	cout << "Usage: reg [-v] RegistryPath [Value]" << endl;
}


bool bVerbose = false;
char* szInputPath = NULL;
char* szInputKey = NULL;

int
main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		usage( argc, argv );
		return 20;
	}

	for ( ++argv; *argv; ++argv )
	{
		cout << *argv << endl;
		if ( strcmp( *argv, "-?" ) == 0 )
		{
			usage( argc, argv );
			return 20;
		}
		else if ( strcmp( *argv, "-v" ) == 0 )
			bVerbose = true;
		else
		{
			if ( !szInputPath )
				szInputPath = *argv;
			else
			{
				if ( szInputKey )
				{
					cerr << "error [szInputKey already defined]" << endl;
					return 20;
				}
				szInputKey = *argv;
			}
		}
	}

	if ( !szInputPath )
	{
		usage( argc, argv );
		return 20;
	}

	char szResult[ 512 ];
	bool success;

	char* szKeySeparator = strrchr( szInputPath, '\\' );
	if ( !szKeySeparator )
	{
		cerr << "error szKeySeparator" << endl;
		return 10;
	}

	char* szPath = szInputPath;
	*szKeySeparator = '\0';
	char* szKey = szKeySeparator + 1;

	if ( szInputKey )
	{
		if ( *szInputKey )
		{	// write
			success = SetLocalMachineStringRegistryEntry( szPath, szKey, szInputKey );
			if ( bVerbose )
			{
				cout << "Setting HKEY_LOCAL_MACHINE\\" << szPath << '\\' << szKey << " = " << szInputKey << endl;
			}
		}
		else
		{	// delete
			cout << "Delete not functional" << endl;
			//RegDeleteValue( HKEY_LOCAL_MACHINE, szInputPath );
		}
	}
	else
	{	// read
		success = GetLocalMachineStringRegistryEntry( szPath, szKey, szResult, sizeof( szResult ) );

		if ( bVerbose )
			cout << "HKEY_LOCAL_MACHINE\\" << szPath << '\\' << szKey << " => ";
		cout << szResult << endl;
	}

	return 0;
}
