#include "rawnumop.h"

int main(UNUSED int argc, const char **argv)
{
	size_t hexlen;
	argv++;

	assert(*argv);
	hexlen = strlen(*argv);
	assert(!(hexlen % 2));

	fputs(bin2hex(hex2bin(strdup(*argv), hexlen), hexlen / 2), stdout);

	return 0;
}
