#ifndef PASSKEEPER_ARGUMENTSPARSER_H
#define PASSKEEPER_ARGUMENTSPARSER_H

//#define DEBUG_ARGUMENTSPARSER

#include <boost/program_options.hpp>

#define PKVPAIR std::unordered_map<Command::Enums::FlagKey, std::string>

namespace Command
{
    using namespace std;
    namespace po = boost::program_options;

    namespace Enums
    {
        enum class ActionType;

        string to_string(ActionType);

        enum class FlagKey;

        string to_string(FlagKey);
    }

    Enums::ActionType parse(int file_path, char *[], PKVPAIR *);

    enum class Enums::ActionType
    {
        NONE = 0x00,
        CREATE,
        READ,
        UPDATE,
        DELETE,
    };

    enum class Enums::FlagKey
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
        OPTIMIZE_DISPLAY,
    };
}
#endif
