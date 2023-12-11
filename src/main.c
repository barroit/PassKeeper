#include "command_parser.h"
#include "prompt.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	app_name = argv[0];

	if (argc < 2)
		usage(USAGE_ALL);

	parse_command(argc, argv);

	validate_field();

	#ifdef DEBUG
	print_field();
	#endif

	//

	return 0;
}
