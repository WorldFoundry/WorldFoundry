//=============================================================================
// registry.cc
//=============================================================================

#include <registry/registry.hp>
#include <windows.h>
#include <pigsys/assert.hp>
#include <cstring>

//=============================================================================

bool
GetLocalMachineStringRegistryEntry( const char* path, const char* valueName, char* contents, int sizeOfContents )
{
	HKEY resultKey = NULL;
    PHKEY phkResult = &resultKey; 	// address of handle of open key

//	KEY_EXECUTE

	long retVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE,	path, 0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, phkResult );
	//cout << "Registry read, retVal = " << retVal << ", key = " << resultKey << endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	assert( phkResult );
	assert( resultKey );

	DWORD type;
	DWORD cbData = sizeOfContents;

	retVal = RegQueryValueEx( resultKey, valueName, NULL, &type, (BYTE*)contents, &cbData );
	//cout << "retVal = " << retVal << endl;
	//cout << "type = " << type << endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( type == REG_SZ );
	assert( retVal == ERROR_SUCCESS );

	return true;
}


bool
SetLocalMachineStringRegistryEntry_NoCreate( const char* path, const char* valueName, char* contents )	//, int sizeOfContents )
{
	HKEY resultKey = NULL;
    PHKEY phkResult = &resultKey; 	// address of handle of open key

	long retVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE,	path, 0,
		KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, phkResult );
	//cout << "Registry read, retVal = " << retVal << ", key = " << resultKey << endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	assert( phkResult );
	assert( resultKey );

//	DWORD cbData = sizeOfContents;
	DWORD cbData = strlen( contents );

	retVal = RegSetValueEx( resultKey, valueName, NULL, REG_SZ, (BYTE*)contents, cbData );
	//cout << "retVal = " << retVal << endl;
	//cout << "type = " << type << endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	return true;
}


bool
SetLocalMachineStringRegistryEntry( const char* path, const char* valueName, char* contents )	//, int sizeOfContents )
{
	HKEY resultKey = NULL;
    PHKEY phkResult = &resultKey; 	// address of handle of open key

	ULONG disposition;
	long retVal = RegCreateKeyEx( HKEY_LOCAL_MACHINE, path, 0,
		"", REG_OPTION_NON_VOLATILE,
		KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE,
		NULL,	// security attributes
		phkResult,
		&disposition
		);
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	assert( phkResult );
	assert( resultKey );

	DWORD cbData = strlen( contents );

	retVal = RegSetValueEx( resultKey, valueName, NULL, REG_SZ, (BYTE*)contents, cbData );
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	return true;
}


#if 0
void
main()
{
	BYTE data[200];
	GetLocalMachineStringRegistryEntry("Software\\World Foundry Group\\World Foundry\\GDK\\1.0","OAD_DIR",data,200);
	cout << "data = " << data << endl;
}
#endif

//=============================================================================
