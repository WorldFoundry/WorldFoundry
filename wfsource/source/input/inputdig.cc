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
			14 May 96 PAT	Added methods for reading raw data from a remapped device

	Class Hierarchy:
			none

	Dependencies:
			IJoystick

	Restrictions:


*/
//==============================================================================
#include "inputdig.hp"

//==============================================================================

InputNull::InputNull() : Input()
{
}


joystickButtonsF
InputNull::read()
{
	return 0;
}

joystickButtonsF
InputNull::readRaw()
{
	return 0;
}

void
InputNull::setButtons( joystickButtonsF )
	{
	assert( 0 );		// Allow once checking can happen above
	}

//==============================================================================

InputScript::InputScript()
	{
	_buttons = joystickButtonsF( 0 );
	}

joystickButtonsF
InputScript::read()
	{
	joystickButtonsF retButtons = _buttons;
	_buttons = joystickButtonsF( 0 );
	return retButtons;
	}

joystickButtonsF
InputScript::readRaw()
	{
	return read();
	}

void
InputScript::setButtons( joystickButtonsF buttons )
	{
	_buttons = buttons;
	}

//==============================================================================

InputJoystick::InputJoystick( joystickWhich joynum ) : Input()
{
	_stick = JoystickNew( joynum );
	assert( _stick );
}

InputJoystick::~InputJoystick()
{
	JoystickDelete( _stick );
}

joystickButtonsF
InputJoystick::read()
{
	return JoystickGetButtonsF( _stick );
}

joystickButtonsF
InputJoystick::readRaw()
{
	return JoystickGetButtonsF( _stick );
}

//==============================================================================
void
QInputDigital::reset()
{
	_current = joystickButtonsF( 0 );
	_previous = joystickButtonsF( 0 );
	_edgeDown = joystickButtonsF( 0 );
	_edgeUp = joystickButtonsF( 0 );

	_currentRaw = joystickButtonsF( 0 );
	_previousRaw = joystickButtonsF( 0 );
	_edgeDownRaw = joystickButtonsF( 0 );
	_edgeUpRaw = joystickButtonsF( 0 );
}


QInputDigital::QInputDigital( Input* input, int )
{
	_memory = NULL;
	assert( ValidPtr( input ) );
	_input = input;		// owns nonexclusively
	reset();
    update();           // so that any keys held down at the beginning of the level don't get noticed by justpressed
}


QInputDigital::QInputDigital( Input* input, Memory& memory ) : _memory(&memory)
{
	DBSTREAM5( cflow << "QInputDigital::QInputDigital:" << std::endl; )
	_memory->Validate();
	assert( ValidPtr( input ) );
	_input = input;		// owns nonexclusively
	reset();
    update();           // so that any keys held down at the beginning of the level don't get noticed by justpressed
	DBSTREAM5( cflow << "QInputDigital::QInputDigital:done" << std::endl; )
}


void
QInputDigital::update()
{
	DBSTREAM5( cflow << "QInputDigital::update:" << std::endl; )
	assert( ValidPtr( _input ) );
	_previous = _current;
	_current = _input->read();		//JoystickGetButtonsF(_ijoy);
	_edgeDown = _current & ~_previous;
	_edgeUp = ~_current & _previous;

	_previousRaw = _currentRaw;
	_currentRaw = _input->readRaw();		//JoystickGetButtonsF(_ijoy);
	_edgeDownRaw = _currentRaw & ~_previousRaw;
	_edgeUpRaw = ~_currentRaw & _previousRaw;
	DBSTREAM5( cflow << "QInputDigital::update: done" << std::endl; )
}


void
QInputDigital::setButtons( joystickButtonsF buttons )
	{
	assert( ValidPtr( _input ) );
	_input->setButtons( buttons );
	}


joystickButtonsF
QInputDigital::arePressed( joystickButtonsF joyMask ) const
{ // Buttons currently pressed
	return _current & joyMask;
}

joystickButtonsF
QInputDigital::arePressedRaw( joystickButtonsF joyMask ) const
{ // Buttons currently pressed
	return _currentRaw & joyMask;
}

joystickButtonsF
QInputDigital::justPressed( joystickButtonsF joyMask ) const
{ // Buttons just pressed this time
	return _edgeDown & joyMask;
}

joystickButtonsF
QInputDigital::justPressedRaw( joystickButtonsF joyMask ) const
{ // Buttons just pressed this time
	return _edgeDownRaw & joyMask;
}

joystickButtonsF
QInputDigital::justReleased( joystickButtonsF joyMask ) const
{ // Buttons pressed before last update, but not any more
	return _edgeUp & joyMask;
}

joystickButtonsF
QInputDigital::justReleasedRaw( joystickButtonsF joyMask ) const
{ // Buttons pressed before last update, but not any more
	return _edgeUpRaw & joyMask;
}

void
QInputDigital::SetInputDevice(Input* newInputDevice)
{
	assert(ValidPtr(newInputDevice));
	if(_memory)
		MEMORY_DELETE((*_memory),_input,Input);
	//DELETE_CLASS( _input );
	_input = newInputDevice;
}


#if 0
static void
QInputDigital::test() {
printf("Testing digital input state class\n");
	QInputDigital foo = QInputDigital(JoystickNew(EJW_JOYSTICK1));
	QInputDigital bar = QInputDigital(JoystickNew(EJW_JOYSTICK2));
	while (!PIGSUserAborted()) {
		foo.update();
		bar.update();
		printf("Joypad 1: ");
   		JoystickButtonsFPrint(foo.arePressed());
		printf("Joypad 2: ");
		JoystickButtonsFPrint(bar.arePressed());
		printf("\n");

		printf("Presses 1: ");
   		JoystickButtonsFPrint(foo.justPressed());
		printf("Presses 2: ");
		JoystickButtonsFPrint(bar.justPressed());
		printf("\n");

		printf("Releases 1: ");
   		JoystickButtonsFPrint(foo.justReleased());
		printf("Releases 2: ");
		JoystickButtonsFPrint(bar.justReleased());
		printf("\n");
		printf("\n");
	}
}

#endif
//=============================================================================

