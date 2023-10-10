#include "orm.hpp"

struct DeletePredicate
{
	int id;
};

void setup_delete_subcommand(CLI::App &pk)
{
	auto field = std::make_shared<DeletePredicate>();
	auto D = pk.add_subcommand("delete", "delete a record by id")->alias("D");

	D->add_option("--id,id", field->id)->mandatory()->check(CLI::Range(0, INT_MAX));

	D->callback(
			[field]()
			{
				ORMBridge orm;
				int r = orm.delete_record(field->id);
				std::cout << "Query OK, " << r << " row affected" << std::endl;
			}
	);
}
