/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "default_config_error.hh"

using namespace paludis;

DefaultConfigError::DefaultConfigError(const std::string & msg) throw () :
    Exception("Default configuration error: " + msg)
{
}
