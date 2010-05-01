/*===============================================================================*/
/* vsprintf.c: needed by brender debug libraries */
/*===============================================================================*/

#include <cstddef>
#include <cstdio>
#include <cstdarg>
typedef unsigned long br_uint_32;


#ifdef DO_ASSERTIONS

/*
 * This function is a nasty hack, and should be replaced with a real
 * GNU C vsprintf() function.  This function is not guaranteed to work under all
 * conditions, and is just a quick hack.
 * Sony do not currently supply a vsprintf function with their development libraries.
 * The following function will not be supported in any way.  Use it at your own risk...
 */
int vsprintf(char *buffer, const char *format,...)
{
	va_list args;
	char *scan, ch, new_format[10], *new_format_ptr;
	int found;

	/*
	 * get first argument
	 */
	va_start(args, format);
	args = (va_list)(br_uint_32 *)*(br_uint_32 *)args;

	for(scan = buffer;;) {

	    	/*
		 * scan for end of format string or '%'
		 */
		for(ch = *format; (ch != '\0') && (ch != '%'); ch = *(++format), scan++)
			*scan = ch;

		if(ch == '\0')
			break;

		new_format_ptr = new_format;

		*new_format_ptr++ = *format++;

		/*
		 * extract format for next argument
		 */
		for(found = 0; !found;) {

		    	ch = *new_format_ptr++ = *format++;

			/*
			 * Not all of these format flags are supported by the sprintf function
			 * supplied by Sony as a part of their development libraries.
			 */
		    	switch(ch) {
				case 's': case 'd': case 'i': case 'x': case 'X': case 'p': case 'c':
				case 'u': case 'e': case 'E': case 'f': case 'F': case 'g':
				case 'G': case 'n': case 'o':

					found = 1;
					break;
			}
		}

		/*
		 * terminate new format string
		 */
		*new_format_ptr++ = '\0';

		/*
		 * use sprintf to format current argument
		 */
		sprintf(scan, new_format, *(br_uint_32 *)args);

		/*
		 * update buffer pointer
		 */
		scan += strlen(scan);

		/*
		 * next argument (hack!)
		 */
		args += 4;
	}

	va_end(args);

	/*
	 * return length of formatted string
	 */
	return strlen(buffer);
}

#else

int vsprintf(char *buffer, const char *format,...)
{
    	return 0;
}
#endif

/*===============================================================================*/
