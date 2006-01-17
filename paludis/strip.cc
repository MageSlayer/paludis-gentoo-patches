/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "strip.hh"

namespace paludis
{
    std::string strip_leading_string(const std::string & s, const std::string & prefix)
    {
        if (0 == s.compare(0, prefix.length(), prefix))
            return s.substr(prefix.length());
        else
            return s;
    }

    std::string strip_leading(const std::string & s, const std::string & remove)
    {
        std::string::size_type p(s.find_first_not_of(remove));
        if (std::string::npos == p)
            return std::string();
        else
            return s.substr(p);
    }

    std::string strip_trailing_string(const std::string & s, const std::string & suffix)
    {
        if (0 == s.compare(s.length() - suffix.length(), suffix.length(), suffix))
            return s.substr(0, s.length() - suffix.length());
        else
            return s;
    }

    std::string strip_trailing(const std::string & s, const std::string & remove)
    {
        std::string::size_type p(s.find_last_not_of(remove));
        if (std::string::npos == p)
            return std::string();
        else
            return s.substr(0, p + 1);
    }
}

