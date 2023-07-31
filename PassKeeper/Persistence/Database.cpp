#include "Database.h"

#include <iostream>
#include <iomanip>

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

bool Database::Connector::select_record(std::stringstream &s_stream, const std::string &site_name, bool optimize_display)
{
    string s_query = "SELECT * FROM " + table_name + " WHERE site_name LIKE ?";
    const char *psz_query = s_query.c_str();
    sqlite3_stmt *p_stmt;

    int rc = sqlite3_prepare_v2(p_db, psz_query, -1, &p_stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "Failed to execute statement: " << sqlite3_errmsg(p_db) << endl;
        return false;
    }

    string match_pattern = site_name + "%";
    sqlite3_bind_text(p_stmt, 1, match_pattern.c_str(), -1, SQLITE_STATIC);

    int c_times = 0;
    while ((rc = sqlite3_step(p_stmt)) == SQLITE_ROW)
    {
        if (rc != SQLITE_ROW && rc != SQLITE_DONE)
        {
            cerr << "Failed to evaluate statement: " << sqlite3_errmsg(p_db) << endl;
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

        if (optimize_display)
        {
            if (!c_times++) s_stream << left << setw(20) << "site_name" << setw(32) << "username" << setw(32) << "password" << endl;
            s_stream << left << setw(20) << p_site_name << setw(32) << p_username << setw(32) << p_password << endl;
        }
        else
        {
            if (c_times++) s_stream << endl;
            s_stream << "id->\t" << id << endl
                     << "site_name->\t" << p_site_name << endl
                     << "site_url->\t" << p_site_url << endl
                     << "username->\t" << p_username << endl
                     << "password->\t" << p_password << endl
                     << "auth_text->\t" << p_auth_text << endl
                     << "recovery_code->\t" << p_recovery_code << endl
                     << "comment->\t" << p_comment << endl;
        }
    }

    return true;
}

bool Database::Connector::create_record(
        const std::string &site_name,
        const std::string &site_url,
        const std::string &username,
        const std::string &password,
        const std::string &auth_text,
        const std::string &recovery_code,
        const std::string &comment
)
{
    return false;
}

bool Database::Connector::update_record(
        const std::string &id,
        const std::string &site_name,
        const std::string &site_url,
        const std::string &username,
        const std::string &password,
        const std::string &auth_text,
        const std::string &recovery_code,
        const std::string &comment
)
{
    return false;
}

bool Database::Connector::delete_record(const std::string &id)
{
    return false;
}

bool Database::Connector::count_record(int &count)
{
    char *pz_error_message = 0;

    string statement = "select count(*) from " + table_name;
    int rc = sqlite3_exec(
            p_db, statement.c_str(),
            [](void *count, int argc, char **argv, char **azColName) { return (*((int *) count) = atoi(argv[0]), 0); },
            &count, &pz_error_message
    );

    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << pz_error_message << endl;
        sqlite3_free(pz_error_message);
        return false;
    }

    return true;
}

