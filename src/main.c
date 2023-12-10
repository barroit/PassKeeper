#include "command_parser.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	struct argument argument = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, 0, 0 };

	parse_command(argc, argv, &argument);

	#ifdef DEBUG
	print_arguments(&argument);
	#endif

	return 0;
}
