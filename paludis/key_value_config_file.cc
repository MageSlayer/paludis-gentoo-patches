/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "key_value_config_file.hh"
#include "key_value_config_file_error.hh"
#include "internal_error.hh"

using namespace paludis;

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s)
{
    need_lines();
}

KeyValueConfigFile::~KeyValueConfigFile()
{
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
                throw KeyValueConfigFileError("Backslash not followed by a character");
            r += s[p++];
        }
        else if ('$' != s[p])
            r += s[p++];
        else
        {
            std::string name;
            if (++p >= s.length())
                throw KeyValueConfigFileError("Dollar not followed by a character");

            if ('{' == s[p])
            {
                std::string::size_type q;
                if (std::string::npos == ((q = s.find("}", p))))
                    throw KeyValueConfigFileError("Closing } not found");

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
                throw KeyValueConfigFileError("Empty variable name");
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
            throw KeyValueConfigFileError("Unterminated quote");
        if (s[s.length() - 1] != s[0])
            throw KeyValueConfigFileError("Mismatched quote");
        return s.substr(1, s.length() - 2);
    }
    else
        return s;
}
