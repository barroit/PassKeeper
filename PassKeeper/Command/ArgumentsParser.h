#ifndef PASSKEEPER_ARGUMENTSPARSER_H
#define PASSKEEPER_ARGUMENTSPARSER_H

//#define DEBUG_ARGUMENTSPARSER

#include <boost/program_options.hpp>

#define VERSION "0.1.5"

#define SZKV_PAIR std::unordered_map<Command::Enums::SZValueKey, std::string>
#define BKV_PAIR std::unordered_map<Command::Enums::BValueKey, bool>

namespace Command
{
    using namespace std;
    namespace po = boost::program_options;

    namespace Enums
    {
        enum class ActionType;

        std::string to_string(ActionType);

        enum class SZValueKey;

        std::string to_string(SZValueKey);

        enum class BValueKey;

        std::string to_string(BValueKey);

        std::string to_string(boost::program_options::validation_error::kind_t);
    }

    class Parser
    {
    public:
        Parser();

        Command::Enums::ActionType parse(int, char *[]);

        const SZKV_PAIR &get_szkv_pair();

        const BKV_PAIR &get_bkv_pair();

    private:
        SZKV_PAIR szkv_pair;
        BKV_PAIR bkv_pair;
        boost::program_options::variables_map variables_map;

        bool add_value(bool, Command::Enums::SZValueKey);

        bool add_value(Command::Enums::BValueKey);
    };

    enum class Enums::ActionType
    {
        NONE = 0x00,
        CREATE,
        READ,
        UPDATE,
        DELETE,
        COUNT,
    };

    enum class Enums::SZValueKey
    {
        NONE = 0x00,
        ID,
        SITE_NAME,
        SITE_URL,
        USERNAME,
        PASSWORD,
        AUTH_TEXT,
        RECOVERY_CODE,
        COMMENT,
        DB_FILE,
        TABLE_NAME,
    };

    enum class Enums::BValueKey
    {
        NONE = 0x00,
        OPTIMIZE_DISPLAY,
        DISPLAY_ALL,
    };
}
#endif
