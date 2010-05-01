//=============================================================================
// scanf.c:
//=============================================================================

#define	_SCANF_C
#define	_SYS_NOCHECK_DIRECT_STD	1
#include <pigsys/pigsys.hp>

#undef va_arg
#define va_arg(AP, TYPE)						\
 (AP = ((char *) (AP)) + __va_rounded_size (TYPE),			\
  *((TYPE *) ((char *) (AP) - __va_rounded_size (TYPE))))


//=============================================================================
// A quick & dirty implementation of scanf (since not present on 3DO)

static int			_scanf(sys_FILE* fp, const char *fmt, va_list* argp);

//==================== beg for sscanf ========================
static const char*	_sscanf_buf = NULL;
static const char*	_sscanf_p	= NULL;
static int			_lastc		= '\0';

static void
_init_sgetc(const char* buf)
{
	assert( _sscanf_buf == NULL );
	assert( _sscanf_p == NULL );
	assert( _lastc == '\0' );
	assert( buf != NULL );
	_sscanf_buf = buf;
	_sscanf_p = buf-1;
}

//=============================================================================

static int
_sgetc(void)
{
	int		ret = EOF;

	assert( _sscanf_buf != NULL );
	assert( _sscanf_p != NULL );
	if ( _lastc != '\0' ) {
		assert( _sscanf_p >= _sscanf_buf );
		ret = _lastc;
		_lastc = '\0';
	} else {
		if ( *(_sscanf_p+1) != '\0' ) {
			ret = *(++_sscanf_p);
			if ( ret == '\0' ) {
				ret = EOF;
			}
		}
	}
	return ret;
}

//=============================================================================

static int
_sungetc(int c)
{
	assert( _sscanf_buf != NULL );
	assert( _sscanf_p != NULL );
	assert( c != '\0' );
	assert( _lastc == '\0' );
	assert( _sscanf_p >= _sscanf_buf );
	assert( *_sscanf_p != '\0' || c == EOF );
	return (_lastc = c);
}

static void
_close_sgetc(void)
{
	assert( _sscanf_buf != NULL );
	_sscanf_p = _sscanf_buf = (const char*)NULL;
	_lastc = '\0';
}
//==================== end for sscanf ========================



#if 0
int
scanf(const char *fmt, ...)
{
	int		ret;
	va_list	arg;

        assert( ValidPtr(stdin) );
	assert( fmt != NULL );
	assert( _sscanf_buf == NULL );
	va_start(arg, fmt);
	ret = _scanf(stdin, fmt, &arg);
	va_end(arg);
	return ret;
}
#endif

#if 0
int
fscanf(sys_FILE *fp, const char *fmt, ...)
{
	int		ret;
	va_list	arg;

        assert( ValidPtr(fp) );
	assert( fmt != NULL );
	assert( _sscanf_buf == NULL );
	va_start(arg, fmt);
	ret = _scanf(fp, fmt, &arg);
	va_end(arg);
	return ret;
}
#endif


int
sscanf(const char* buf, const char *fmt, ...)
{
	int		ret;
	va_list	arg;

	assert( buf != NULL );
	assert( fmt != NULL );
	_init_sgetc(buf);
	va_start(arg, fmt);
	ret = _scanf((sys_FILE*)NULL, fmt, &arg);
	va_end(arg);
	_close_sgetc();
	return ret;
}


static void	clearset(int);
static void	invertset(void);
static void	setbit(int);
static int	testbit(int);

static struct format {
	unsigned 		suppress : 1;
	unsigned		haveWidth : 1;
	unsigned 		fetchBase : 1;
	unsigned		negate : 1;
	unsigned 		valid : 1;
	unsigned 		unsigned_ : 1;
	unsigned		dot : 1;
	unsigned 		hSize : 1;
	unsigned 		lSize : 1;
	unsigned 		LSize : 1;
	int				fieldWidth;
} default_format;

static char scanset[32];

//==============================================================================

