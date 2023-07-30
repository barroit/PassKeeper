#ifndef PASSKEEPER_DATABASE_H
#define PASSKEEPER_DATABASE_H

#include <sstream>
#include <sqlite3.h>

namespace Database
{
    using namespace std;

    class Connector
    {
    public:
        Connector(const string &);

        bool connect(const string &file_path);

        bool select_record(stringstream &, const string &);

        bool create_record(
                const string &,
                const string &,
                const string &,
                const string &,
                const string &,
                const string &,
                const string &
        );

        bool update_record(
                const string &,
                const string &,
                const string &,
                const string &,
                const string &,
                const string &,
                const string &,
                const string &
        );

        bool delete_record(const string &);

        ~Connector();

    private:
        sqlite3 *p_db;
        string table_name;
    };
}
#endif
