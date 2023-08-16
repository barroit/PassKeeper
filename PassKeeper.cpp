#include <iostream>
#include "Command/ArgumentsParser.h"
#include "Persistence/Database.h"

//#define DEBUG_PASSKEEPER
#define EXIT_ON_ERROR(r) if (!r) return 1;

/*
CREATE TABLE password (
id INTEGER PRIMARY KEY AUTOINCREMENT,
site_name TEXT NOT NULL,
site_url TEXT,
username TEXT NOT NULL,
password TEXT NOT NULL,
auth_text TEXT,
recovery_code TEXT,
comment TEXT,
sqltime DATETIME DEFAULT (datetime('now', '+9 hours')),
updatetime DATETIME
);

CREATE TRIGGER update_time_trigger
AFTER UPDATE ON password
FOR EACH ROW
BEGIN
   UPDATE password SET updatetime = datetime('now', '+9 hours') WHERE id = OLD.id;
END;
*/

//#include <iostream>
//#include <string>
//#include <vector>
//#include <fstream>
//#include <cstdlib>
//
//using namespace std;
//
//vector<string> split(const string &s, char delimiter)
//{
//    vector<string> tokens;
//    string::size_type start = 0;
//    string::size_type end;
//
//    while ((end = s.find(delimiter, start)) != string::npos)
//    {
//        tokens.push_back(s.substr(start, end - start));
//        start = end + 1;
//    }
//
//    tokens.push_back(s.substr(start));
//    return tokens;
//}
//
//string get_or_empty(const char *field, const string &str)
//{
//    string res;
//    return str.empty() ? res : res + " --" + field + " \"" + str + "\"";
//}

int main(int argc, char *argv[])
{
//    ifstream file("password.txt");
//
//    vector<string> lines;
//    string line;
//    while (getline(file, line))
//    {
//        if (!line.empty() && line[line.size() - 1] == '\r')
//        {
//            line.erase(line.size() - 1);
//        }
//        lines.push_back(line);
//    }
//
//    for (const auto &text: lines)
//    {
//        vector<string> result = split(text, '\t');
//
//        // id   site_name   site_url    username    password    auth_text   recovery_code   comment
//        string cmd = "./pk -C ";
//        cmd +=
//                get_or_empty("site_name", result[1]) +
//                get_or_empty("site_url", result[2]) +
//                get_or_empty("username", result[3]) +
//                get_or_empty("password", result[4]) +
//                get_or_empty("auth_text", result[5]) +
//                get_or_empty("recovery_code", result[6]) +
//                get_or_empty("comment", result[7]);
//
//        cout << cmd << endl;
//    }

    using namespace std;
    using namespace Command::Enums;


    Command::Parser parser;
    ActionType action = parser.parse(argc, argv);
    if (action == ActionType::NONE)
        return 1;

    const SZKV_PAIR &szkv_pair = parser.get_szkv_pair();
    const BKV_PAIR &bkv_pair = parser.get_bkv_pair();

#ifdef DEBUG_PASSKEEPER
    cout << "action: " << to_string(action) << endl;
    for (const auto &pair: szkv_pair)
    {
        cout << to_string(pair.first) << ": " << pair.second << endl;
    }
    for (const auto &pair: bkv_pair)
    {
        cout << to_string(pair.first) << ": " << pair.second << endl;
    }
#endif

    Database::Connector connector(szkv_pair.at(SZValueKey::TABLE_NAME));

    if (!connector.connect(szkv_pair.at(SZValueKey::DB_FILE)))
        return 1;

    switch (action)
    {
        case ActionType::READ:
        {
            stringstream s_stream;
            EXIT_ON_ERROR(connector.select_record(s_stream, szkv_pair.at(SZValueKey::SITE_NAME), bkv_pair.at(BValueKey::OPTIMIZE_DISPLAY)))
            cout << s_stream.str();
            break;
        }
        case ActionType::CREATE:
        {
            int inserted = false;
            EXIT_ON_ERROR(connector.insert_record(
                    inserted,
                    szkv_pair.at(SZValueKey::SITE_NAME),
                    szkv_pair.at(SZValueKey::SITE_URL),
                    szkv_pair.at(SZValueKey::USERNAME),
                    szkv_pair.at(SZValueKey::PASSWORD),
                    szkv_pair.at(SZValueKey::AUTH_TEXT),
                    szkv_pair.at(SZValueKey::RECOVERY_CODE),
                    szkv_pair.at(SZValueKey::COMMENT)
            ))
            cout << "Inserted record: " << inserted << endl;
            break;
        }
        case ActionType::UPDATE:
        {
            int id = 0;
            try
            {
                id = stoi(szkv_pair.at(SZValueKey::ID));
            }
            catch (const invalid_argument &)
            {
                cout << "The id you provided cannot be converted to int";
            }
            catch (const out_of_range &)
            {
                cout << "The id you provided is fall out of the range";
            }

            int updated = false;
            EXIT_ON_ERROR(connector.update_record(
                    updated,
                    id,
                    szkv_pair.at(SZValueKey::SITE_NAME),
                    szkv_pair.at(SZValueKey::SITE_URL),
                    szkv_pair.at(SZValueKey::USERNAME),
                    szkv_pair.at(SZValueKey::PASSWORD),
                    szkv_pair.at(SZValueKey::AUTH_TEXT),
                    szkv_pair.at(SZValueKey::RECOVERY_CODE),
                    szkv_pair.at(SZValueKey::COMMENT)
            ))
            cout << "Updated record: " << updated << endl;
            break;
        }
        case ActionType::DELETE:
        {
            int id = 0;
            try
            {
                id = stoi(szkv_pair.at(SZValueKey::ID));
            }
            catch (const invalid_argument &)
            {
                cout << "The id you provided cannot be converted to int";
            }
            catch (const out_of_range &)
            {
                cout << "The id you provided is fall out of the range";
            }

            int deleted = false;
            EXIT_ON_ERROR(connector.delete_record(deleted, id))
            cout << "Deleted record: " << deleted << endl;
            break;
        }
        case ActionType::COUNT:
        {
            int count = 0;
            EXIT_ON_ERROR(connector.count_record(count))
            cout << count << endl;
            break;
        }
        default:
            throw runtime_error("Unexpected Behaviour");
    }
}