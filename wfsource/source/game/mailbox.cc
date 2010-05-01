//==============================================================================
// game/mailbox.cc: game specific mailbox Read/Write routines
// Copyright ( c ) 2002,2003 World Foundry Group  
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

#include "level.hp"
#include "game.hp"
#include "mailbox.hp"

//==============================================================================
                    
LevelMailboxes::LevelMailboxes(Level& level, Mailboxes* parent) :
MailboxesWithStorage(EMAILBOX_GLOBAL_USER_START,EMAILBOX_GLOBAL_USER_MAX-EMAILBOX_GLOBAL_USER_START,parent),
_level(level)
{
    assert(EMAILBOX_GLOBAL_USER_START == 0);
	MailboxesWithStorage::WriteMailbox( 0, Scalar::zero);
	MailboxesWithStorage::WriteMailbox( 1, Scalar::one);
}

//==============================================================================
                     
Scalar
LevelMailboxes::ReadMailbox(long mailbox) const
{
   DBSTREAM1(cmailbox << "LevelMailboxes::ReadMailbox: mailbox = " << mailbox << std::endl; )

   if(mailbox >= EMAILBOX_GLOBAL_SYSTEM_START && mailbox < EMAILBOX_GLOBAL_SYSTEM_MAX)
       return _level.ReadSystemMailbox(mailbox);
   else
       return MailboxesWithStorage::ReadMailbox(mailbox);
}

//==============================================================================

void
LevelMailboxes::WriteMailbox(long mailbox, Scalar value)
{
    if(mailbox >= EMAILBOX_GLOBAL_SYSTEM_START && mailbox < EMAILBOX_GLOBAL_SYSTEM_MAX)
        _level.WriteSystemMailbox(mailbox, value);
    else
    {
        if(mailbox >= 2)
            MailboxesWithStorage::WriteMailbox(mailbox,value);
        else
            AssertMsg(mailbox >= 2, "attempt to write to mailbox #" << mailbox << ", which is not allowed");
    }
}

//==============================================================================
                    
GameMailboxes::GameMailboxes(WFGame& game) :
MailboxesWithStorage(EMAILBOX_PERSISTENT_USER_START,EMAILBOX_PERSISTENT_USER_MAX-EMAILBOX_PERSISTENT_USER_START,NULL),
_game(game)
{

}

//==============================================================================

Scalar
GameMailboxes::ReadMailbox(long mailbox) const
{
   DBSTREAM1(cmailbox << "GameMailboxes::ReadMailbox: mailbox = " << mailbox << std::endl; )

   if(mailbox >= EMAILBOX_PERSISTENT_SYSTEM_START && mailbox < EMAILBOX_PERSISTENT_SYSTEM_MAX)
       return _game.ReadSystemMailbox(mailbox);
   else
       return MailboxesWithStorage::ReadMailbox(mailbox);
}

//==============================================================================

void
GameMailboxes::WriteMailbox(long mailbox, Scalar value)
{
   if(mailbox >= EMAILBOX_PERSISTENT_SYSTEM_START && mailbox < EMAILBOX_PERSISTENT_SYSTEM_MAX)
       _game.WriteSystemMailbox(mailbox, value);
   else
       MailboxesWithStorage::WriteMailbox(mailbox,value);

}

//==============================================================================

