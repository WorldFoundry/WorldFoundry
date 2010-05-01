#include <max.h>
#include <string.h>
#include "../lib/registry.h"
#include "../lib/wf_id.hp"

const char*
GetLevelDir()
{
	Interface* ip = GetCOREInterface();
	assert( ip );

	// Level
	TSTR szProjectFilename = ip->GetCurFileName();
	TSTR szProjectPath = ip->GetCurFilePath();

	if ( !*szProjectFilename )
	{	// Error -- no level to run
		return NULL;
	}

	static char szLevelDir[ _MAX_PATH ];
	strcpy( szLevelDir, szProjectPath );
	*( szLevelDir + strlen( szLevelDir ) - strlen( szProjectFilename ) - 1 ) = '\0';

	const char* szSeparator = strrchr( szLevelDir, '\\' );
	if ( !szSeparator )
		return "\\";

	return szSeparator + 1;
}


const char* 
CreateLevelDirName()
{
	static char szLevelDirName[ _MAX_PATH ];
	*szLevelDirName = '\0';

	char szLevelsDir[ _MAX_PATH ];
	bool bSuccess = GetLocalMachineStringRegistryEntry( 
		szRegWorldFoundryGDK, "LEVELS_DIR", 
		szLevelsDir, sizeof( szLevelsDir ) );
	if(!bSuccess)
		MessageBox(GetCOREInterface()->GetMAXHWnd(), "Failed to find LEVELS_DIR in registry", "Attrib Error", MB_OK);
	//assert( bSuccess );

	const char* szLevelDir = GetLevelDir();
	if ( !szLevelDir )
		return NULL;

	sprintf( szLevelDirName, "%s/%s/", szLevelsDir, szLevelDir );

	return szLevelDirName;
}

