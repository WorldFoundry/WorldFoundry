// handle.cc

#include <gfx/handle.hp>

// ------------------------------------------------------------------------
// class HandleID
// ------------------------------------------------------------------------

#if DO_IOSTREAMS

binistream&
operator >> ( binistream& binis, HandleID& x )
{
	return binis >> x._id;
}

#endif

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const HandleID& x )
{
	return binos << x._id;
}

#endif

// ------------------------------------------------------------------------

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& os, const HandleID& x )
{
	const char * pbID = (const char *)&x._id;
	return os
		<< *( pbID )
		<< *( pbID + 1 )
		<< *( pbID + 2 )
		<< *( pbID + 3 );
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------
// class Handle
// ------------------------------------------------------------------------

Handle::Handle()
{
	_id = HandleID();
	_vertexNum = 0;
}


Handle::Handle( HandleID thisID, int32 vertexNum )
	: _id(thisID), _vertexNum(vertexNum)
{
}

// ------------------------------------------------------------------------

