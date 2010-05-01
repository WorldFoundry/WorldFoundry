//=============================================================================
// toolstub.cc:
//=============================================================================

#include <hal/hal.h>
#include <memory/lmalloc.hp>

//=============================================================================

class InitToolStub
{
public:
	InitToolStub();
	~InitToolStub();
private:
	void* _memory;
};

//=============================================================================
// globals

LMalloc* _HALLmalloc;
InitToolStub _globalToolStub;
int32 cbHalLmalloc = 3500000;

//=============================================================================

#undef new

InitToolStub::InitToolStub()
{
	_memory = new  char[cbHalLmalloc];
	_HALLmalloc = new LMalloc(_memory,cbHalLmalloc, MEMORY_NAMED("HALLmalloc"));
}

//=============================================================================

InitToolStub::~InitToolStub()
{
	delete _HALLmalloc;
	delete [] (char*)_memory;
}

//=============================================================================

void
FatalError( const char* string )
{
	printf("Fatal Error: %s",string);
	exit(1);
}

//=============================================================================