int
_scanf(sys_FILE* fp, const char *fmt, va_list* argp)
{
	int nassigned = 0;
	int nconverted = 0;
	int nread = 0;
	int overflow;
	int state;
	unsigned char first;
	register short c;
	register short base;
	register short digit;
	register long result;
	register char* s = NULL;
	struct format F;

	assert(fp == NULL);			// there is no fgetc on the psx

        assert( (_sscanf_buf == NULL && ValidPtr(fp)) ||
			(_sscanf_buf != NULL && fp == NULL) );
#define	_SCANF_GETC(_fp)		(short)((_sgetc()))
#define	_SCANF_UNGETC(_c,_fp)	(short)((_sungetc(_c)))
//#define	_SCANF_GETC(_fp)		(short)((_fp ? fgetc(_fp) : _sgetc()))
//#define	_SCANF_UNGETC(_c,_fp)	(short)((_fp ? ungetc(_c,_fp) : _sungetc(_c)))
	for (c = *fmt; c; c = *++fmt)
	{
		if (c != '%')
			goto match1;
		F = default_format;

			//  check for assignment-suppression flag

		c = *++fmt;
		if (c == '*')
		{
			F.suppress = true;
			c = *++fmt;
		}

			//  decode field width

		if (isdigit(c))
		{
			F.haveWidth = true;
			do
			{
				F.fieldWidth = (10 * F.fieldWidth) + (c - '0');
				c = *++fmt;
			} while (isdigit(c));
			if (F.fieldWidth <= 0)
				goto done;
		}

			//  determine appropriate conversion

conv:
		switch (c)
		{
				//  'h' size modifier
			case 'h':
				F.hSize = true;
				c = *++fmt;
				goto conv;

				//  'l' size modifier
			case 'l':
				F.lSize = true;
				c = *++fmt;
				goto conv;

				//  'L' size modifier
			case 'L':
				F.LSize = true;
				c = *++fmt;
				goto conv;

				//  '?' base modifier
			case '?':
				F.fetchBase = true;
				c = *++fmt;
				goto conv;

				//  decimal (signed)
			case 'd':
				base = 10;
				goto conv_signed;

				//  integer (signed)
			case 'i':
				base = 0;
				goto conv_signed;

				//  octal (unsigned)
			case 'o':
				base = 8;
				goto conv_unsigned;

				//  decimal (unsigned)
			case 'u':
				base = 10;
				goto conv_unsigned;

				//  hexadecimal (unsigned)
			case 'p':
				F.lSize = true;
				// ...
			case 'x':
			case 'X':
				base = 16;
				goto conv_unsigned;

				//  string
			case 's':
				do
				{
					c = _SCANF_GETC(fp), ++nread;
				} while (isspace(c));
				clearset(1);
				goto string;

				//  scanset
			case '[':
				c = *++fmt;
				if (c == '^')
				{
					F.negate = true;
					c = *++fmt;
				}
				clearset(0);
				for (;;)
				{
					if (c == '\0')
						goto done;
					setbit((unsigned char) c);
					c = *++fmt;
					if (c == ']')
						break;
					if (c == '-' && fmt[1] != ']' && ((unsigned char)fmt[1]) >= (first = fmt[-1]))
					{
						for (c = *++fmt; first != c; setbit(first++))
							;
					}
				}
				if (F.negate)
					invertset();
				c = _SCANF_GETC(fp), ++nread;
				goto string;

				//  character
			case 'c':
				if (!F.haveWidth)
					F.fieldWidth = 1;

				if (!F.suppress)
					s = va_arg(*argp, char *);
				while (F.fieldWidth-- > 0) {
					if ((c = _SCANF_GETC(fp)) == EOF)
						goto done;
					if (!F.suppress)
						*s++ = c;
					++nread;
				}
				if (!F.suppress)
					++nassigned;
				++nconverted;
				continue;

				//  store # bytes read so far
			case 'n':
				result = nread;
				if (!F.suppress)
					--nassigned;
				goto assign;

				//  others
			default:
				if (c != '%')
					goto done;
			match1:
				if (isspace(c))
				{
					do
					{
						c = _SCANF_GETC(fp), ++nread;
					} while (isspace(c));
					_SCANF_UNGETC(c, fp), --nread;
				}
				else
				{
					if ((c = _SCANF_GETC(fp)) != (unsigned char) *fmt)
					{
						_SCANF_UNGETC(c, fp);
						goto done;
					}
					++nread;
				}
				continue;
		}
		//NOTREACHED

			//  string scanner
string:
		if (!F.haveWidth)
			F.fieldWidth = INT_MAX;
		if (!F.suppress)
			s = va_arg(*argp, char *);
		for (; c != EOF; c = _SCANF_GETC(fp), ++nread)
		{
			--F.fieldWidth;
			if (!testbit(c))
				break;
			F.valid = true;
			if (!F.suppress)
				*s++ = c;
			if (F.fieldWidth == 0)
				goto endstring;
		}
		_SCANF_UNGETC(c, fp), --nread;
endstring:
		if (!F.valid)
			goto done;
		if (!F.suppress)
		{
			*s = 0;
			++nassigned;
		}
		++nconverted;
		continue;

			//  integer conversions

conv_unsigned:
		F.unsigned_ = true;
conv_signed:
		if (F.fetchBase)
			base = va_arg(*argp, int);
		state = 0;

			//  numeric scanner

		result = 0;
		do
		{
			c = _SCANF_GETC(fp), ++nread;
		} while (isspace(c));
		if (!F.haveWidth)
			F.fieldWidth = INT_MAX;
		overflow = false;
		for (; c != EOF; c = _SCANF_GETC(fp), ++nread)
		{
			--F.fieldWidth;
			switch (state)
			{

				//  (integer) start state
				case 0:
					state = 1;
					if (c == '-')
					{
						F.negate = true;
						break;
					}
					if (c == '+')
						break;
					// ...

				//  (integer) look for leading '0'
				case 1:
					state = 3;
					if (c == '0')
					{
						F.valid = true;
						if (F.fieldWidth)
						{
							if (base == 0)
							{
								base = 8;
								state = 2;
							}
							else if (base == 16)
								state = 2;
						}
						break;
					}
					goto state3;

				//  (integer) look for leading '0x'

				case 2:
					state = 3;
					if (c == 'x' || c == 'X')
					{
						base = 16;
						F.valid = false;
						break;
					}
					// ...

				//  (integer) process each digit

				case 3:
				state3:
					digit = c;
					if (digit >= '0' && digit <= '9')
						digit -= '0';
					else if (digit >= 'A' && digit <= 'Z')
						digit -= 'A' - 10;
					else if (digit >= 'a' && digit <= 'z')
						digit -= 'a' - 10;
					else
						goto pushback;
					if (base == 0)
						base = 10;
					if (digit >= base)
						goto pushback;
					result *= base, result += digit;	// we don't check for overflow here
					F.valid = true;
					break;
			}

				//  get next character
			if (F.fieldWidth == 0)
				goto endscan;
		}

			//  push back last character read

pushback:
		_SCANF_UNGETC(c, fp), --nread;
endscan:
		if (!F.valid)
			goto done;

			//  set sign of result

		if (F.negate && result)
		{
			result = -result;
			if (F.unsigned_ || result > 0)
				overflow = true;
		}
		else if (!F.unsigned_ && result < 0)
			overflow = true;

			//  see if result fits

		if (F.hSize)
		{
			if (F.unsigned_)
			{
				if ((unsigned short) result != result)
					overflow = true;
			}
			else
			{
				if ((short) result != result)
					overflow = true;
			}
		}
		else if (!F.lSize)
		{
			if (F.unsigned_)
			{
				if ((unsigned int) result != result)
					overflow = true;
			}
			else
			{
				if ((int) result != result)
					overflow = true;
			}
		}

			//  report overflow

		if (overflow)
		{
			if (F.unsigned_)
				result = 0;
			else if (F.hSize)
				result = SHRT_MIN;
			else if (F.lSize)
				result = LONG_MIN;
			else
				result = INT_MIN;
			if (!F.negate)
				result = ~result;
		}

			//  assign result

assign:
		if (!F.suppress)
		{
			s = va_arg(*argp, char*); //void *); ARMcc complains here (illegal in C++
			if (F.lSize)
				* (long *) s = result;
			else if (F.hSize)
				* (short *) s = (short)result;
			else
				* (int *) s = (int)result;
			++nassigned;
		}
		++nconverted;
	}

		//  all done!

done:
	if (nconverted == 0 && c == EOF)
		return(EOF);
	return(nassigned);
}


//==============================================================================
// ---------- scanset primitives ----------

//  clearset - initialize set to "nothing" or "everything but whitespace"

static void
clearset(int whitespace)
{
	(void)memset(scanset, 0, sizeof(scanset));
	if (whitespace) {
		scanset[1] = 0x3E;	//  \t \n \v \f \r
		scanset[4] = 0x01;	//  space
		invertset();
	}
}


//  setbit - add character to scan set

static void
setbit(int c)
{
	scanset[c >> 3] |= (1 << (c & 7));
}


//  invertset - complement scan set

static void
invertset(void)
{
	long*	p = (long*)(void*)scanset;
	p[0] = ~p[0];
	p[1] = ~p[1];
	p[2] = ~p[2];
	p[3] = ~p[3];
	p[4] = ~p[4];
	p[5] = ~p[5];
	p[6] = ~p[6];
	p[7] = ~p[7];
}


//  testbit - see if character is in scanset

static int
testbit(int c)
{
	return(scanset[c >> 3] & (1 << (c & 7)));
}
