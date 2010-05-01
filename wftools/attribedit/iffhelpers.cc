

#include "iffhelpers.hp"

//==============================================================================
// iff read helper function to read an entire chunk into a string
// move these into iffread at some point

std::string
IFFReadString(IFFChunkIter& iter)
{
    char buffer[1024];
    int len = iter.BytesLeft();
    RangeCheck(0,len,1024);
    iter.ReadBytes(buffer, len);
    buffer[len] = 0;
    std::string retVal(buffer);
    return retVal;
}

int32
IFFReadInt32(IFFChunkIter& iter)
{
    int32 retVal;
    iter.ReadBytes(&retVal,sizeof(int32));
    return retVal;
}

Scalar
IFFReadFixed32(IFFChunkIter& iter)
{
    int32 temp;
    iter.ReadBytes(&temp,sizeof(int32));
    Scalar retVal(temp);
    return retVal;
}

bool
IFFReadBool(IFFChunkIter& iter)
{
    int8 retVal;
    iter.ReadBytes(&retVal,sizeof(int8));
    return retVal;
}

