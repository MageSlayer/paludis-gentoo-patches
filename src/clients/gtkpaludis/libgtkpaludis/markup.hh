/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_MARKUP_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_MARKUP_HH 1

#include <string>

namespace gtkpaludis
{
    std::string markup_escape(const std::string & s);
    std::string markup_foreground(const std::string & c, const std::string & s);
    std::string markup_bold(const std::string & s);
    std::string markup_italic(const std::string & s);
}


#endif
