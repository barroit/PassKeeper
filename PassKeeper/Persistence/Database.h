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
        Connector(const std::string &);

        bool connect(const std::string &);

        bool select_record(std::stringstream &, const std::string &, bool);

        bool create_record(
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &
        );

        bool update_record(
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &,
                const std::string &
        );

        bool delete_record(const std::string &);

        bool count_record(int &);

        ~Connector();

    private:
        sqlite3 *p_db;
        std::string table_name;
    };
}
#endif
