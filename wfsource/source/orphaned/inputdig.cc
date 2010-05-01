#define _INPUTDIG_CC

//=============================================================================
// inputdig.cc:
//=============================================================================
/*
Documentation:

	Abstract:
	   		C++ class representing a buttons-only input device and recent state

	History:
			24 Jan 95 JB	Created	by Joseph Boyle 24 Jan 95
			 1 Apr 95 WBNIV	Convenience to pass in button to test added
			 2 Apr 95 WBNIV	Added button remapping
			 9 Apr 96 PAT	Added InputCameraRemapper class
			14 May 96 PAT	Added methods for reading raw data from a remapped device

	Class Hierarchy:
			none

	Dependencies:
			IJoystick

	Restrictions:


*/
//==============================================================================
#include "inputdig.hp"
#include "game.hp"
#include "level.hp"
#include "camera.hp"

extern Level* theLevel;

//==============================================================================

InputCameraRemapper::InputCameraRemapper(Input* unmappedInput, Memory& memory) : Input(), _memory(memory)
{
	_memory.Validate();
	assert(ValidPtr(unmappedInput));
	inputSource = unmappedInput;
}


InputCameraRemapper::~InputCameraRemapper()
{
	assert( ValidPtr( inputSource ) );
	MEMORY_DELETE(_memory,inputSource,Input);
//	DELETE_CLASS( inputSource );
}


//==============================================================================
// Remapping table used by InputCameraRemapper::read()

int32 dirTable[] =					// table of joystick directions to look up new direction in
{
	EJ_BUTTONF_UP,				// Angle = 225 to 315
	EJ_BUTTONF_DOWN,
	EJ_BUTTONF_LEFT,
	EJ_BUTTONF_RIGHT,
	EJ_BUTTONF_RIGHT,					// Angle = 135 to 225
	EJ_BUTTONF_LEFT,
	EJ_BUTTONF_UP,
	EJ_BUTTONF_DOWN,
	EJ_BUTTONF_DOWN,					// Angle = 45 to 135 (no correction)
	EJ_BUTTONF_UP,
	EJ_BUTTONF_RIGHT,
	EJ_BUTTONF_LEFT,
	EJ_BUTTONF_LEFT,					// Angle = 315 to 45
	EJ_BUTTONF_RIGHT,
	EJ_BUTTONF_DOWN,
	EJ_BUTTONF_UP
};

//-----------------------------------------------------------------------------

joystickButtonsF
InputCameraRemapper::read()
{
	assert( ValidPtr( inputSource ) );
	int32 joybits = inputSource->read();
	int32  retval =  joybits & ~(EJ_BUTTONF_UP|EJ_BUTTONF_LEFT|EJ_BUTTONF_RIGHT|EJ_BUTTONF_DOWN);			// mask off direction
	assert( ValidPtr( theLevel ) );
	assert( ValidPtr( theLevel->camera() ) );
	Vector3 lookAt = theLevel->camera()->GetLookAt();

	int32 angle = 0;						// Build a bitfield to encode the look-at direction...
	if(lookAt.X().Abs() < lookAt.Y().Abs())
	{
		angle |= 1;
		if (lookAt.Y() < Scalar::zero )
			angle |= 2;
	}
	else
	{
		if (lookAt.X() >= Scalar::zero )
			angle |= 2;
	}

	assert(angle >= 0);
	assert(angle < 4);
	angle *= 4;

	DBSTREAM3( cactor << "ICR:Read: vector = " << lookAt << ", angle = " << angle <<  std::endl; )

	if(joybits & EJ_BUTTONF_UP)
		retval |= dirTable[angle];
	if(joybits & EJ_BUTTONF_DOWN)
		retval |= dirTable[angle+1];
	if(joybits & EJ_BUTTONF_LEFT)
		retval |= dirTable[angle+2];
	if(joybits & EJ_BUTTONF_RIGHT)
		retval |= dirTable[angle+3];

	return(retval);
}

//-----------------------------------------------------------------------------

joystickButtonsF
InputCameraRemapper::readRaw()
{
	assert( ValidPtr( inputSource ) );
	return (inputSource->read());
}

//=============================================================================

