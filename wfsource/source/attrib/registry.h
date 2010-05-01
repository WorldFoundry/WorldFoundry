bool GetLocalMachineStringRegistryEntry( const char* path, const char* valueName, char* contents, int sizeOfContents );
bool SetLocalMachineStringRegistryEntry( const char* path, const char* valueName, char* contents );	//, int sizeOfContents );
bool SetLocalMachineStringRegistryEntry_NoCreate( const char* path, const char* valueName, char* contents );	//, int sizeOfContents );
