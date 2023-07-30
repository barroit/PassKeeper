#include <fstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <iostream>
#include "ArgumentsParser.h"

#define EXIT_ON_ERROR(r) if (!r) return Command::Enums::ActionType::NONE;

#define ACTION_SEMANTIC(ActionType) \
po::bool_switch()->notifier( \
    [&c_action](bool check) { ::valid_or_throw(check, c_action, ActionType); } \
)

#define COLUMN_SEMANTIC(FlagKey) \
po::value<string>()->notifier( \
    [](string value) { ::valid_or_throw(value, FlagKey); } \
)

// ensure those functions can only be used in this cpp file
namespace
{
    using namespace std;
    using namespace Command::Enums;
    namespace po = boost::program_options;

    string to_string(po::validation_error::kind_t);

    void valid_or_throw(bool, int &, ActionType);

    void valid_or_throw(const string &, FlagKey);

    bool add_value(bool, Command::Enums::FlagKey, const boost::program_options::variables_map &, PKVPAIR *);
}

Command::Enums::ActionType Command::parse(int argc, char *argv[], PKVPAIR *pkv_pair)
{
    using namespace Enums;
    po::options_description command_line;

    po::options_description generic("generic");
    generic.add_options()
            ("help", "Get command information.")
            ("db_file,db", po::value<string>()->default_value("Secret.db")->notifier(
                    [](string file_path) {
                        ifstream s_file(file_path);
                        bool b_exist = s_file.is_open();
                        s_file.close();
                        if (!b_exist)
                        {
                            cerr << "Please provide a .db file for operation, or make sure a default Secret.db exists in the same folder of PassKeeper." << endl;
                            throw po::validation_error(po::validation_error::invalid_option_value, "db_file");
                        }
                    }
            ), "The db file for operation.")
            ("table_name,table", po::value<string>()->default_value("Password"), "The table name in the database.");

    int c_action = 0;
    po::options_description operations("operations");
    operations.add_options()
            ("create_record,C", ACTION_SEMANTIC(ActionType::CREATE), "Create a record.")
            ("read,R", ACTION_SEMANTIC(ActionType::READ), "Read a record.")
            ("update_record,U", ACTION_SEMANTIC(ActionType::UPDATE), "Update a record.")
            ("delete,D", ACTION_SEMANTIC(ActionType::DELETE), "Delete a record.");

    // just for prompt
    po::options_description R("read");
    R.add_options()
            ("site_name", po::value<string>(), "The site name to locate for read.")
            ("optimize_display", po::bool_switch(), "Either to only display some crucial information to the user.");

    po::options_description CU("create_record, update_record");
    CU.add_options()
            ("site_name", COLUMN_SEMANTIC(FlagKey::SITE_NAME), "The site name should be in upper camel case and cannot be null.")
            ("site_url", COLUMN_SEMANTIC(FlagKey::SITE_URL), "The URL for site name disambiguation.")
            ("username", po::value<string>()->default_value("NULL"), "Your username. If you have a particular case, like storing a CDK, do not pass this.")
            ("password", po::value<string>()->default_value("NULL"), "Your password. If you have a particular case, like storing a bank card, do not pass this.")
            ("auth_text", COLUMN_SEMANTIC(FlagKey::AUTH_TEXT), "The authentication text for two-step verification.")
            ("recovery_code", COLUMN_SEMANTIC(FlagKey::RECOVERY_CODE), "The recovery code for account restoration.")
            ("comment", COLUMN_SEMANTIC(FlagKey::COMMENT), "Some extra information.");

    po::options_description D("delete, update_record");
    D.add_options()
            ("id", COLUMN_SEMANTIC(FlagKey::ID), "The record id to locate for deletion.");

    po::variables_map variables_map;

    // exclude R(read) to prevent name ambiguous
    command_line.add(generic).add(operations).add(CU).add(D);

    try
    {
        po::store(po::parse_command_line(argc, argv, command_line), variables_map);
    }
    catch (const boost::program_options::multiple_occurrences &error)
    {
        cerr << error.get_option_name() << " cannot be specified more than once" << endl;
        return ActionType::NONE;
    }
    catch (...)
    {
        cerr << "parse_command_line() raise an uncaught exception," << endl;
        return ActionType::NONE;
    }

    try
    {
        po::notify(variables_map);
    }
    catch (const po::validation_error &error) // not a copy
    {
        cerr << error.get_option_name() << ": " << ::to_string(error.kind()) << endl;
        return ActionType::NONE;
    }

#ifdef DEBUG_ARGUMENTSPARSER
    for (const auto &it: variables_map)
    {
        cout << it.first.c_str() << ": ";
        const auto &value = it.second.value();
        if (auto iv = boost::any_cast<int>(&value))
            cout << *iv;
        else if (auto sv = boost::any_cast<std::string>(&value))
            cout << *sv;
        else if (auto bv = boost::any_cast<bool>(&value))
            cout << *bv;
        cout << endl;
    }
#endif

    if (variables_map.count("help"))
    {
        po::options_description visible;
        visible.add(command_line).add(R);
        cout << visible << endl;
        return ActionType::NONE;
    }

    if (!c_action)
    {
        cerr << "Please provide at least one action for operation." << endl;
        return ActionType::NONE;
    }

    EXIT_ON_ERROR(add_value(true, FlagKey::DB_FILE, variables_map, pkv_pair))
    EXIT_ON_ERROR(add_value(true, FlagKey::TABLE_NAME, variables_map, pkv_pair))

    if (variables_map.at(to_string(ActionType::READ)).as<bool>())
    {
        EXIT_ON_ERROR(add_value(true, FlagKey::SITE_NAME, variables_map, pkv_pair))
        return ActionType::READ;
    }

    if (variables_map.at(to_string(ActionType::CREATE)).as<bool>())
    {
        EXIT_ON_ERROR(add_value(true, FlagKey::SITE_NAME, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::SITE_URL, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(true, FlagKey::USERNAME, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(true, FlagKey::PASSWORD, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::AUTH_TEXT, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::RECOVERY_CODE, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::COMMENT, variables_map, pkv_pair))
        return ActionType::CREATE;
    }

    if (variables_map.at(to_string(ActionType::UPDATE)).as<bool>())
    {
        EXIT_ON_ERROR(add_value(true, FlagKey::ID, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::SITE_NAME, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::SITE_URL, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::USERNAME, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::PASSWORD, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::AUTH_TEXT, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::RECOVERY_CODE, variables_map, pkv_pair))
        EXIT_ON_ERROR(add_value(false, FlagKey::COMMENT, variables_map, pkv_pair))
        return ActionType::UPDATE;
    }

    if (variables_map.at(to_string(ActionType::DELETE)).as<bool>())
    {
        EXIT_ON_ERROR(add_value(true, FlagKey::ID, variables_map, pkv_pair))
        return ActionType::DELETE;
    }

    throw runtime_error("Unexpected Behaviour");
}

// implementation of internal linkage functions
namespace
{
    std::string to_string(boost::program_options::validation_error::kind_t kind)
    {
        switch (kind)
        {
            case po::validation_error::invalid_option_value:
                return "invalid option value";
            case po::validation_error::at_least_one_value_required:
                return "at least one value required";
            case po::validation_error::invalid_bool_value:
                return "invalid bool value";
            case po::validation_error::multiple_values_not_allowed:
                return "multiple values not allowed";
            case po::validation_error::invalid_option:
                return "invalid option";
            default:
                throw runtime_error("ErrorKind out of range");
        }
    }

    void valid_or_throw(bool check, int &c_action, Command::Enums::ActionType action)
    {
        if (check && ++c_action - 1)
        {
            cerr << "Only one action is allowed." << endl;
            throw po::validation_error(po::validation_error::invalid_option, to_string(action));
        }
    }

    void valid_or_throw(const std::string &value, Command::Enums::FlagKey key)
    {
        // check if string is blank
        if (all_of(value.begin(), value.end(), [](unsigned char c) { return isspace(c); }))
        {
            cerr << "The value you have provided is empty or blank." << endl;
            throw po::validation_error(po::validation_error::invalid_option_value, to_string(key));
        }
    }

    bool add_value(bool required, Command::Enums::FlagKey key, const boost::program_options::variables_map &variables_map, PKVPAIR *pkv_pair)
    {
        string c_key = to_string(key);
        if (required && !variables_map.count(c_key))
        {
            cerr << c_key << ": at least one value required" << endl;
            return false;
        }
        pkv_pair->insert({ key, required ? variables_map.at(c_key).as<string>() : "" });
        return true;
    }
}

std::string Command::Enums::to_string(Command::Enums::ActionType action)
{
    switch (action)
    {
        case ActionType::NONE:
            return "none";
        case ActionType::CREATE:
            return "create_record";
        case ActionType::READ:
            return "read";
        case ActionType::UPDATE:
            return "update_record";
        case ActionType::DELETE:
            return "delete";
        default:
            throw runtime_error("ActionType out of range");
    }
}

std::string Command::Enums::to_string(Command::Enums::FlagKey key)
{
    switch (key)
    {
        case FlagKey::ID:
            return "id";
        case FlagKey::SITE_NAME:
            return "site_name";
        case FlagKey::SITE_URL:
            return "site_url";
        case FlagKey::USERNAME:
            return "username";
        case FlagKey::PASSWORD:
            return "password";
        case FlagKey::AUTH_TEXT:
            return "auth_text";
        case FlagKey::RECOVERY_CODE:
            return "recovery_code";
        case FlagKey::COMMENT:
            return "comment";
        case FlagKey::DB_FILE:
            return "db_file";
        case FlagKey::TABLE_NAME:
            return "table_name";
        case FlagKey::OPTIMIZE_DISPLAY:
            return "optimize_display";
        default:
            throw runtime_error("ColumnKey out of range");
    }
}