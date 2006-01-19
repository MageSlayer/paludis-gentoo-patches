/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "key_value_config_file.hh"
#include "internal_error.hh"

using namespace paludis;

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s)
{
    need_lines();
}

void
KeyValueConfigFile::accept_line(const std::string & line) const
{
    std::string::size_type p(line.find('='));
    if (std::string::npos == p)
        _entries[line] = "";
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        _entries[key] = replace_variables(strip_quotes(value));
    }
}

std::string
KeyValueConfigFile::replace_variables(const std::string & s) const
{
    std::string r;
    std::string::size_type p(0), old_p(0);

    while (p < s.length())
    {
        old_p = p;

        if ('\\' == s[p])
        {
            if (++p >= s.length())
                throw InternalError(PALUDIS_HERE, "todo");
            r += s[p++];
        }
        else if ('$' != s[p])
            r += s[p++];
        else
        {
            std::string name;
            if (++p >= s.length())
                throw InternalError(PALUDIS_HERE, "todo"); /// \bug

            if ('{' == s[p])
            {
                std::string::size_type q;
                if (std::string::npos == ((q = s.find("}", p))))
                    throw InternalError(PALUDIS_HERE, "todo");

                name = s.substr(p + 1, q - p - 1);
                p = q + 1;
            }
            else
            {
                std::string::size_type q;
                if (std::string::npos == ((q = s.find_first_not_of(
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "_0123456789", p))))
                    q = s.length();

                name = s.substr(p, q - p);
                p = q;
            }

            if (name.empty())
                throw InternalError(PALUDIS_HERE, "todo");
            r += get(name);
        }

        if (p <= old_p)
            throw InternalError(PALUDIS_HERE, "Infinite loop");
    }

    return r;
}

std::string
KeyValueConfigFile::strip_quotes(const std::string & s) const
{
    if (s.empty())
        return s;
    if (std::string::npos != std::string("'\"").find(s[0]))
    {
        if (s.length() < 2)
            throw InternalError(PALUDIS_HERE, "todo");
        if (s[s.length() - 1] != s[0])
            throw InternalError(PALUDIS_HERE, "todo");
        return s.substr(1, s.length() - 2);
    }
    else
        return s;
}
