#include "command.hpp"

int main(int argc, char **argv)
{
	CLI::App pk{ "passkeeper, A command-line password manager leveraging SQLite3 for barroit." };
	pk.require_subcommand(1);

	setup_create_subcommand(pk);
	setup_read_subcommand(pk);
	setup_update_subcommand(pk);
	setup_delete_subcommand(pk);

	try
	{
		pk.parse(argc, argv);
	}
	catch (const CLI::ParseError &e)
	{
		if (e.get_name() == "CallForHelp")
		{
			return pk.exit(e);
		}
		std::cerr << e.what() << std::endl;

		if (pk.get_subcommands().empty())
		{
			std::cerr << "Available subcommands are: create(C), read(R), update(U), delete(D)" << std::endl << "Run with --help for more information" << std::endl;
		}

		return e.get_exit_code();
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}