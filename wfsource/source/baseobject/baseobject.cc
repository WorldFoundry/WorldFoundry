//==============================================================================
// baseobject.cc:
// Copyright ( c ) 2003 World Foundry Group.  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

//==============================================================================
// Description: all game objects need to be derived from this object
// since it is what the object list code refers to, as well as the asset handling code
//==============================================================================

#include "baseobject.hp"

//==============================================================================
                                      
BaseObject::BaseObject(const void* oadData, const CommonBlock& commonBlock) :
   _oadData(oadData),
   _commonBlock(commonBlock)
{
}

//==============================================================================

BaseObject::~BaseObject()
{
}

//==============================================================================
    
#if SW_DBSTREAM >= 1

std::ostream&
operator << ( std::ostream& s, const BaseObject& obj )
{
	s << "BaseObject:" << std::endl;
   obj.Print(s);
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================
                      
bool
BaseObject::sendMsg(const int16 msgType, const int32 msgData)
{
	bool bSuccess = GetMsgPort().PutMsg( msgType, msgData );
	assert( bSuccess );
	//DBSTREAM3( cactor << "Actor #" << _idxActor << " sent message: " << (MsgPort::MSG_TYPE)msgType << " with data: " << std::hex << msgData << std::dec << std::endl; )
	return bSuccess;
}
//==============================================================================

bool
BaseObject::sendMsg(const int16 msgType, const void* msgData, uint32 msgDataSize)
{
   assert(ValidPtr(msgData));
	bool bSuccess = GetMsgPort().PutMsg( msgType, msgData, msgDataSize );
	assert( bSuccess );
	//DBSTREAM3( cactor << "Actor #" << _idxActor << " sent message: " << (MsgPort::MSG_TYPE)msgType << " with data: " << std::hex << msgData << std::dec << std::endl; )
	return bSuccess;
}

//==============================================================================

void 
BaseObject::BindAssets(Memory& /*memory*/)
{

}

//==============================================================================

void 
BaseObject::UnBindAssets()
{

}

//==============================================================================

void 
BaseObjectIterator::Validate() const
{
#if DO_VALIDATION
   _Validate();
#endif   
}

//==============================================================================

BaseObjectIterator& 
BaseObjectIterator::operator+=(int offset)              // will do it the slow way by default
{
   while(offset--)
      ++(*this);
   return *this;
}

//==============================================================================
//==============================================================================

BaseObjectIteratorFromInt16List::BaseObjectIteratorFromInt16List(const Int16ListIter& listIter, Array<BaseObject*>& objects) 
: _listIter(listIter), _objects(objects)
{

}

//==============================================================================

BaseObjectIteratorFromInt16List::~BaseObjectIteratorFromInt16List()
{

}

//==============================================================================

BaseObject& 
BaseObjectIteratorFromInt16List::operator*()
{
   Validate();
   _objects.Validate();
   _listIter.Validate();
   int actorIndex = *_listIter;
   BaseObject* bo = _objects[actorIndex];
   assert(ValidPtr(bo));
   return *bo;
}

//==============================================================================

BaseObjectIterator&
BaseObjectIteratorFromInt16List::operator++()
{
   ++_listIter;
   return *this;
}

//==============================================================================

bool 
BaseObjectIteratorFromInt16List::Empty() const
{
   return _listIter.Empty();
}

BaseObjectIterator* 
BaseObjectIteratorFromInt16List::Copy() const
{
   Validate();
   BaseObjectIteratorFromInt16List* iter = new BaseObjectIteratorFromInt16List(_listIter,_objects);
   assert(ValidPtr(iter));
   iter->Validate();
   return iter;
}

//==============================================================================

void 
BaseObjectIteratorFromInt16List::_Validate() const
{
   _listIter.Validate();
   _objects.Validate();
}

//==============================================================================

