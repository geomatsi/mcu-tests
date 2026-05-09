#include "stdlib.h"

size_t strlen(const char *s)
{
	size_t len = 0;
	char *ptr = (char *) s;

	while (*ptr++ != '\0') {
		len += 1;
	}

	return len;
}
