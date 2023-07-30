#include "Database.h"

#include <iostream>

Database::Connector::Connector(const std::string &table_name)
        : p_db(nullptr), table_name(table_name)
{
}

Database::Connector::~Connector()
{
    sqlite3_close(p_db);
}

bool Database::Connector::connect(const std::string &file_path)
{
    if (sqlite3_open(file_path.c_str(), &p_db))
    {
        cerr << "Cannot open database: " << file_path << endl;
        return false;
    }
    return true;
}

bool Database::Connector::select_record(std::stringstream &s_stream, const std::string &site_name)
{
    string s_query = "SELECT * FROM " + table_name + " WHERE site_name LIKE ?";
    const char *psz_query = s_query.c_str();
    sqlite3_stmt *p_stmt;

    int rc = sqlite3_prepare_v2(p_db, psz_query, -1, &p_stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "Failed to execute statement: " << sqlite3_errmsg(p_db) << std::endl;
        return false;
    }

    string match_pattern = site_name + "%";
    sqlite3_bind_text(p_stmt, 1, match_pattern.c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(p_stmt)) == SQLITE_ROW)
    {
        if (rc != SQLITE_ROW && rc != SQLITE_DONE)
        {
            std::cerr << "Failed to evaluate statement: " << sqlite3_errmsg(p_db) << std::endl;
            return false;
        }

        int id = sqlite3_column_int(p_stmt, 0);
        const unsigned char *p_site_name = sqlite3_column_text(p_stmt, 1);
        const unsigned char *p_site_url = sqlite3_column_text(p_stmt, 2);
        const unsigned char *p_username = sqlite3_column_text(p_stmt, 3);
        const unsigned char *p_password = sqlite3_column_text(p_stmt, 4);
        const unsigned char *p_auth_text = sqlite3_column_text(p_stmt, 5);
        const unsigned char *p_recovery_code = sqlite3_column_text(p_stmt, 6);
        const unsigned char *p_comment = sqlite3_column_text(p_stmt, 7);

        s_stream << endl << "id-> " << id << endl
                 << "site_name-> " << p_site_name << endl
                 << "site_url-> " << p_site_url << endl
                 << "username-> " << p_username << endl
                 << "password-> " << p_password << endl
                 << "auth_text-> " << p_auth_text << endl
                 << "recovery_code-> " << p_recovery_code << endl
                 << "comment-> " << p_comment << endl;
    }

    return true;
}

bool Database::Connector::create_record(const string &, const string &, const string &, const string &, const string &, const string &, const string &)
{
    return false;
}

bool Database::Connector::update_record(const string &, const string &, const string &, const string &, const string &, const string &, const string &, const string &)
{
    return false;
}

bool Database::Connector::delete_record(const std::string &)
{
    return false;
}


