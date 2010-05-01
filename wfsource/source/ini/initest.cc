// initest.cc

#include <pigsys/pigsys.hp>
#include <ini/profile.hp>

char szIniFile[] = "textile.ini";

void
PIGSMain( int argc, char* argv[] )
{
	char szConfig[] = "Configuration";

	int x_page = GetPrivateProfileInt( szConfig, "pagex", x_page, szIniFile );
	int y_page = GetPrivateProfileInt( szConfig, "pagey", y_page, szIniFile );
	int x_align = GetPrivateProfileInt( szConfig, "alignx", x_align, szIniFile );
	int y_align = GetPrivateProfileInt( szConfig, "aligny", y_align, szIniFile );
	bool bDebug = bool( GetPrivateProfileInt( szConfig, "debug", bDebug, szIniFile ) );
	bool bVerbose = bool( GetPrivateProfileInt( szConfig, "verbose", bVerbose, szIniFile ) );
	bool bFrame = bool( GetPrivateProfileInt( szConfig, "showframe", bFrame, szIniFile ) );
	bool bShowAlign = bool( GetPrivateProfileInt( szConfig, "showalign", bShowAlign, szIniFile ) );
	bool bShowPacking = bool( GetPrivateProfileInt( szConfig, "showpacking", bShowPacking, szIniFile ) );
	bool bCropOutputImage = !bool( GetPrivateProfileInt( szConfig, "crop", !bCropOutputImage, szIniFile ) );
	char szOutFile[ _MAX_PATH ];
	GetPrivateProfileString( szConfig, "out", szOutFile, szOutFile, sizeof( szOutFile ), szIniFile );
	int nRooms = GetPrivateProfileInt( "Rooms", "nRooms", nRooms, szIniFile );
	bool bPowerOf2Size = bool( GetPrivateProfileInt( szConfig, "powerof2size", bPowerOf2Size, szIniFile ) );
	bool bSourceControl = bool( GetPrivateProfileInt( szConfig, "sourcecontrol", bSourceControl, szIniFile ) );

	// Target system
	//ts = TargetSystem( GetPrivateProfileInt( szConfig, "TargetSystem", int( ts ), szIniFile ) );

	// Palette configuration
	char szPalette[] = "Palette";
	int pal_x_page = GetPrivateProfileInt( szPalette, "pagex", pal_x_page, szIniFile );
	int pal_y_page = GetPrivateProfileInt( szPalette, "pagey", pal_y_page, szIniFile );
	int pal_x_align = GetPrivateProfileInt( szPalette, "alignx", pal_x_align, szIniFile );
	int pal_y_align = GetPrivateProfileInt( szPalette, "aligny", pal_y_align, szIniFile );

	int rTransparent = 0;
	int gTransparent = 0;
	int bTransparent = 0;
	char szTransparent[ 128 ];
	sprintf( szTransparent, "%d,%d,%d", rTransparent, gTransparent, bTransparent );
	GetPrivateProfileString( szPalette, "transparent", szTransparent, szTransparent, sizeof( szTransparent ), szIniFile );
	sscanf( szTransparent, "%d,%d,%d", &rTransparent, &gTransparent, &bTransparent );
	// TODO: Add colour cycle file specification

	// Paths
	const char szPath[] = "Path";
	char szTexturePath[ _MAX_PATH ];
	GetPrivateProfileString( "Texture", (char*)szPath, szTexturePath, szTexturePath, sizeof( szTexturePath ), szIniFile );
	char szVrmlPath[ _MAX_PATH ];
	GetPrivateProfileString( "VRML", (char*)szPath, szVrmlPath, szVrmlPath, sizeof( szVrmlPath ), szIniFile );
}
