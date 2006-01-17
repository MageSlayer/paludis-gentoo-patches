/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "getenv.hh"
#include <cstdlib>

using namespace paludis;

GetenvError::GetenvError(const std::string & key) throw () :
    Exception("Environment variable '" + key + "' not set")
{
}

std::string
paludis::getenv_with_default(const std::string & key, const std::string & def)
{
    const char * const e(std::getenv(key.c_str()));
    return e ? e : def;
}

std::string
paludis::getenv_or_error(const std::string & key)
{
    const char * const e(std::getenv(key.c_str()));
    if (! e)
        throw GetenvError(key);
    return e;
}
