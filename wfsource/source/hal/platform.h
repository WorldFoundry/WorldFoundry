
#if defined( __DOS__ )
#	include <hal/dos/platform.h>
#elif defined( __WIN__ )
#	include <hal/win/platform.h>
#elif defined( __PSX__ )
#	include <hal/psx/platform.h>
#elif defined( __LINUX__ )
#	include <hal/linux/platform.h>
#else
#	error Unsupported platform
#endif
