#define	__CF_WIN_C
#define	_SYS_NOCHECK_DIRECT_STD	1
#include <pigsys/pigsys.hp>

void
_win32_init(void)
{
}


// rindex doesn't exist under windows
char *rindex(char *s, int c)
{
	char* end = s+strlen(s);
	while(end > s)
	{
		if(*end == c)
			return end;
		end--;
	}
	return NULL;
}

