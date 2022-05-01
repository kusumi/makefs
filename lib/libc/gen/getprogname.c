#include <sys/compat.h> /* __progname */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

//#include "namespace.h"
#include <stdlib.h>
//#include "un-namespace.h"

//#include "libc_private.h"

//__weak_reference(_getprogname, getprogname);

const char *
getprogname(void)
{

	return (__progname);
}
