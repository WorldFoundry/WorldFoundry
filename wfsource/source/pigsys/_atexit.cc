//=============================================================================
// atexit.c:
//=============================================================================

#define	__ATEXIT_C
#define	_SYS_NOCHECK_DIRECT_STD	1
#include <pigsys/_atexit.h>

//=============================================================================

static unsigned int						_num_atFuns = 0;
static sys_atexitFunctionPtr	_atFuns[SYS_MAC_ATEXIT_FUNCS];

//=============================================================================

int
sys_atexit(sys_atexitFunctionPtr atfunc)
{
	assert( atfunc != NULL );
	assert( sizeof(_atFuns)/sizeof(*_atFuns) == SYS_MAC_ATEXIT_FUNCS );
	assert( _num_atFuns <= sizeof(_atFuns)/sizeof(*_atFuns) );
	if ( _num_atFuns >= SYS_MAC_ATEXIT_FUNCS ) {
		Fail("Too many calls to sys_atexit, please increase SYS_MAC_ATEXIT_FUNCS (" << SYS_MAC_ATEXIT_FUNCS << ")\n");
		return -1;
	}
	_atFuns[_num_atFuns++] = atfunc;
	return 0;
}

//=============================================================================

void
_sys_call_atexit_funs(int code)
{
	int		i;
    printf("sys_call_atexit function called, num_atFuns = %d\n",_num_atFuns);

	assert( sizeof(_atFuns)/sizeof(*_atFuns) == SYS_MAC_ATEXIT_FUNCS );
	assert( _num_atFuns <= sizeof(_atFuns)/sizeof(*_atFuns) );
	for ( i = _num_atFuns-1; i >= 0; i-- ) 
    {
		assert( _atFuns[i] != NULL );
        printf("Calling atexit function at address %p\n",*_atFuns[i]);
		(*_atFuns[i])(code);
	}
	_num_atFuns = 0;			// Only ever call these once
}

//=============================================================================
