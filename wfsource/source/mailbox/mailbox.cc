//==============================================================================
// mailbox/mailbox.cc: 
// Copyright ( c ) 2003 World Foundry Group  
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

#include <pigsys/pigsys.hp>
#include <cpplib/algo.hp>
#include "mailbox.hp"

//==============================================================================

Mailboxes::~Mailboxes()
{

}

//==============================================================================

#if SW_DBSTREAM

std::ostream& operator <<( std::ostream& s, const Mailboxes& mailboxes )
{
	mailboxes._Print( s );
    return s;
}

void
Mailboxes::_Print( std::ostream& s ) const
{
    s << "Mailboxes: " << std::endl;
}

#endif // SW_DBSTREAM

//==============================================================================

MailboxesWithStorage::MailboxesWithStorage(long mailboxBase, long numberOfLocalMailboxes, Mailboxes* parent) :
    _mailboxBase(mailboxBase),
    _localMailboxes(numberOfLocalMailboxes),
    _parent(parent)
{
    RangeCheck(0,_mailboxBase,10000);   // kts arbitrary
    RangeCheck(0,numberOfLocalMailboxes,10000);   // kts arbitrary

    for(int index=0; index < numberOfLocalMailboxes;index++)
        _localMailboxes[index] = Scalar::zero;
}


MailboxesWithStorage::~MailboxesWithStorage()
{

}

Scalar 
MailboxesWithStorage::ReadMailbox(long mailbox) const
{
    if(mailbox >= _mailboxBase && mailbox < _localMailboxes.Size()+_mailboxBase)
    {
        return _localMailboxes[mailbox-_mailboxBase];
    }
    else
    {
        if(_parent)
            return _parent->ReadMailbox(mailbox);
        else
            assert(0);
            return Scalar::zero;
    }
}

void 
MailboxesWithStorage::WriteMailbox(long mailbox, Scalar value)
{
    if(mailbox >= _mailboxBase && mailbox < _localMailboxes.Size()+_mailboxBase)
    {
        _localMailboxes[mailbox-_mailboxBase] = value;

    }
    else
    {
        if(_parent)
            _parent->WriteMailbox(mailbox, value);
        else
            AssertMsg(0,"Attempt to write mailbox " << mailbox << " which doesn't exist");
    }
}


#if SW_DBSTREAM

void
MailboxesWithStorage::_Print( std::ostream& s ) const
{
    Mailboxes::_Print(s);
    s << "Local storage: ";
    s << _localMailboxes << std::endl;
}

#endif // SW_DBSTREAM

//==============================================================================

MailboxesManager::~MailboxesManager()
{

}

//==============================================================================

SingleMailboxesManager::SingleMailboxesManager(Mailboxes& mailboxes) :
_mailboxes(mailboxes)
{

}

//==============================================================================


SingleMailboxesManager::~SingleMailboxesManager()
{

}

//==============================================================================

Mailboxes& 
SingleMailboxesManager::LookupMailboxes(int objectIndex)
{
    assert(objectIndex == 0);
    return _mailboxes;
}
//==============================================================================

