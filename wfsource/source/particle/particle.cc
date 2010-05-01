//=============================================================================
// particle/particle.cc:
// Copyright ( c ) 1997,98,99 World Foundry Group  
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
// Original Author: William B. Norris IV
//============================================================================

#include <particle/particle.hp>

//==============================================================================

#pragma message ("KTS " __FILE__ ": keep this in sync with whats in the oad")
#pragma message ("KTS " __FILE__ ": figure out a way to nest structures in the .ht file")
struct SingleForce
{
		int32 forceType;               /* Minumum: 0 Maximum: 3 Default: 0 */
		Scalar Magnitude;         /* Minumum: ( ((long) 0.01 * 65536.0)) Maximum: ( ((long) 10 * 65536.0)) */
		Scalar VectorX;         /* Minumum: ( ((long) -1000 * 65536.0)) Maximum: ( ((long) 1000 * 65536.0)) */
		Scalar VectorY;         /* Minumum: ( ((long) -1000 * 65536.0)) Maximum: ( ((long) 1000 * 65536.0)) */
		Scalar VectorZ;         /* Minumum: ( ((long) -1000 * 65536.0)) Maximum: ( ((long) 1000 * 65536.0)) */
};

void
Particle::Update(const Clock& /*clock*/, const _Mesh& meshOAD)
{
	alpha -= meshOAD.GetalphaDecrement();
//			if ( cur_point->alpha <= Scalar::zero )
	pos += _deltaPos;
	rot += _deltaRot;

	const int forceEntries = 2;

	const SingleForce* force = (SingleForce*)&meshOAD.forceType;

	for(int forceIndex=0;forceIndex<forceEntries;forceIndex++,force++)
	{
		// now do forces
		switch(force->forceType)
		{
#pragma message ("KTS " __FILE__ ": figure out a way to do enumerations in the .ht file")
			case 0:				        // none
				break;
			case 1:				        // constant
				_deltaPos += *(const Vector3*)&force->VectorX;
				break;
			case 2:				        // random
				_deltaPos += Vector3(
					Scalar::Random(Scalar::zero, force->VectorX),
					Scalar::Random(Scalar::zero, force->VectorY),
					Scalar::Random(Scalar::zero, force->VectorZ));
				break;
			case 3:				        // radial
				// first treat the vector as a position
				const Vector3& pointPos = *(const Vector3*)&force->VectorX;

				// now calculate a unit vector pointing in the direction of the force
				// (created from a line between the point and the particles current position)
				Vector3 delta = pos - pointPos;
				delta.Normalize();
				delta *= force->Magnitude;
				_deltaPos += delta;
				break;
		}
	}
}

//==============================================================================
