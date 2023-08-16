#include "Validation.h"

bool Validation::is_blank(const std::string &value)
{
    return all_of(value.begin(), value.end(), [](unsigned char c) { return isspace(c); });
}
