#include "orm.hpp"

struct ReadPredicate
{
	std::string name;
	bool verbose{ false };
};

void setup_read_subcommand(CLI::App &pk)
{
	auto field = std::make_shared<ReadPredicate>();
	auto *R = pk.add_subcommand("read", "read record(s) by name")->alias("R");

	R->add_option("-n,--name,name", field->name, "name of site");
	R->add_flag("-v,--verbose", field->verbose, "show more information")->multi_option_policy(CLI::MultiOptionPolicy::Throw);

	R->callback(
			[field]()
			{
				ORMBridge orm{ ".secret.db" };
				std::cout << orm.select_record(field->name, field->verbose);
			}
	);
}
