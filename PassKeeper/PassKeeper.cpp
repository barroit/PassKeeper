#include <iostream>
#include "Command/ArgumentsParser.h"
#include "Persistence/Database.h"

//#define DEBUG_PASSKEEPER

int main(int argc, char *argv[])
{
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
            connector.select_record(s_stream, szkv_pair.at(SZValueKey::SITE_NAME), bkv_pair.at(BValueKey::OPTIMIZE_DISPLAY));
            cout << s_stream.str();
            break;
        }
        case ActionType::CREATE:
        case ActionType::UPDATE:
        case ActionType::DELETE:
            break;
        case ActionType::COUNT:
        {
            int count;
            connector.count_record(count);
            cout << count << endl;
            break;
        }
        default:
            throw runtime_error("Unexpected Behaviour");
    }
}