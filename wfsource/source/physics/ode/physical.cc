//==============================================================================
// ode/physical.cc:
// Copyright (c) 2002 World Foundry Group  
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
// Description: The PhysicalAttributes class encapsulates the objects position
//   and orientation, collision boxes, and velocity vectors
// Original Author: Kevin T. Seghetti / Phil Torre
//==============================================================================

void
PhysicalAttributes::Update()
{
    assert(_geom);
    if(_geom)
    {
        ode::dGeomID g = _geom;
        assert(g);
        const ode::dReal* pos = ode::dGeomGetPosition(g);
        const ode::dReal* R = ode::dGeomGetRotation(g);

        int type = ode::dGeomGetClass (g);
        assert(type == ode::dBoxClass);

//             dVector3 sides;
//             dGeomBoxGetLengths (g,sides);
          SetMatrix( 
              Matrix34
              (
                 Vector3(Scalar::FromDouble(R[0]),Scalar::FromDouble(R[4]),Scalar::FromDouble(R[8])),
                 Vector3(Scalar::FromDouble(R[1]),Scalar::FromDouble(R[5]),Scalar::FromDouble(R[9])),
                 Vector3(Scalar::FromDouble(R[2]),Scalar::FromDouble(R[6]),Scalar::FromDouble(R[10])),
                 Vector3(Scalar::FromDouble(pos[0]),Scalar::FromDouble(pos[1]),Scalar::FromDouble(pos[2]))
                 ));    
    }
}
                           
//==============================================================================

