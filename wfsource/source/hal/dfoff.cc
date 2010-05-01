//=============================================================================
// DiskFileOFF.cc:
// Copyright ( c ) 1996,97,99 World Foundry Group  
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
// Description:
//============================================================================

#define _DFOFF_CC

//=============================================================================

void
HalInitFileSubsystem()
{
#if defined(__PSX__)
	int PCinit_err = PCinit();			// xina - initialize filesystem
	assert( PCinit_err == 0 );
#endif
}

//=============================================================================

#include <pigsys/pigsys.hp>
#include <cpplib/stdstrm.hp>
#include <pigsys/genfh.hp>
#include <hal/diskfile.hp>

//=============================================================================
