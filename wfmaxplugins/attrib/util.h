#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include "types.h"

void* LoadBinaryFile( const char* _szFilename, int& nSize );
char* LoadTextFile( const char* _szFilename, int& nSize );
char* strchrs( const char* s, const char* charset );
long ffilesize( FILE *fp );
bool fileExists( const char* );
//const char* GetOadDir();

// in pigsys.h
//#define until( _cond_ )		while ( !( _cond_ ) )

//#define	MAX(_1,_2)				((_1)>(_2)?(_1):(_2))
//#define	MIN(_1,_2)				((_1)>(_2)?(_2):(_1))

//ItemData* objectExists( const char* name );

#endif
