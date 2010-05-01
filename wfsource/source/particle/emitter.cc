//=============================================================================
// particle/emitter.cc:
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

#include <particle/emitter.hp>

#if 0
//	static const EmitterParameters wall;	// = { 100,  0, 0, 0,  1,  1, 0, 0, 1,  1, 1.3, 0.01, 32, 0 };
//	static EmitterParameters shell = { 10000,  0, 0, -1,  1,  1, 0, 0, 1,  0.4, 0.8, 0.01, 1, 0 };
//	static EmitterParameters spray = { 250,  0, 0, -1,  1,  1, 0, 0, 1,  0.4, 0.8, 0.01, 100, 0 };
//	static EmitterParameters i1e16o = { 100,  0, 0, 0,  1,  1, 0, 0, 1,  1, 1.3, 0.01, 32 };
//	static EmitterParameters i2e3o_0 = { 100  2 0 0   1  1 0 0 1  1 .3 .01 12 };
//	static EmitterParameters i2e3o_1 = { 100  -2 0 0   1  1 0 0 1  1 .3 .01 12 };
//	static EmitterParameters overlap_0 = { 100  0 0 0   1  1 0 0 1  1 1.3 .01 32 0 };
// overlap.txt [2--]
100  0 0 0   1  1 1 0 1  1 1.3 .01 32 0
100  0 0 0   1  1 1 1 1  1 1.3 .01 32 0
100  0 0 0   1  1 0 1 1  1 1.3 .01 32 0
100  0 0 0   1  0 0 1 1  1 1.3 .01 32 0
// pinball.txt
100  0 0 -1   1  1 0 0 1  .4 1.3 .01 10 0
100  0 3 10   1  1 1 0 1  .4 1.3 .01 10 5
100 -10 7 15   1  1 1 1 1  .4 3.3 .01 10 15
// pingpong.txt
100  0 -4 0   1  1 0 0 1  .4 1.3 .01 1 0
100  0  4 0   1  1 0 0 1  .4 2.6 .01 1 5
100  0 -4 0   1  1 0 0 1  .4 3.9 .01 1 10
// ripple.txt
100  -30 0 0   1  1 0 0 1  1 1.3 .01 32 0
100  -20 0 0   1  1 1 0 1  1 1.3 .01 32 10
100    0 0 0   1  1 1 1 1  1 1.3 .01 32 20
100   10 0 0   1  1 0 1 1  1 1.3 .01 32 30
100   20 0 0   1  0 0 1 1  1 1.3 .01 32 40
// squeeze.txt
100  0 -4 0   1  1 0 0 1  .4 1.3 .01 10 0
100  0  4 0   1  1 0 0 1  .4 1.3 .01 10 0
#endif

//==============================================================================

Emitter::Emitter( Memory& memory, const _Mesh& meshOAD, const EmitterParameters& ep, const ParticleParameters& pp, const Clock& clock ) :
	_memory( memory ),
	_meshOAD(meshOAD),
	_particleParameters( pp ),
	_particleMemPool(sizeof(Particle),ep.nParticles*( Scalar::one / _particleParameters.alphaDecrement ).Round()*10,memory MEMORY_NAMED( COMMA "Particle Mem Pool" ))
{
	_nParticles = ep.nParticles;
	_position = ep.position;
	_rotation = ep.rotation;
	_currentSphereRadius = ep.initialSphereRadius;

	_timeStart = clock() + ep.startDelay;
	_period = ep.period;
	_timeToGenerate = _timeStart + _period;

	_yArc = ep.yArc;
	_zArc = ep.zArc;

	RangeCheckInclusive( Scalar::zero, _particleParameters.initialAlpha, Scalar::one );
	RangeCheckInclusive( Scalar::zero, _particleParameters.alphaDecrement, Scalar::one );
	assert( _particleParameters.templateObject );

#pragma message( __FILE__ ": could set time to generate to _timeStart & not construct here" )

//	cout << "Emitter::Emitter _timeStart = " << _timeStart << endl;

	_nParticlesAvailable = _nParticles * ( Scalar::one / _particleParameters.alphaDecrement ).Round()*10;
//	cout << "_nParticlesAvailable = " << _nParticlesAvailable << endl;
	_tblParticles = new ( _memory )( Particle* [ _nParticlesAvailable ] );
	assert( ValidPtr( _tblParticles ) );

	_tblObjects = new ( _memory )( RenderObject3D* [ _nParticlesAvailable ] );
	assert( ValidPtr( _tblObjects ) );

	for ( int idxParticle=0; idxParticle<_nParticlesAvailable; ++idxParticle )
	{
		_tblParticles[ idxParticle ] = NULL;
		_tblObjects[ idxParticle ] = NULL;
	}

//	GeneratePoints();
}

//==============================================================================

Emitter::~Emitter()
{
	for ( int idxParticle=0; idxParticle<_nParticlesAvailable; ++idxParticle )
	{
		if ( _tblObjects[ idxParticle ] )
			MEMORY_DELETE( _memory, _tblObjects[ idxParticle ], RenderObject3D );
		if ( _tblParticles[ idxParticle ] )
			_particleMemPool.Free(_tblParticles[ idxParticle]);
//			MEMORY_DELETE( _memory, _tblParticles[ idxParticle ], Particle );
	}
}

//==============================================================================

