#include <iostream>
#include "Command/ArgumentsParser.h"
#include "Persistence/Database.h"

//#define DEBUG_PASSKEEPER

int main(int argc, char *argv[])
{
    using namespace std;
    using namespace Command::Enums;

    PKVPAIR pkv_pair;

    ActionType action = Command::parse(argc, argv, &pkv_pair);
    if (action == ActionType::NONE)
        return 1;

#ifdef DEBUG_PASSKEEPER
    cout << "action: " << to_string(action) << endl;
    for (const auto &pair: pkv_pair)
    {
        cout << to_string(pair.first) << ": " << pair.second << endl;
    }
#endif

    Database::Connector connector(pkv_pair.at(FlagKey::TABLE_NAME));

    if (!connector.connect(pkv_pair.at(FlagKey::DB_FILE)))
        return 1;

    switch (action)
    {
        case ActionType::READ:
        {
            stringstream s_stream;
            connector.select_record(s_stream, pkv_pair.at(FlagKey::SITE_NAME));
            cout << s_stream.str();
            break;
        }
        case ActionType::CREATE:
        case ActionType::UPDATE:
        case ActionType::DELETE:
            break;
        default:
            throw runtime_error("Unexpected Behaviour");
    }
}