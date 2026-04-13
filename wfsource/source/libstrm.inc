//=============================================================================
// wfsource/source/libstrm.inc
//=============================================================================
// X-macro table of debug/library streams, included via `#include "../libstrm.inc"`
// from cpplib/libstrm.hp (declares extern) and cpplib/libstrm.cc (defines).
//
// Each per-library subdir (anim/, asset/, iff/, memory/, ...) keeps its own
// libstrm.inc with just its one stream, but that is not what
// cpplib/libstrm.hp ends up picking up: the quoted `../libstrm.inc` resolves
// relative to cpplib/libstrm.hp's directory (wfsource/source/cpplib/), i.e.
// to *this* file regardless of which subdir is building. So this is the
// union that every build sees.
//
// Format: STREAMENTRY(streamname, default_assignment, stream_letter, helptext)
//=============================================================================

	STREAMENTRY(canim,       NULL_STREAM, 'a', "animation/path system" )
	STREAMENTRY(casset,      NULL_STREAM, 'A', "Asset ID system" )
	STREAMENTRY(cgfx,        NULL_STREAM, 'g', "graphics system" )
	STREAMENTRY(ciff,        NULL_STREAM, 'i', "iff reader library" )
	STREAMENTRY(cmailbox,    NULL_STREAM, 'I', "mailbox system" )
	STREAMENTRY(cmem,        NULL_STREAM, 'm', "memory system" )
	STREAMENTRY(cmovement,   NULL_STREAM, 'M', "movement handlers" )
	STREAMENTRY(ccollision,  NULL_STREAM, 'c', "collision system" )
	STREAMENTRY(croom,       NULL_STREAM, 'r', "room system" )
	STREAMENTRY(croomslots,  NULL_STREAM, 'R', "room slot assets" )
	STREAMENTRY(cscripting,  NULL_STREAM, 's', "scripting languages" )

//=============================================================================
