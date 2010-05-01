#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>

//#include "pxp.h"
//#include "dialog.h"

#include "oas/pigtool.h"
#include "oas/oad.h"

/* A chunk header */
typedef struct
	{
	unsigned short int id;
	unsigned long length;
	} Header;


typedef struct
	{
	ulong version;
	} State;


// Application types

//#define FALSE 0
//#define TRUE 1
//typedef short bool;
// MSVC 5.0
//typedef enum { false, true } bool;
	//#undef FALSE
	//#define FALSE	false
	//#undef TRUE
	//#define TRUE	true

typedef unsigned char byte;
//typedef char byte;

//typedef char int8;
//typedef short int16;
typedef short fixed16;
typedef long fixed32;
//typedef short boolean;


//typedef char Filename[ 8+3+1+1 ];

typedef struct
	{
	char oadFilename[ _MAX_PATH ];
	} ClassChunk;

typedef ClassChunk MeshNameChunk;

class uiDialog;

typedef struct
	{
//	int offset;
	void* data;
	uiDialog* theGadget;
	} DialogGlue;


#endif
