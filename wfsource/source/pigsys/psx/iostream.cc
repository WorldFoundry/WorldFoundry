// ------------------------------------------------------------------------
// iostream.cc
// ------------------------------------------------------------------------

#include <pigsys/pigsys.hp>
#include <iostream>
#include <_iostr.hp>
#include <pigsys/genfh.hp>
#include <hal/hal.h>
#include <libgte.h>
#include <libgpu.h>

// used by screen_flush
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#if DO_IOSTREAMS

//=============================================================================

StreamOutput::~StreamOutput()
{
	assert(useCount == 0);
}

//=============================================================================

void
StreamOutput_stdout::OutputChars(const char* string, int ASSERTIONS(length))
{
	assert(string[length] == 0);
	printf( "%s", string );
}

//=============================================================================

void
StreamOutput_screen::OutputChars(const char* string, int ASSERTIONS(length))
{
	assert(string[length] == 0);
	FntPrint(string);
}

//=============================================================================

void
StreamOutput_screen_flush::OutputChars(const char* string, int ASSERTIONS(length))
{
	assert(string[length] == 0);
	FntPrint(string);
}

//-----------------------------------------------------------------------------

void
StreamOutput_screen_flush::Flush()
{
	static RECT bg = {0, 0, 320, 240};
	static DISPENV g_ExDispEnv;
	static DRAWENV g_ExDrawEnv;

	if(firstTime)
	{
		ResetGraph(0);
		FntLoad(640, 0);
		SetDumpFnt(FntOpen(0,12, 320, 200, 0, 1024));
		SetDefDrawEnv(&g_ExDrawEnv, 0, 0, 320, 240);
		SetDefDispEnv(&g_ExDispEnv, 0, 0, 320, 240);
		PutDrawEnv(&g_ExDrawEnv);
		PutDispEnv(&g_ExDispEnv);
		SetDispMask(1);
	}

	DrawSync(0);
	VSync(0);
	ClearImage(&bg, 10, 50,10);
	FntFlush(-1);

	FntLoad(640, 0);
	SetDumpFnt(FntOpen(0,12, 320, 200, 0, 1024));
//	SetDefDrawEnv(&g_ExDrawEnv, 0, 0, 320, 240);
//	SetDefDispEnv(&g_ExDispEnv, 0, 0, 320, 240);
//	PutDrawEnv(&g_ExDrawEnv);
//	PutDispEnv(&g_ExDispEnv);
//	SetDispMask(1);

	firstTime = 0;
}

//=============================================================================
//=============================================================================

StreamOutput_null null_output;
StreamOutput_stdout stdout_output;
StreamOutput_screen screen_output;
StreamOutput_screen_flush screen_output_flush;

ostream cnull(null_output);
ostream cout(stdout_output);
ostream cerr(stdout_output);
ostream cscreen(screen_output);
ostream cscreenflush(screen_output_flush);

// ------------------------------------------------------------------------

//const char * endl = "\r\n";
stream_endl endl;

// ------------------------------------------------------------------------

ios::ios()
{
	x_flags = 0;
    x_fill = ' ';
    x_width = 0;
	Validate();
}

// ------------------------------------------------------------------------

int
ios::GetBase()
{
	Validate();
	if(x_flags & ios::hex)
		return 16;
	else
		return 10;
}

// ------------------------------------------------------------------------
// returns a pointer to the first character used in the buffer

char*
ostream::_PrintNumBase(char* buffer, int bufLen, unsigned int val,int base)
{
	Validate();
	assert(bufLen > x_width);
	char* reverseBuffer = buffer+(bufLen-1);
	*reverseBuffer-- = '\0';


// kts kludge, base 10 is signed, base 16 is not

	bool minusFlag = false;
	if(base == 10)
	{
		assert(sizeof(unsigned int) == 4);
		if( val  & 0x80000000)
		{
			val = -val;
			minusFlag = true;
		}
	}


//	reverseBuffer -= 18;
//	sprintf(reverseBuffer, "PNBase: %x",val);

	char* desiredWidth = reverseBuffer-x_width;
	assert(x_width >= 0);
	char ch;
	if(val)
		while(val && reverseBuffer >= buffer)
		{
			int digit = val % base;
			if(digit > 9)
				ch = 'A' + (digit-10);
			else
				ch = '0' + digit;
			*reverseBuffer-- = ch;
			val = val / base;
		}
	else
		*reverseBuffer-- = '0';
//	assert(buffer <= reverseBuffer);
	while(reverseBuffer > desiredWidth)
		*reverseBuffer-- = x_fill;
	if(minusFlag)
		*reverseBuffer-- = '-';
	reverseBuffer++;
	return reverseBuffer;
}

// ------------------------------------------------------------------------

ostream::ostream( StreamOutput& output )
{
	assert(ValidPtr(&output));
//	printf("constructing ostream at address %p from StreamOutput at address %p\n",this,&output);
	_output = &output;
	_output->IncUseCount();
	Validate();
}

// ------------------------------------------------------------------------


