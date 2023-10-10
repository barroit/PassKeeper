#include "orm.hpp"
#include "pkerr.hpp"

#define THROW_ON_FAILED(condition, message)																				\
	if (condition)																										\
	{																													\
		throw ORMException{ std::string{ message } + ": " + sqlite3_errmsg(db) };										\
	}

#define BIND_TEXT(index, value)																							\
	rc = sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);											\
	THROW_ON_FAILED(rc != SQLITE_OK, "failed to bind text")

#define BIND_INT(index, value)																							\
	rc = sqlite3_bind_int(stmt, index, value);																			\
	THROW_ON_FAILED(rc != SQLITE_OK, "failed to bind int")

#define PREPARE_STATEMENT(value)																						\
	rc = sqlite3_prepare_v2(db, value, -1, &stmt, nullptr);																\
	THROW_ON_FAILED(rc != SQLITE_OK, "failed to prepare statement")

#define EXECUTE_STATEMENT(message)																						\
	rc = sqlite3_step(stmt);																							\
	THROW_ON_FAILED(rc != SQLITE_DONE, message)

#define OUT_ROW(key, value)																								\
	std::left << std::setw(20) << key << value << std::endl

ORMBridge::ORMBridge()
		: db{ nullptr }, stmt{ nullptr }
{
	auto db_url = ORMBridge::get_secret_location();

	if (!std::filesystem::exists(db_url))
	{
		throw ORMException{ std::string{ "db file not exists: " }.append(db_url) };
	}

	rc = sqlite3_open(db_url.c_str(), &db);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		throw ORMException{ std::string{ "failed to open database: " }.append(db_url) };
	}
}

ORMBridge::~ORMBridge()
{
	sqlite3_close(db);
	sqlite3_finalize(stmt);
}

std::string ORMBridge::select_record(const std::string &name, bool verbose)
{
	PREPARE_STATEMENT("SELECT * FROM password WHERE site_name LIKE ?")

	std::string match_pattern = name + "%";
	BIND_TEXT(1, match_pattern)

	int index = 0;
	std::stringstream ss;
	const unsigned char none{ 0x0 };

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		THROW_ON_FAILED(rc != SQLITE_ROW && rc != SQLITE_DONE, "failed to evaluate statement: ")

		const unsigned char *site_name = sqlite3_column_text(stmt, 1);
		const unsigned char *username = sqlite3_column_text(stmt, 3);
		const unsigned char *password = sqlite3_column_text(stmt, 4);

		if (!verbose)
		{
			if (!index++) ss << std::left << std::setw(27) << "site_name" << std::setw(32) << "username" << "password" << std::endl;
			ss << std::left << std::setw(27) << site_name << std::setw(32) << username << password << std::endl;
		}
		else
		{
			int id = sqlite3_column_int(stmt, 0);
			const unsigned char *site_url = sqlite3_column_text(stmt, 2);
			const unsigned char *auth_text = sqlite3_column_text(stmt, 5);
			const unsigned char *recovery_code = sqlite3_column_text(stmt, 6);
			const unsigned char *comment = sqlite3_column_text(stmt, 7);
			const unsigned char *sqltime = sqlite3_column_text(stmt, 8);
			const unsigned char *updatetime = sqlite3_column_text(stmt, 9);

			if (index++) ss << std::endl;
			ss << OUT_ROW("id->", id)
			   << OUT_ROW("site_name->", site_name)
			   << OUT_ROW("site_url->", site_url)
			   << OUT_ROW("username->", username)
			   << OUT_ROW("password->", password)
			   << OUT_ROW("auth_text->", auth_text)
			   << OUT_ROW("recovery_code->", recovery_code)
			   << OUT_ROW("comment->", comment)
			   << OUT_ROW("sqltime->", sqltime)
			   << OUT_ROW("updatetime->", (updatetime == nullptr ? &none : updatetime));
		}
	}

	return ss.str();
}

int ORMBridge::insert_record(const std::string &site_name, const std::string &site_url, const std::string &username, const std::string &password, const std::string &auth_text, const std::string &recovery_code, const std::string &comment)
{
	PREPARE_STATEMENT("INSERT INTO password (site_name, site_url, username, password, auth_text, recovery_code, comment) VALUES (?, ?, ?, ?, ?, ?, ?)")

	BIND_TEXT(1, site_name)
	BIND_TEXT(2, site_url)
	BIND_TEXT(3, username)
	BIND_TEXT(4, password)
	BIND_TEXT(5, auth_text)
	BIND_TEXT(6, recovery_code)
	BIND_TEXT(7, comment)

	EXECUTE_STATEMENT("failed to create record")

	return sqlite3_changes(db);
}

int ORMBridge::update_record(int id, const std::map<std::string, std::string> &args)
{
	if (args.empty()) return 0;

	std::stringstream ss;
	int index = 0;

	for (const auto &[ key, _ ]: args)
	{
		if (index++) ss << ", ";
		ss << key << " = ?";
	}

	std::string statement = "UPDATE password SET " + ss.str() + " WHERE id = ?";
	PREPARE_STATEMENT(statement.c_str())

	index = 1;
	for (const auto &[ _, value ]: args)
	{
		BIND_TEXT(index++, value)
	}

	BIND_INT(index, id)

	EXECUTE_STATEMENT("failed to update record")

	return sqlite3_changes(db);
}

int ORMBridge::delete_record(int id)
{
	PREPARE_STATEMENT("DELETE FROM password WHERE id = ?")

	BIND_INT(1, id)

	EXECUTE_STATEMENT("failed to delete record")

	return sqlite3_changes(db);
}

std::string ORMBridge::get_secret_location()
{
	std::string onedrive_path;
	std::string onedrive_key{ "onedrive" };

	for (char **next_env = environ; *next_env != nullptr; ++next_env)
	{
		std::string env{ *next_env };

		std::string::size_type pos = env.find('=');
		if (pos == std::string::npos)
		{
			continue;
		}

		std::string key = env.substr(0, pos);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		if (key == onedrive_key)
		{
			onedrive_path = env.substr(pos + 1);
		}
	}

	std::string local_db{ ".secret.db" };

	if (onedrive_path.empty()) return local_db;

	if (std::filesystem::exists(onedrive_path) && std::filesystem::is_directory(onedrive_path))
	{
		std::string remote_db = onedrive_path + "/Secret/.secret.db";
		if (std::filesystem::exists(remote_db)) return remote_db;
	}

	return local_db;
}
