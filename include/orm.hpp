#pragma once

#include "pch.hpp"

struct FCreate
{
	std::string site_name;
	std::string site_url;
	std::string username;
	std::string password;
	std::string auth_text;
	std::string recovery_code;
	std::string comment;
};

struct FUpdate : FCreate
{
	int id{ -1 };
};

class ORMBridge
{
public:
	explicit ORMBridge(const char *db_url);

	std::string select_record(const std::string &name, bool verbose);

	int insert_record(
			const std::string &site_name,
			const std::string &site_url,
			const std::string &username,
			const std::string &password,
			const std::string &auth_text,
			const std::string &recovery_code,
			const std::string &comment
	);

	int update_record(int id, const std::map<std::string, std::string> &args);

	int delete_record(int id);

	~ORMBridge();

private:
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int rc;
};
