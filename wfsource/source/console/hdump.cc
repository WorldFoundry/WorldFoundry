#include <pigsys/pigsys.hp>
#include <console/console.hp>
#if defined(__PSX__)
#	include <libetc.h>
#endif
static char
charTable[256] =
{
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
	'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
	'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
	'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.'
};

//============================================================================

ulong
HDumpLine( char* buffer, size_t bufferSize, Console& console )
{
	ulong i = bufferSize;
	char* dataBuffer = buffer;
	char ch;
	uint count = 0;
	while(i--)					// first print hex #'s
	 {
		console.print( "%02x",*dataBuffer++ );
		if(!((count+1) % 4))
			console.print(" ");
		count++;
	 }

	i = bufferSize;
	dataBuffer = buffer;
	while(i--)					// now print ascii
	 {
		ch = *dataBuffer++;
		console.print("%c",charTable[ch]);
	 }
	console.print("\n");
 	return(bufferSize);
}

//============================================================================

const unsigned int BYTES_PER_LINE = 12;
const unsigned int MAX_LINES = 24;
const size_t MAX_BUFFER_SIZE = BYTES_PER_LINE * MAX_LINES;

void
HDump( char* title, void* buffer, size_t bufferSize, Console& console )
{
	console.clear();
	console.print( "%s: addr $%x\n", title, buffer );

	if(bufferSize > MAX_BUFFER_SIZE)
		bufferSize = MAX_BUFFER_SIZE;

	ulong len;
	ulong offset = 0;
	char* dataBuffer = (char*)buffer;
	assert(buffer);

	printf("buffersize %d\n", bufferSize);
	while(bufferSize)
	 {
		console.print("%02x: ", offset);
		len = HDumpLine( dataBuffer, bufferSize >= BYTES_PER_LINE?BYTES_PER_LINE:bufferSize, console );
		dataBuffer += len;
		offset += len;
		bufferSize -= len;
	 }
	console.print("Press a button to continue");
	console.flush();

#if defined( __PSX__ )
	PadInit(0);
	while(!(PadRead(0) & PADstart))
		;
	while((PadRead(0) & PADstart))
		;
#endif
}