bool
Emitter::Update( const Clock& clock )
{
	bool retval = false;

//	cout << "Emitter::Update() clock = " << clock() << endl;
//	cout << "Emitter::Update() _timeStart = " << _timeStart << endl;
//	cout << "Emitter::Update() _timeToGenerate = " << _timeToGenerate << endl;

	_bRenderThisFrame = clock() >= _timeStart;
	if ( !_bRenderThisFrame )
		return retval;

	_currentSphereRadius += _particleParameters.initialVelocity; //calculate explosion radius

	if ( clock() >= _timeToGenerate )
	{
		GeneratePoints(clock);
		_timeToGenerate += _period;
	}

	for ( int idxParticle = 0; idxParticle < _nParticlesAvailable; ++idxParticle )
	{
		// move, spin, and fade out each object
		Particle* currentParticle = _tblParticles[ idxParticle ];
		if ( currentParticle )
		{
			assert( ValidPtr( currentParticle ) );
//			cout << "updating particle " << idxParticle << "timetodie = " << currentParticle->_timeToDie << endl;
//			cout << "Emitter::Update() clock = " << clock() << endl;
			currentParticle->Update(clock, _meshOAD);

			if ( clock() >= currentParticle->_timeToDie)
			{
				//cout << "deleting particle " << idxParticle << endl;
				delete _tblParticles[ idxParticle ];
				_tblParticles[ idxParticle ] = NULL;
				// don't delete the model because it can be re-used
				continue;
			}
		}
	}
	return retval;
}

//==============================================================================

void
Emitter::Render( RenderCamera& camera )
{
//	cout << "brender" << _bRenderThisFrame << endl;
	if ( !_bRenderThisFrame )
		return;

//	cout << "Emitter::Render() clock = " << clock() << endl;
//	cout << "Emitter::Render() _timeStart = " << _timeStart << endl;

//	cout << endl;

	for ( int idxParticle = 0; idxParticle < _nParticlesAvailable; ++idxParticle )
	{
		// move each point
//		cout << "checking particle " << idxParticle << endl;
		Particle* currentParticle = _tblParticles[ idxParticle ];
		if ( currentParticle )
		{
//			cout << "Particle #" << idxParticle << "\tAlpha: " << currentParticle->alpha << "\tPos: " << currentParticle->pos << endl;

//			Vector3 position( currentParticle->pos );
//?			position += Vector3(SCALAR_CONSTANT(2)*Scalar(index%3,0),SCALAR_CONSTANT(2)*Scalar(index/3,0),Scalar::zero);
			Matrix34 mat3( currentParticle->rot, currentParticle->pos);
			camera.RenderObject( *_tblObjects[ idxParticle ], mat3);
		}
	}
}

//==============================================================================

void
Emitter::GeneratePoints(const Clock& clock)
{
//	cout << "generating points!" << endl;
	for ( int nParticle=0; nParticle<_nParticles; ++nParticle )
	{
		// find available particle slot
        int idxParticle;
		for ( idxParticle=0; idxParticle<_nParticlesAvailable; ++idxParticle )
		{
			if ( !_tblParticles[ idxParticle ] )
				break;
		}
		if ( idxParticle == _nParticlesAvailable )
			return;

//		cout << "Generating particle at " << idxParticle << endl;
		if ( !_tblObjects[ idxParticle ] )
			_tblObjects[ idxParticle ] = new ( _memory ) RenderObject3D( _memory, *_particleParameters.templateObject );
//		cout << "_tblObjects[ " << idxParticle << " ] = " << _tblObjects[ idxParticle ] << endl;
		assert( ValidPtr( _tblObjects[ idxParticle ] ) );

		_tblParticles[ idxParticle ] = new ( _particleMemPool ) Particle;
		assert( ValidPtr( _tblParticles[ idxParticle ] ) );

		Particle* p = _tblParticles[ idxParticle ];
		assert( ValidPtr( p ) );

		p->alpha = _particleParameters.initialAlpha;
		p->rot = Euler::zero;
		p->rot = _particleParameters.initialRotation;
		p->_deltaRot = _particleParameters.deltaRotation;
		p->_timeToDie = clock() + _particleParameters.lifeTime;

//		cout << "clock = " << clock() << ", timetodie = " << p->_timeToDie << "lifeTime = " << _particleParameters.lifeTime << endl;
		Angle angle1(Angle::Revolution(Scalar::Random(Scalar::zero, _yArc)));
		Angle angle2(Angle::Revolution(Scalar::Random(Scalar::zero, _zArc)));
		Vector3 direction(Scalar::one, Scalar::zero, Scalar::zero);
		direction.RotateY(angle1);
		direction.RotateZ(angle2);

#pragma message ("KTS " __FILE__ ": kts temp code, once well know, replace it with the new euler * vector code below")
//		cout << "rotation = " << _rotation << endl;

		Matrix34 tempMat(_rotation,Vector3::zero);
//		cout << "matrix = " << tempMat << endl;
//		Vector3 tempVect;
		direction = direction * tempMat;
//		cout << "direction  = " << direction << endl;
//		direction *= _rotation;
//		assert(tempVect == direction);

		Scalar randomizedVelocity = _particleParameters.initialVelocity * Scalar::Random( SCALAR_CONSTANT( 0.9 ), SCALAR_CONSTANT( 1.1 ) );
		p->_deltaPos = direction*randomizedVelocity;
		p->pos = _position + p->_deltaPos;
	}
}
//==============================================================================
