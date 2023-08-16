#include "Database.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include "../Util/Validation.h"

#define EXIT_ON_ERROR(r, flag, message) if (r != flag) { cerr << message << ": " << sqlite3_errmsg(p_db) << endl; return false; }
#define INSERT_VALUE(variable) if (!variable.empty()) kv_pair.insert({ #variable, variable });

Database::Connector::Connector(std::string table_name)
        : p_db(nullptr), p_stmt(nullptr), table_name(std::move(table_name))
{
}

Database::Connector::~Connector()
{
    sqlite3_close(p_db);
    sqlite3_finalize(p_stmt);
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
    string statement = "SELECT * FROM " + table_name + " WHERE site_name LIKE ?";
    EXIT_ON_ERROR(sqlite3_prepare_v2(p_db, statement.c_str(), -1, &p_stmt, nullptr), SQLITE_OK, "Failed to prepare statement")

    // we need this, not the temporary string, to ensure the string not be destroyed before sqlite copies it.
    string match_pattern = site_name + "%";
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 1, match_pattern.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")

    int rc;
    int c_times = 0;
    while ((rc = sqlite3_step(p_stmt)) == SQLITE_ROW)
    {
        if (rc != SQLITE_ROW && rc != SQLITE_DONE)
        {
            cerr << "Failed to evaluate statement: " << sqlite3_errmsg(p_db) << endl;
            return false;
        }

        const unsigned char *p_site_name = sqlite3_column_text(p_stmt, 1);
        const unsigned char *p_username = sqlite3_column_text(p_stmt, 3);
        const unsigned char *p_password = sqlite3_column_text(p_stmt, 4);

        if (optimize_display)
        {
            if (!c_times++) s_stream << left << setw(24) << "site_name" << setw(32) << "username" << "password" << endl;
            s_stream << left << setw(24) << p_site_name << setw(32) << p_username << p_password << endl;
        }
        else
        {
            int id = sqlite3_column_int(p_stmt, 0);
            const unsigned char *p_site_url = sqlite3_column_text(p_stmt, 2);
            const unsigned char *p_auth_text = sqlite3_column_text(p_stmt, 5);
            const unsigned char *p_recovery_code = sqlite3_column_text(p_stmt, 6);
            const unsigned char *p_comment = sqlite3_column_text(p_stmt, 7);
            const unsigned char *p_sqltime = sqlite3_column_text(p_stmt, 8);
            const unsigned char *p_updatetime = sqlite3_column_text(p_stmt, 9);
            const unsigned char *none = new unsigned char(0x0);

            if (c_times++) s_stream << endl;
            s_stream << "id->\t\t" << id << endl
                     << "site_name->\t" << p_site_name << endl
                     << "site_url->\t" << p_site_url << endl
                     << "username->\t" << p_username << endl
                     << "password->\t" << p_password << endl
                     << "auth_text->\t" << p_auth_text << endl
                     << "recovery_code->\t" << p_recovery_code << endl
                     << "comment->\t" << p_comment << endl
                     << "sqltime->\t" << p_sqltime << endl
                     << "updatetime->\t" << (p_updatetime == nullptr ? none : p_updatetime) << endl;

            delete none;
        }
    }

    return true;
}

bool Database::Connector::insert_record(
        int &inserted,
        const std::string &site_name,
        const std::string &site_url,
        const std::string &username,
        const std::string &password,
        const std::string &auth_text,
        const std::string &recovery_code,
        const std::string &comment
)
{
    string statement =
            "INSERT INTO " + table_name + " (site_name, site_url, username, password, auth_text, recovery_code, comment) VALUES (?, ?, ?, ?, ?, ?, ?)";
    EXIT_ON_ERROR(sqlite3_prepare_v2(p_db, statement.c_str(), -1, &p_stmt, nullptr), SQLITE_OK, "Failed to prepare statement")

    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 1, site_name.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 2, site_url.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 3, Validation::is_blank(username) ? "NULL" : username.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 4, Validation::is_blank(password) ? "NULL" : password.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 5, auth_text.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 6, recovery_code.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, 7, comment.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")

    EXIT_ON_ERROR(sqlite3_step(p_stmt), SQLITE_DONE, "Failed to create record")

    inserted = sqlite3_changes(p_db);

    return true;
}

bool Database::Connector::update_record(
        int &updated,
        int id,
        const std::string &site_name,
        const std::string &site_url,
        const std::string &username,
        const std::string &password,
        const std::string &auth_text,
        const std::string &recovery_code,
        const std::string &comment
)
{
    stringstream s_stream;
    map<string, string> kv_pair;
    INSERT_VALUE(site_name)
    INSERT_VALUE(site_url)
    INSERT_VALUE(username)
    INSERT_VALUE(password)
    INSERT_VALUE(auth_text)
    INSERT_VALUE(recovery_code)
    INSERT_VALUE(comment)
    if (kv_pair.empty()) return false;

    int c_value = 0;
    for (const auto &pair: kv_pair)
    {
        if (c_value++) s_stream << ", ";
        s_stream << pair.first << " = ?";
    }

    string statement = "UPDATE " + table_name + " SET " + s_stream.str() + " WHERE id = ?";
    cout << statement << endl;
    EXIT_ON_ERROR(sqlite3_prepare_v2(p_db, statement.c_str(), -1, &p_stmt, nullptr), SQLITE_OK, "Failed to prepare statement")

    int c_column = 1;
    for (const auto &pair: kv_pair)
    {
        EXIT_ON_ERROR(sqlite3_bind_text(p_stmt, c_column++, pair.second.c_str(), -1, SQLITE_TRANSIENT), SQLITE_OK, "Failed to bind value")
    }
    EXIT_ON_ERROR(sqlite3_bind_int(p_stmt, c_column, id), SQLITE_OK, "Failed to bind value")

    EXIT_ON_ERROR(sqlite3_step(p_stmt), SQLITE_DONE, "Failed to update record")

    updated = sqlite3_changes(p_db);

    return true;
}

bool Database::Connector::delete_record(int &deleted, int id)
{
    string statement = "DELETE FROM " + table_name + " WHERE id = ?";
    EXIT_ON_ERROR(sqlite3_prepare_v2(p_db, statement.c_str(), -1, &p_stmt, nullptr), SQLITE_OK, "Failed to prepare statement")

    EXIT_ON_ERROR(sqlite3_bind_int(p_stmt, 1, id), SQLITE_OK, "Failed to bind value")

    EXIT_ON_ERROR(sqlite3_step(p_stmt), SQLITE_DONE, "Failed to delete record")

    // get affected row, should be one
    deleted = sqlite3_changes(p_db);

    return true;
}

bool Database::Connector::count_record(int &count)
{
    string statement = "select count(*) from " + table_name;
    char *pz_error_message = nullptr;

    int rc = sqlite3_exec(
            p_db, statement.c_str(),
            [](void *count, int argc, char **argv, char **azColName) { return *((int *) count) = atoi(argv[0]), 0; }, // NOLINT(*-err34-c)
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
