#include "orm.hpp"

#define ADD_FIELD(text)																									\
	if (text->count()) args.insert({ #text, field->text });

void setup_update_subcommand(CLI::App &pk)
{
	auto field = std::make_shared<FUpdate>();
	auto U = pk.add_subcommand("update", "update a record by id")->alias("U");

	U->add_option("--id,id", field->id)->mandatory()->check(CLI::Range(0, INT_MAX));
	auto site_name = U->add_option("--site_name", field->site_name);
	auto site_url = U->add_option("--site_url", field->site_url);
	auto username = U->add_option("--username", field->username);
	auto password = U->add_option("--password", field->password);
	auto auth_text = U->add_option("--auth_text", field->auth_text, "multiple values separated with comma");
	auto recovery_code = U->add_option("--recovery_code", field->recovery_code, "multiple values separated with comma");
	auto comment = U->add_option("--comment", field->comment);

	U->callback(
			[=]()
			{
				std::map<std::string, std::string> args;
				ADD_FIELD(site_name)
				ADD_FIELD(site_url)
				ADD_FIELD(username)
				ADD_FIELD(password)
				ADD_FIELD(site_name)
				ADD_FIELD(auth_text)
				ADD_FIELD(recovery_code)
				ADD_FIELD(comment)

				ORMBridge orm{ ".secret.db" };
				int r = orm.update_record(field->id, args);
				std::cout << "Query OK, " << r << " row affected" << std::endl;
			}
	);
}
