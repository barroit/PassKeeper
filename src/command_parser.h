#ifndef COMMAND_PARSER_HEADER
#define COMMAND_PARSER_HEADER

#define OPTION_ALIAS_ID			12160
#define OPTION_ALIAS_SITE_NAME		12161
#define OPTION_ALIAS_SITE_URL		12162
#define OPTION_ALIAS_USERNAME		12163
#define OPTION_ALIAS_PASSWORD		12164
#define OPTION_ALIAS_AUTH_TEXT		12165
#define OPTION_ALIAS_RECOVERY_CODE	12166
#define OPTION_ALIAS_COMMENT		12167

struct field
{
	char *site_name;
	char *site_url;
	char *username;
	char *password;
	char *auth_text;
	char *recovery_code;
	char *comment;

	char *db_file;

	int id;

	int is_verbose;
	int is_help;
	int is_version;

	int subcommand;
};

extern struct field field;

void parse_command(int argc, char **argv);

void resolve_option(int c);

int get_command(char **argv);

void resolve_argument(char *value, int position);

void validate_field(void);

int is_number(char *text);

void print_field(void);

#endif