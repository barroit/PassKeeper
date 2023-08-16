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
        Connector(std::string );

        bool connect(const std::string &);

        bool select_record(std::stringstream &, const std::string &, bool);

        bool insert_record(
                int &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &
        );

        bool update_record(
                int &,
                int,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &
        );

        bool delete_record(int &, int);

        bool count_record(int &);

        ~Connector();

    private:
        sqlite3 *p_db;
        sqlite3_stmt *p_stmt;
        std::string table_name;
    };
}
#endif
