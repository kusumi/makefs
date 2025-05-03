//#include "namespace.h"
//#include <stdlib.h>
//#include "un-namespace.h"

//#include "libc_private.h"

//__weak_reference(_getprogname, getprogname);

extern const char *__progname;

const char *
getprogname(void)
{

	return (__progname);
}
