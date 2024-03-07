#include "optparser.h"

static char *search_pattern;

static const char *usagestr[] = { "t" };

int cmd_count(int argc, const char **argv)
{
	struct option cmd_count_option[] = {
		OPTION_STRING(0, "search", &search_pattern, "pattern", "count for a particular site"),
		OPTION_END(),
	};

	search_pattern = "";
	argc = parse_option(argc, argv, cmd_count_option, usagestr, PARSER_STOP_AT_NON_OPTION);

	printf("%d, %s", argc, search_pattern);
	puts("");

	return 0;
}