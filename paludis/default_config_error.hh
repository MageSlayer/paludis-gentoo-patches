/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_ERROR_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_ERROR_HH 1

#include <paludis/exception.hh>

namespace paludis
{
    class DefaultConfigError : public Exception
    {
        public:
            DefaultConfigError(const std::string & msg) throw ();
    };
}

#endif
