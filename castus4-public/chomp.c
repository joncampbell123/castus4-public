
#include <string.h>

/* C/C++ equivalent of Perl's chomp function */
void castus4public_chomp(char *s) {
	char *orig_s = s;
	size_t l = strlen(s);
	if (l == 0) return;
	s += l - 1;
	while (s > orig_s && (*s == '\n' || *s == '\r')) *s-- = 0;
}

