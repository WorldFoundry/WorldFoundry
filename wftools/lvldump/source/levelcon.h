/*============================================================================*/
/* levelcon.h:  */
/* Copyright(c) 1995,96,97 Recombinant Limited */
/*============================================================================*/
/* Documentation:

	Abstract:
			C header file containing the disk structures created by levelcon
			This file is include by both velocity and by levelcon
	History:
			Created	05-05-95 10:54am Kevin T. Seghetti
			Added cameras, paths, & lights 06-07-95 05:25pm Kevin T. Seghetti
			Added collision rect 06-20-95 02:56pm KTS
			Added optimization of COMMONBLOCK data	9/29/95 Phil Torre
	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:
		Be sure to update the LEVEL_VERSION if you change any structures
	Example:

	Level on disk format:

	struct _LevelOnDisk			which contains offsets to the rest
*/
/*============================================================================*/
/* use only once insurance */
#ifndef _lEVELCON_H
#define _lEVELCON_H

#if defined( __WIN__ )
#	pragma pack(4)
#endif

/* if any changes are made to this file, the version # should be updated
   so the game will detect any invalid levels */
enum { LEVEL_VERSION = 28 };
/* -----------------list of changes to make if version # is bumped-------
 *
 * --------------------------------------------------*/


#define PATH_NULL -1				/* index of invalid path */
#define CHANNEL_NULL -1				/* index of invalid channel */
#define OBJECT_NULL -1				/* index of invalid object reference */
#define MAX_ADJACENT_ROOMS 	2		/* maximum # of rooms which can be next to one room */
#define ADJACENT_ROOM_NULL -1		/* index of invalid adjacent room */

enum	// These go in the lightType field of a Light OAD
{
	AMBIENT_LIGHT=0,
	DIRECTIONAL_LIGHT
};

/*============================================================================*/
/* kts for now, simple box */

struct _CollisionRectOnDisk
{
	fixed32 minX,minY,minZ;
	fixed32 maxX,maxY,maxZ;
};

/*============================================================================*/

typedef struct wf_euler
{
	uint16 a;
	uint16 b;
	uint16 c;
	uint8 order;
} wf_euler;

/*============================================================================*/
/* on disk representation of a level object */

struct _ObjectOnDisk
{
	int16 type;
	fixed32 x,y,z;
	fixed32 x_scale,y_scale,z_scale;
	wf_euler  rotation;
	struct _CollisionRectOnDisk coarse;
	int32 oadFlags;					/* bit field containing all of the oad flags */
	int16 pathIndex;				/* = PATH_NULL if no path */
	int16 OADSize;					/* size of following oad data */
/*	Object Attribute Descriptor (OAD) data (springiness, etc) follows */
};

/*============================================================================*/
/* on disk representation of a room */

struct _RoomOnDiskEntry
{
	int16 object;					// index (in master object list) of object in this room
};

struct _RoomOnDisk
{
	int32 count;					// # of objects in this room
	struct _CollisionRectOnDisk boundingBox;
	int16 adjacentRooms[MAX_ADJACENT_ROOMS];
	int16 roomObjectIndex;						// kts added 10-01-96 01:28pm

	// variable sized array of _RoomOnDiskEntry entries follow
//	struct _RoomOnDiskEntry entries[0];		// first entry in array
//	.
//	.
//	.
};

/*============================================================================*/
/* on disk representation of a level path */

// All of this commented out stuff is the old path code...
//struct _PathOnDiskEntry
//{
//	fixed32 x,y,z;	// position
//	wf_euler rot;		// rotation (as Euler XYZS)
//	int32 frame;
//};

//struct _PathOnDisk
//{
//	int32 count;									// # of key frames follow
//	struct { fixed32 x,y,z; wf_euler rot; } base;	// base for (relative) path
//	// variable sized array of _PathOnDiskEntry entries follow
////	struct _PathOnDiskEntry entries[0];	// first entry in array
////	.
////	.
////	.
//};

// This is the new path code (using Channels in the LVL file)
struct _PathOnDisk
{
	struct pathBase { fixed32 x,y,z; wf_euler rot; } base;	// base for (relative) path
	int32 PositionXChannel;		// index into the LVL file's channel list (-1 == no channel)
	int32 PositionYChannel;
	int32 PositionZChannel;
	int32 RotationAChannel;
	int32 RotationBChannel;
	int32 RotationCChannel;
};

/*============================================================================*/
/* on disk representation of a Channel object */

enum	/* channel compressor/decompressor types */
{
	LINEAR_COMPRESSION = 0,		// Just stored keys with linear interpolation
	CONSTANT_COMPRESSION,		// A single constant int32 for all time values
	RLE_COMPRESSION             // (Is this the same as just stored keys? Hmm...)
};

struct _ChannelOnDiskEntry
{
	fixed32 time;
	int32 value;
};

struct _ChannelOnDisk
{
	int32 compressorType;		// Choose which decompressor to use
	fixed32 endTime;			// time of last key in channel (start time is always zero)
	int32 numKeys;				// how many keys this channel contains
	// variable sized array of _ChannelOnDiskEntry structs follow...
	// (In the case of CONSTANT_COMPRESSION, a single key follows, and it's Time component is
	// meaningless.)
};

/*============================================================================*/
/* level format on disk */

struct _LevelOnDisk
{
	int32 versionNum;

	int32 objectCount;
	int32 objectsOffset;			// offset to array of object offsets

	int32 pathCount;
	int32 pathsOffset;				// offset to array of path offsets

	int32 channelCount;
	int32 channelsOffset;			// offset to array of channel offsets

	int32 lightCount;
	int32 lightsOffset;				// offset to array of lights

	int32 roomCount;
	int32 roomsOffset;				// offset to array of room offsets

	int32 commonDataLength;			// size of common data area
	int32 commonDataOffset;			// offset to common data area
};

/*============================================================================*/

#if defined( __DOS__ ) || defined( __WIN__ )
#	pragma pack()
#endif

/*============================================================================*/
#endif
/*============================================================================*/