ostream::ostream( ostream& other )
{
//	other.Validate();				// cannot, other could be cnull or such

//	printf("constructing ostream at address %p from another ostream at address %p\n",this,&other);
	if(this != &other)
	{
		// kts hack since we cannot control the ordering of global class construction

		if(!other._output)
		{
			if(&other == &cnull)
				_output = &null_output;
			else if(&other == &cout)
				_output = &stdout_output;
			else if(&other == &cerr)
				_output = &stdout_output;
			else
				assert(0);
		}
		else
		{
			assert(ValidPtr(other._output));
			_output = other._output;
		}
		assert(ValidPtr(_output));
		_output->IncUseCount();
	}
	Validate();
}

// ------------------------------------------------------------------------

ostream::~ostream( void )
{
	Validate();
	_output->DecUseCount();
	if(!_output->UseCount())
		MEMORY_DELETE(HALDmalloc,_output, StreamOutput );
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const char val )
{
	Validate();
	char buffer[longest_printf];
	int length = sprintf( buffer, "%c", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(buffer,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const short val )
{
	Validate();
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;
//	char buffer[longest_printf];
//	int length;
//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%hx", val );
//	else
//		length = sprintf( buffer, "%hd", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const long val )
{
	Validate();
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;

//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%lx", val );
//	else
//		length = sprintf( buffer, "%ld", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const int val )
{
	Validate();
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;
//	char buffer[longest_printf];
//	int length;
//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%x", val );
//	else
//		length = sprintf( buffer, "%d", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const unsigned char val )
{
	Validate();
	char buffer[longest_printf];
	int length = sprintf( buffer, "%c", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(buffer,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const unsigned short val )
{
	Validate();
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;
//	char buffer[longest_printf];
//	int length;
//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%hx", val );
//	else
//		length = sprintf( buffer, "%hu", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const unsigned long val )
{
	Validate();
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;
//	char buffer[longest_printf];
//	int length;
//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%lx", val );
//	else
//		length = sprintf( buffer, "%lu", val );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const unsigned int val )
{
	Validate();

//	printf("processing %d\n", val);
	char buffer[longest_printf];
	char* bufferStart = _PrintNumBase(buffer, longest_printf, val, GetBase());
	int length = (buffer+(longest_printf-1)) - bufferStart;

//	char buffer[longest_printf];
//	int length;
//	if(x_flags & ios::hex)
//		length = sprintf( buffer, "%x", val );
//	else
//		length = sprintf( buffer, "%u", val );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(bufferStart,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const char * val )
{
	Validate();
	int length = strlen( val );
	_output->OutputChars(val,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const void * val)
{
	Validate();
	char buffer[longest_printf];
	int length = sprintf( buffer, "%08X", int( val ) );
	AssertMsg( length + 1 < longest_printf, "ostream printf buffer overflow" );
	_output->OutputChars(buffer,length);
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const ostreamModifier /*val*/ )
{
	Validate();
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( ostream& (*f)( ostream& ) )
{
	Validate();
	return f( *this );
}

// ------------------------------------------------------------------------

ostream&
ostream::operator << ( const stream_endl&  )
{
	Validate();
	operator<<("\r\n");
	_output->Flush();
	return *this;
}

// ------------------------------------------------------------------------

ostream&
ostream::operator = ( const ostream& other )
{
	Validate();
	other.Validate();
	_output->DecUseCount();
	if(_output->UseCount() == 0)
		MEMORY_DELETE(HALDmalloc, _output, StreamOutput );

	_output = other._output;
	_output->IncUseCount();
	return *this;
	Validate();
}

//=============================================================================

StreamInput::~StreamInput()
{
	Validate();
	assert(useCount == 0);
}

//=============================================================================

istream::istream( StreamInput& input )
{
	assert(ValidPtr(&input));
//	printf("constructing ostream at address %p from StreamOutput at address %p\n",this,&output);
	_input = &input;
	_input->IncUseCount();
	Validate();
}

//-----------------------------------------------------------------------------

istream::~istream( void )
{
	Validate();
	_input->DecUseCount();
	if(!_input->UseCount())
		MEMORY_DELETE(HALDmalloc, _input, StreamInput );
}

// ------------------------------------------------------------------------

char
lower(char in)
{
	if(in >= 'A' && in <= 'Z')
		return(in + 'a'-'A');
	return(in);
}

//-----------------------------------------------------------------------------

istream&
istream::operator >> ( long val )
{
	int base = GetBase();
	char ch;
	val = 0;
	do
	{
		_input->InputChars(&ch,1);
		val *= base;
		if(ch >= '0' && ch <= '9')
			val += ch-'0';
		else
		{
			val += (lower(ch)-'a')+10;
			assert(lower(ch)-'a' <= base);
		}
	}
	while(ch != '\n' && ch != ' ');
	return (*this);
}

//-----------------------------------------------------------------------------

int
istream::get(char* buffer, const int size, const char delim)
{
	int len = 0;
	char input;

	assert(ValidPtr(_input));
	do
	{
		_input->InputChars(&input,1);
		*buffer++ = input;
		++len;
	}
	while(input != delim && size > len);

	if(delim == 10 && *(buffer-2) == 13)
		buffer -= 1;
	*--buffer = '\0';
	return len;
}

//-----------------------------------------------------------------------------

istream&
istream::getline( char * _b, int _lim, char _delim)
{
//	lock();
//	_fGline++;
	get( _b, _lim, (int)(unsigned char)_delim);
//	unlock();
	return *this;
}
//=============================================================================
//=============================================================================


//=============================================================================
#endif			// DO_IOSTREAMS
//-----------------------------------------------------------------------------
