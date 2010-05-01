#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>

#include "source/oas/pigtool.h"
#include "source/oas/oad.h"

// A chunk header
typedef struct
{
	unsigned short int id;
	unsigned long length;
} Header;


typedef struct
{
	ulong version;
} State;


typedef unsigned char byte;
typedef short fixed16;
typedef long fixed32;

typedef struct
{
	char oadFilename[ _MAX_PATH ];
} ClassChunk;

typedef ClassChunk MeshNameChunk;

class uiDialog;

typedef struct
{
	void* data;
	uiDialog* theGadget;
} DialogGlue;

#endif
