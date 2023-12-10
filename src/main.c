#include "command_parser.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	parse_command(argc, argv);

	#undef DEBUG
	#ifdef DEBUG
	print_arguments();
	#endif

	return 0;
}
