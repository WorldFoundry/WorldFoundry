//=============================================================================
// Copyright ( c ) 1997,99,2000,2001,2002 World Foundry Group  
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
// ===========================================================================

#include "scriptinterpreter.hp"
#include "tcl.hp"

//==============================================================================
 
ScriptInterpreter::ScriptInterpreter(MailboxesManager& manager) : _mailboxesManager(manager)
{
}

//==============================================================================

ScriptInterpreter::~ScriptInterpreter()
{
}

//==============================================================================

void 
ScriptInterpreter::Validate()
{

}

//==============================================================================

void 
ScriptInterpreter::AddConstantArray(IntArrayEntry* /*entryList */)
{

}

//==============================================================================

void 
ScriptInterpreter::DeleteConstantArray(IntArrayEntry* /*entryList*/)
{

}

//==============================================================================

ScriptInterpreter* ScriptInterpreterFactory(MailboxesManager& mailboxesManager, Memory& memory)
{
   // kts right now the factory always makes TCL interpreters, I need to decide how the designer will specify
   // which laungauge each script is in (I could either put it at the top line of the script, or have a seperate OAS field for it)

   return new (memory) TCLInterpreter(mailboxesManager);
}

//==============================================================================

