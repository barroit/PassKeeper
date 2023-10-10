#include "orm.hpp"
#include "pkerr.hpp"

void setup_create_subcommand(CLI::App &pk)
{
	auto field = std::make_shared<FCreate>();
	auto C = pk.add_subcommand("create", "create a record")->alias("C");

	C->add_option("--site_name", field->site_name)->mandatory();
	C->add_option("--site_url", field->site_url);
	C->add_option("--username", field->username);
	C->add_option("--password", field->password);
	C->add_option("--auth_text", field->auth_text, "multiple values separated with comma");
	C->add_option("--recovery_code", field->recovery_code, "multiple values separated with comma");
	C->add_option("--comment", field->comment);

	C->callback(
			[field]()
			{
				if (field->username.empty() & field->password.empty())
				{
					throw ValidateException{ std::string{ "either username or password must be set" }};
				}

				ORMBridge orm;
				int r = orm.insert_record(field->site_name, field->site_url, field->username, field->password, field->auth_text, field->recovery_code, field->comment);
				std::cout << "Query OK, " << r << " row affected" << std::endl;
			}
	);
}
